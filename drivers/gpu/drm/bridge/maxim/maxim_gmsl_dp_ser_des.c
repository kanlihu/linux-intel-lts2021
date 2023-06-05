// SPDX-License-Identifier: GPL-2.0-only
/*
 * MAXIM DP Serializer driver for MAXIM GMSL Serializers
 *
 */

#include <linux/device.h>
#include <linux/fwnode.h>
#include <linux/gpio/consumer.h>
#include <linux/gpio/driver.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/i2c-mux.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/regmap.h>
#include <linux/workqueue.h>
#include <max_96749.h>

#define I2C_BUS_AVAILABLE       (          15 )              // I2C Bus available in USB
#define SLAVE_SER_DEVICE_NAME   ( "MAX_GMSL_DP_SER" )        // Device and Driver Name
#define SLAVE_DES_DEVICE_NAME   ( "MAX_GMSL_DP_DES" )         // Device and Driver Name
#define MAX_SER_SLAVE_ADDR      (       0x80 )              // MAX SER Slave Address
#define MAX_DES_SLAVE_ADDR      (       0x90 )              // MAX DES Slave Address
#define MAX_GMSL_DP_SER_PWRDN_GPIO       ("tegra_main_gpio TEGRA234_MAIN_GPIO(G, 3) GPIO_ACTIVE_HIGH")
#define MAX_GMSL_DP_SER_ERRB_GPIO        ("tegra_main_gpio TEGRA234_MAIN_GPIO(G, 7) 0")
#define MAX_GMSL_DP_SER_DPRX_LINK_RATE   (0X1E)
#define MAX_GMSL_DP_SER_DPRX_LANE_COUNT  (0x04)
#define MAX_GMSL_DP_SER_ENABLE_MST       (0x0)
#define MAX_GMSL_DP_SER_MST_PAYLOAD_IDS  {0x1, 0x2, 0x3, 0x4}
#define MAX_GMSL_DP_SER_GMSL_STREAM_IDS  {0x0, 0x1, 0x2, 0x3}
#define MAX_GMSL_DP_SER_GMSL_LINK_SELECT {0x0, 0x0, 0x1, 0x1}
#define MAX_GMSL_DP_SER_ENABLE_DP_FEC    (0x1)
#define MAX_GMSL_DP_SER_ENABLE_DSC       {0x1, 0x0}
#define MAX_GMSL_DP_SER_ENABLE_GMSL_FEC  {0x1, 0x0}

static struct i2c_adapter *max_i2c_adapter     = NULL;  // I2C Adapter Structure
static struct i2c_client *max_ser_i2c_client   = NULL;
static struct i2c_client *max_des_i2c_client   = NULL;

struct max_gmsl_dp_ser_source {
	struct fwnode_handle *fwnode;
};

static const struct regmap_config max_gmsl_dp_ser_i2c_regmap = {
	.reg_bits = 16,
	.val_bits = 8,
};

struct max_gmsl_dp_ser_priv {
	struct i2c_client *client;
	struct i2c_client *max_des_i2c_client;
	struct gpio_desc *gpiod_pwrdn;
	u8 dprx_lane_count;
	u8 dprx_link_rate;
	struct mutex mutex;
	struct regmap *regmap;
	struct work_struct work;
	struct delayed_work delay_work;
	struct workqueue_struct *wq;
	int ser_errb;
	unsigned int ser_irq;
	bool enable_mst;
	u8 mst_payload_ids[MAX_GMSL_ARRAY_SIZE];
	u8 gmsl_stream_ids[MAX_GMSL_ARRAY_SIZE];
	u8 gmsl_link_select[MAX_GMSL_ARRAY_SIZE];
	bool link_a_is_enabled;
	bool link_b_is_enabled;
};

static int max_gmsl_dp_ser_read(struct max_gmsl_dp_ser_priv *priv, int reg)
{
	int ret, val = 0;

	ret = regmap_read(priv->regmap, reg, &val);
	if (ret < 0)
		dev_err(&priv->client->dev,
			"%s: register 0x%02x read failed (%d)\n",
			__func__, reg, ret);
	return val;
}

static int max_gmsl_dp_ser_write(struct max_gmsl_dp_ser_priv *priv, u32 reg, u8 val)
{
	int ret;

	ret = regmap_write(priv->regmap, reg, val);
	if (ret < 0)
		dev_err(&priv->client->dev,
			"%s: register 0x%02x write failed (%d)\n",
			__func__, reg, ret);
	return ret;
}

/* static api to update given value */
static inline void max_gmsl_dp_ser_update(struct max_gmsl_dp_ser_priv *priv,
					  u32 reg, u32 mask, u8 val)
{
	u8 update_val;

	update_val = max_gmsl_dp_ser_read(priv, reg);
	update_val = ((update_val & (~mask)) | (val & mask));
	max_gmsl_dp_ser_write(priv, reg, update_val);
}

static void max_gmsl_dp_ser_mst_setup(struct max_gmsl_dp_ser_priv *priv)
{
	int i;
	static const int max_mst_payload_id_reg[] = {
		MAX_GMSL_DP_SER_MST_PAYLOAD_ID_0,
		MAX_GMSL_DP_SER_MST_PAYLOAD_ID_1,
		MAX_GMSL_DP_SER_MST_PAYLOAD_ID_2,
		MAX_GMSL_DP_SER_MST_PAYLOAD_ID_3,
	};
	static const int max_gmsl_stream_id_regs[] = {
		MAX_GMSL_DP_SER_TX3_0,
		MAX_GMSL_DP_SER_TX3_1,
		MAX_GMSL_DP_SER_TX3_2,
		MAX_GMSL_DP_SER_TX3_3,
	};

	/* enable MST by programming MISC_CONFIG_B1 reg  */
	max_gmsl_dp_ser_update(priv, MAX_GMSL_DP_SER_MISC_CONFIG_B1,
			       MAX_GMSL_DP_SER_MISC_CONFIG_B1_MASK,
			       MAX_GMSL_DP_SER_MISC_CONFIG_B1_VAL);

	/* program MST payload IDs */
	for (i = 0; i < ARRAY_SIZE(priv->mst_payload_ids); i++) {
		max_gmsl_dp_ser_write(priv, max_mst_payload_id_reg[i],
				      priv->mst_payload_ids[i]);
	}

	/* Program stream IDs */
	for (i = 0; i < ARRAY_SIZE(priv->gmsl_stream_ids); i++) {
		max_gmsl_dp_ser_write(priv, max_gmsl_stream_id_regs[i],
				      priv->gmsl_stream_ids[i]);
	}
}

static void max_gmsl_dp_ser_setup(struct max_gmsl_dp_ser_priv *priv)
{
	int i;
	u8 gmsl_link_select_value = 0;
	static const int max_gmsl_ser_vid_tx_regs[] = {
		MAX_GMSL_DP_SER_VID_TX_X,
		MAX_GMSL_DP_SER_VID_TX_Y,
		MAX_GMSL_DP_SER_VID_TX_Z,
		MAX_GMSL_DP_SER_VID_TX_U,
	};

	max_gmsl_dp_ser_write(priv, MAX_GMSL_DP_SER_PHY_EDP_0_CTRL0_B0, 0x0f);
	max_gmsl_dp_ser_write(priv, MAX_GMSL_DP_SER_PHY_EDP_0_CTRL0_B1, 0x0f);
	max_gmsl_dp_ser_write(priv, MAX_GMSL_DP_SER_PHY_EDP_1_CTRL0_B0, 0x0f);
	max_gmsl_dp_ser_write(priv, MAX_GMSL_DP_SER_PHY_EDP_1_CTRL0_B1, 0x0f);
	max_gmsl_dp_ser_write(priv, MAX_GMSL_DP_SER_PHY_EDP_2_CTRL0_B0, 0x0f);
	max_gmsl_dp_ser_write(priv, MAX_GMSL_DP_SER_PHY_EDP_2_CTRL0_B1, 0x0f);
	max_gmsl_dp_ser_write(priv, MAX_GMSL_DP_SER_PHY_EDP_3_CTRL0_B0, 0x0f);
	max_gmsl_dp_ser_write(priv, MAX_GMSL_DP_SER_PHY_EDP_3_CTRL0_B1, 0x0f);

	max_gmsl_dp_ser_write(priv, MAX_GMSL_DP_SER_LOCAL_EDID, 0x1);

	/* Disable MST Mode */
	max_gmsl_dp_ser_write(priv, MAX_GMSL_DP_SER_MISC_CONFIG_B1, 0x0);

	max_gmsl_dp_ser_write(priv, MAX_GMSL_DP_SER_MAX_LINK_RATE,
			      priv->dprx_link_rate);

	max_gmsl_dp_ser_write(priv, MAX_GMSL_DP_SER_MAX_LINK_COUNT,
			      priv->dprx_lane_count);

	for (i = 0; i < MAX_GMSL_ARRAY_SIZE; i++) {
		gmsl_link_select_value = (priv->gmsl_link_select[i] <<
					  MAX_GMSL_DP_SER_LINK_SEL_SHIFT_VAL);
		max_gmsl_dp_ser_update(priv, max_gmsl_ser_vid_tx_regs[i],
				       MAX_GMSL_DP_SER_VID_TX_LINK_MASK,
				       gmsl_link_select_value);
	}

	max_gmsl_dp_ser_update(priv, MAX_GMSL_DP_SER_I2C_SPEED_CAPABILITY,
			       MAX_GMSL_DP_SER_I2C_SPEED_CAPABILITY_MASK,
			       MAX_GMSL_DP_SER_I2C_SPEED_CAPABILITY_100KBPS);

	if (priv->enable_mst)
		max_gmsl_dp_ser_mst_setup(priv);
}

static bool max_gmsl_dp_ser_check_dups(u8 *ids)
{
	int i = 0, j = 0;

	/* check if IDs are unique */
	for (i = 0; i < MAX_GMSL_ARRAY_SIZE; i++) {
		for (j = i + 1; j < MAX_GMSL_ARRAY_SIZE; j++) {
			if (ids[i] == ids[j])
				return false;
		}
	}

	return true;
}

static int max_gmsl_read_lock(struct max_gmsl_dp_ser_priv *priv,
			      u32 reg_addr, u32 mask,
			      u32 expected_value)
{
	u8 reg_data;

	reg_data = max_gmsl_dp_ser_read(priv, reg_addr);
	if ((reg_data & mask) == expected_value)
		return 0;

	return -1;
}

static irqreturn_t max_gsml_dp_ser_irq_handler(int irq, void *dev_id)
{
	struct max_gmsl_dp_ser_priv *priv = dev_id;
	int ret = 0;
	struct device *dev = &priv->client->dev;

	ret = max_gmsl_dp_ser_read(priv, MAX_GMSL_DP_SER_INTR9);
	if (ret & MAX_GMSL_DP_SER_LOSS_OF_LOCK_FLAG)
		dev_dbg(dev, "%s: Fault due to GMSL Link Loss\n", __func__);

	dev_dbg(dev, "%s: Sticky bit LOSS_OF_LOCK_FLAG cleared\n", __func__);

	return IRQ_HANDLED;
}

static void tegra_poll_gmsl_training_lock(struct work_struct *work)
{
	struct delayed_work *dwork = container_of(work,
					struct delayed_work, work);
	struct max_gmsl_dp_ser_priv *priv = container_of(dwork,
					struct max_gmsl_dp_ser_priv, delay_work);
	int ret = 0;

	ret = max_gmsl_read_lock(priv, MAX_GMSL_DP_SER_CTRL3,
				 MAX_GMSL_DP_SER_CTRL3_LOCK_MASK,
				 MAX_GMSL_DP_SER_CTRL3_LOCK_VAL);
	if (ret < 0) {
		dev_dbg(&priv->client->dev, "GMSL Lock is not set\n");
		goto reschedule;
	}

	if (priv->link_a_is_enabled) {
		ret = max_gmsl_read_lock(priv, MAX_GMSL_DP_SER_LCTRL2_A,
					 MAX_GMSL_DP_SER_LCTRL2_LOCK_MASK,
					 MAX_GMSL_DP_SER_LCTRL2_LOCK_VAL);
		if (ret < 0) {
			dev_dbg(&priv->client->dev, "GMSL Lock set failed for Link A\n");
			goto reschedule;
		}
	}

	if (priv->link_b_is_enabled) {
		ret = max_gmsl_read_lock(priv, MAX_GMSL_DP_SER_LCTRL2_B,
					 MAX_GMSL_DP_SER_LCTRL2_LOCK_MASK,
					 MAX_GMSL_DP_SER_LCTRL2_LOCK_VAL);
		if (ret < 0) {
			dev_dbg(&priv->client->dev, "GMSL Lock set failed for Link B\n");
			goto reschedule;
		}
	}

	ret = max_gmsl_read_lock(priv, MAX_GMSL_DP_SER_DPRX_TRAIN,
				 MAX_GMSL_DP_SER_DPRX_TRAIN_STATE_MASK,
				 MAX_GMSL_DP_SER_DPRX_TRAIN_STATE_VAL);
	if (ret < 0) {
		dev_dbg(&priv->client->dev,
			"DP Link tranining hasn't completed\n");
		goto reschedule;
	}

	max_gmsl_dp_ser_update(priv, MAX_GMSL_DP_SER_VID_TX_X,
			       MAX_GMSL_DP_SER_VID_TX_MASK, 0x1);
	max_gmsl_dp_ser_update(priv, MAX_GMSL_DP_SER_VID_TX_Y,
			       MAX_GMSL_DP_SER_VID_TX_MASK, 0x1);
	max_gmsl_dp_ser_update(priv, MAX_GMSL_DP_SER_VID_TX_Z,
			       MAX_GMSL_DP_SER_VID_TX_MASK, 0x1);
	max_gmsl_dp_ser_update(priv, MAX_GMSL_DP_SER_VID_TX_U,
			       MAX_GMSL_DP_SER_VID_TX_MASK, 0x1);

	return;

reschedule:
	queue_delayed_work(priv->wq,
			   &priv->delay_work, msecs_to_jiffies(500));
}

static int max_gmsl_dp_ser_init(struct device *dev)
{
	struct max_gmsl_dp_ser_priv *priv;
	struct i2c_client *client;
	int ret = 0;

	client = to_i2c_client(dev);
	priv = i2c_get_clientdata(client);

	priv->gpiod_pwrdn = devm_gpiod_get_optional(&client->dev, "enable",
						    GPIOD_OUT_HIGH);
	if (IS_ERR(priv->gpiod_pwrdn)) {
		dev_err(dev, "%s: gpiopwrdn is not enabled\n", __func__);
		return PTR_ERR(priv->gpiod_pwrdn);
	}
	gpiod_set_consumer_name(priv->gpiod_pwrdn, "max_gmsl_dp_ser-pwrdn");

	/* Drive PWRDNB pin high to power up the serializer */
	gpiod_set_value_cansleep(priv->gpiod_pwrdn, 1);

	/* Wait ~2ms for powerup to complete */
	usleep_range(2000, 2200);

	/*
	 * Write RESET_LINK = 1 (for both Phy A, 0x29, and Phy B, 0x33)
	 * within 10ms
	 */
	max_gmsl_dp_ser_update(priv, MAX_GMSL_DP_SER_LINK_CTRL_PHY_A,
			       MAX_GMSL_DP_SER_LINK_CTRL_A_MASK, 0x1);
	max_gmsl_dp_ser_update(priv, MAX_GMSL_DP_SER_LINK_CTRL_PHY_B,
			       MAX_GMSL_DP_SER_LINK_CTRL_B_MASK, 0x1);

	/*
	 * Disable video output on the GMSL link by setting VID_TX_EN = 0
	 * for Pipe X, Y, Z and U
	 */
	max_gmsl_dp_ser_update(priv, MAX_GMSL_DP_SER_VID_TX_X,
			       MAX_GMSL_DP_SER_VID_TX_MASK, 0x0);
	max_gmsl_dp_ser_update(priv, MAX_GMSL_DP_SER_VID_TX_Y,
			       MAX_GMSL_DP_SER_VID_TX_MASK, 0x0);
	max_gmsl_dp_ser_update(priv, MAX_GMSL_DP_SER_VID_TX_Z,
			       MAX_GMSL_DP_SER_VID_TX_MASK, 0x0);
	max_gmsl_dp_ser_update(priv, MAX_GMSL_DP_SER_VID_TX_U,
			       MAX_GMSL_DP_SER_VID_TX_MASK, 0x0);

	/*
	 * Set LINK_ENABLE=0 (0x7000) to force the DP HPD
	 * pin low to hold off DP link training and
	 * SOC video
	 */
	max_gmsl_dp_ser_update(priv, MAX_GMSL_DP_SER_LINK_ENABLE,
			       MAX_GMSL_DP_SER_LINK_ENABLE_MASK, 0x0);

	max_gmsl_dp_ser_setup(priv);

	/*
	 * Write RESET_LINK = 0 (for both Phy A, 0x29, and Phy B, 0x33)
	 * to initiate the GMSL link lock process.
	 */
	if (priv->link_a_is_enabled)
		max_gmsl_dp_ser_update(priv,
				       MAX_GMSL_DP_SER_LINK_CTRL_PHY_A,
				       MAX_GMSL_DP_SER_LINK_CTRL_A_MASK,
				       0x0);
	if (priv->link_b_is_enabled)
		max_gmsl_dp_ser_update(priv,
				       MAX_GMSL_DP_SER_LINK_CTRL_PHY_B,
				       MAX_GMSL_DP_SER_LINK_CTRL_B_MASK,
				       0x0);

	/*
	 * Set LINK_ENABLE = 1 (0x7000) to enable SOC DP link training,
	 * enable SOC video output to the serializer.
	 */
	max_gmsl_dp_ser_update(priv, MAX_GMSL_DP_SER_LINK_ENABLE,
			       MAX_GMSL_DP_SER_LINK_ENABLE_MASK, 0x1);

	queue_delayed_work(priv->wq, &priv->delay_work,
			   msecs_to_jiffies(500));

	return ret;
}

static int max_gmsl_dp_ser_init_priv(struct i2c_client *client,
					struct max_gmsl_dp_ser_priv *priv)
{
	struct device *dev = &priv->client->dev;
	struct device_node *ser = dev->of_node;
	int err = 0, i;
	u32 val = 0;

	dev_info(dev, "%s: init serializer device:\n", __func__);

	priv->dprx_lane_count = MAX_GMSL_DP_SER_DPRX_LANE_COUNT;
	dev_info(dev, "%s: - dprx-lane-count %i\n", __func__, val);

	priv->dprx_link_rate = MAX_GMSL_DP_SER_DPRX_LINK_RATE;
	dev_info(dev, "%s: - dprx-link-rate %i\n", __func__, val);

	uint8_t buf[] = MAX_GMSL_DP_SER_GMSL_LINK_SELECT;
	memcpy(priv->gmsl_link_select, buf, sizeof(buf));

	for (i = 0; i < ARRAY_SIZE(priv->gmsl_link_select); i++) {
		if ((priv->gmsl_link_select[i] == MAX_GMSL_DP_SER_ENABLE_LINK_A)
		    || (priv->gmsl_link_select[i] ==
		    MAX_GMSL_DP_SER_ENABLE_LINK_AB)) {
			priv->link_a_is_enabled = true;
		} else if ((priv->gmsl_link_select[i] == MAX_GMSL_DP_SER_ENABLE_LINK_B)
		    || (priv->gmsl_link_select[i] ==
		    MAX_GMSL_DP_SER_ENABLE_LINK_AB)) {
			priv->link_b_is_enabled = true;
		} else {
			dev_info(dev,
				 "%s: GMSL Link select values are invalid\n",
				 __func__);
			return -EINVAL;
		}
	}
	return err;
}

static int max_gmsl_dp_ser_des_probe(struct i2c_client *client)
{
	struct max_gmsl_dp_ser_priv *priv;
	struct device *dev;
	#if 0
	struct device_node *ser = client->dev.of_node;
	#endif
	int ret;

	priv = devm_kzalloc(&client->dev, sizeof(*priv), GFP_KERNEL);
	if (priv == NULL)
		return -ENOMEM;

	mutex_init(&priv->mutex);

	priv->client = client;
	i2c_set_clientdata(client, priv);

	priv->regmap = devm_regmap_init_i2c(client, &max_gmsl_dp_ser_i2c_regmap);
	if (IS_ERR(priv->regmap))
		return PTR_ERR(priv->regmap);

	dev = &priv->client->dev;

	ret = max_gmsl_dp_ser_read(priv, MAX_GMSL_DP_SER_REG_13);
	if (ret != 0) {
		dev_info(dev, "%s: MAXIM Serializer detected\n", __func__);
	} else {
		dev_err(dev, "%s: MAXIM Serializer Not detected\n", __func__);
		return -ENODEV;
	}

	ret = max_gmsl_dp_ser_init_priv(client, priv);
	if (ret < 0) {
		dev_err(dev, "%s: error init device\n", __func__);
		return -EFAULT;
	}

	priv->wq = alloc_workqueue("tegra_poll_gmsl_training_lock", WQ_HIGHPRI, 0);
	INIT_DELAYED_WORK(&priv->delay_work, tegra_poll_gmsl_training_lock);

	ret = max_gmsl_dp_ser_init(&client->dev);
	if (ret < 0) {
		dev_err(dev, "%s: dp serializer init failed\n", __func__);
		return -EFAULT;
	}

	ret = max_gmsl_dp_ser_read(priv, MAX_GMSL_DP_SER_INTR9);
	if (ret < 0) {
		dev_err(dev, "%s: INTR9 register read failed\n", __func__);
		return -EFAULT;
	}
	/* enable INTR8.LOSS_OF_LOCK_OEN */
	max_gmsl_dp_ser_update(priv, MAX_GMSL_DP_SER_INTR8,
			       MAX_GMSL_DP_SER_INTR8_MASK,
			       MAX_GMSL_DP_SER_INTR8_VAL);

	return ret;
}

static int max_gmsl_dp_ser_des_remove(struct i2c_client *client)
{
	struct max_gmsl_dp_ser_priv *priv = i2c_get_clientdata(client);

	i2c_unregister_device(client);
	gpiod_set_value_cansleep(priv->gpiod_pwrdn, 0);

	return 0;
}

#ifdef CONFIG_PM
static int max_ser_des_suspend(struct device *dev)
{
	return 0;
}

static int max_ser_des_resume(struct device *dev)
{
	return 0;
}
#else
#define max_ser_des_suspend	NULL
#define max_ser_des_resume	NULL
#endif /* CONFIG_PM */

/*
** I2C Board Info strucutre
*/
static struct i2c_board_info max_ser_i2c_board_info = {
    I2C_BOARD_INFO(SLAVE_SER_DEVICE_NAME, MAX_SER_SLAVE_ADDR),
	.platform_data = NULL,
};

static struct i2c_board_info max_des_i2c_board_info = {
    I2C_BOARD_INFO(SLAVE_DES_DEVICE_NAME, MAX_DES_SLAVE_ADDR),
	.platform_data = NULL,
};



static struct i2c_device_id max_96749_device_id[] = {
	{ "max,ser96749", 0},
	{},
};

static struct i2c_device_id max_96774_device_id[] = {
	{ "max,ser96774", 0},
	{},
};

MODULE_DEVICE_TABLE(i2c, max_96749_device_id);

static const struct dev_pm_ops max_gmsl_dp_ser_des_pm_ops = {
	.suspend = max_ser_des_suspend,
	.resume = max_ser_des_resume,
};

static struct i2c_driver max_gmsl_dp_ser_i2c_driver = {
	.driver = {
		.name = SLAVE_SER_DEVICE_NAME,
		.pm = &max_gmsl_dp_ser_des_pm_ops,
	},
	.probe_new	= max_gmsl_dp_ser_des_probe,
	.remove	= max_gmsl_dp_ser_des_remove,
	.id_table = max_96749_device_id,
};



static int max_dp_i2c_register() {
	int ret = -1;
    max_i2c_adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);
    
    if( max_i2c_adapter != NULL ) {
        max_ser_i2c_client = i2c_new_client_device(max_i2c_adapter, &max_ser_i2c_board_info);
        if( max_ser_i2c_client != NULL ) {
            i2c_add_driver(&max_gmsl_dp_ser_i2c_driver);
            ret = 0;
        }
        
        i2c_put_adapter(max_i2c_adapter);
    }
	return ret;
}

static void max_dp_i2c_unregister() {
	i2c_unregister_device(max_ser_i2c_client);
    i2c_del_driver(&max_gmsl_dp_ser_i2c_driver);
}
/*
** Module Init function
*/
static int __init max_ser_des_init(void)
{
	int ret;

	ret = max_dp_i2c_register();
	if (ret)
		return ret;
#if 0
	ret = max_dp_gpio_register();
	if (ret)
		goto err_main_was_registered;

	ret = max_dp_pwm_register();
	if (ret)
		goto err_gpio_was_registered;

	ret = auxiliary_driver_register(&max_dp_aux_driver);
	if (ret)
		goto err_pwm_was_registered;

	ret = auxiliary_driver_register(&max_dp_bridge_driver);
	if (ret)
		goto err_aux_was_registered;

	return 0;

err_aux_was_registered:
	auxiliary_driver_unregister(&max_dp_aux_driver);
err_pwm_was_registered:
	max_dp_pwm_unregister();
err_gpio_was_registered:
	max_dp_gpio_unregister();
err_main_was_registered:
	max_dp_i2c_unregister();
#endif
	return ret;
}

module_init(max_ser_des_init);

/*
** Module Exit function
*/
static void __exit max_ser_des_exit(void)
{
	max_dp_i2c_unregister();
}

module_exit(max_ser_des_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Maxim DP GMSL SerDes Driver");
MODULE_AUTHOR("Kanli.Hu <kanli.hu@intel.com>");

