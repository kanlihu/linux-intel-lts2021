#ifndef __MAX_96772_H__
#define __MAX_96772_H__

//address
#define MAX_GMSL_DP_DES_CTRL0                   0x0010
#define MAX_GMSL_DP_DES_CTRL0_VAL               0x00

#define MAX_GMSL_DP_DES_RX0_SET_STREAM_ID       0x0050
#define MAX_GMSL_DP_DES_RX0_SET_STREAM_ID_VAL   0x00

#define MAX_GMSL_DP_DES_LINK_RATE               0xE790
#define MAX_GMSL_DP_DES_LINK_RATE_1             0xE791
#define MAX_GMSL_DP_DES_LINK_RATE_VAL           0x0A
#define MAX_GMSL_DP_DES_LINK_RATE_1_VAL         0x00

#define MAX_GMSL_DP_DES_LANE_COUNT              0xE792
#define MAX_GMSL_DP_DES_LANE_COUNT_1            0xE793
#define MAX_GMSL_DP_DES_LANE_COUNT_VAL          0x04
#define MAX_GMSL_DP_DES_LANE_COUNT_1_VAL        0x00

#define MAX_GMSL_DP_DES_HRES_B0                 0xE794
#define MAX_GMSL_DP_DES_HRES_B1                 0xE795
#define MAX_GMSL_DP_DES_HRES_B0_VAL             0x00
#define MAX_GMSL_DP_DES_HRES_B1_VAL             0x0A

#define MAX_GMSL_DP_DES_HFP_B0                  0xE796
#define MAX_GMSL_DP_DES_HFP_B1                  0xE797
#define MAX_GMSL_DP_DES_HFP_B0_VAL              0x30
#define MAX_GMSL_DP_DES_HFP_B1_VAL              0x00

#define MAX_GMSL_DP_DES_HSW_B0                  0xE798
#define MAX_GMSL_DP_DES_HSW_B1                  0xE799
#define MAX_GMSL_DP_DES_HSW_B0_VAL              0x20
#define MAX_GMSL_DP_DES_HSW_B1_VAL              0x00

#define MAX_GMSL_DP_DES_HBP_B0                  0xE79A
#define MAX_GMSL_DP_DES_HBP_B1                  0xE79B
#define MAX_GMSL_DP_DES_HBP_B0_VAL              0x40
#define MAX_GMSL_DP_DES_HBP_B1_VAL              0x01

#define MAX_GMSL_DP_DES_VRES_B0                 0xE79C
#define MAX_GMSL_DP_DES_VRES_B1                 0xE79D
#define MAX_GMSL_DP_DES_VRES_B0_VAL             0x40
#define MAX_GMSL_DP_DES_VRES_B1_VAL             0x06

#define MAX_GMSL_DP_DES_VFP_B0                  0xE79E
#define MAX_GMSL_DP_DES_VFP_B1                  0xE79F
#define MAX_GMSL_DP_DES_VFP_B0_VAL              0x1E
#define MAX_GMSL_DP_DES_VFP_B1_VAL              0x00

#define MAX_GMSL_DP_DES_VSW_B0                  0xE7A0
#define MAX_GMSL_DP_DES_VSW_B1                  0xE7A1
#define MAX_GMSL_DP_DES_VSW_B0_VAL              0x03
#define MAX_GMSL_DP_DES_VSW_B1_VAL              0x00

#define MAX_GMSL_DP_DES_VBP_B0                  0xE7A2
#define MAX_GMSL_DP_DES_VBP_B1                  0xE7A3
#define MAX_GMSL_DP_DES_VBP_B0_VAL              0x2F
#define MAX_GMSL_DP_DES_VBP_B1_VAL              0x00

#define MAX_GMSL_DP_DES_HWORDS_B0               0xE7A4
#define MAX_GMSL_DP_DES_HWORDS_B1               0xE7A5
#define MAX_GMSL_DP_DES_HWORDS_B0_VAL           0xFC
#define MAX_GMSL_DP_DES_HWORDS_B1_VAL           0x0E

#define MAX_GMSL_DP_DES_MVID_B0                 0xE7A6
#define MAX_GMSL_DP_DES_MVID_B1                 0xE7A7
#define MAX_GMSL_DP_DES_MVID_B0_VAL             0x73
#define MAX_GMSL_DP_DES_MVID_B1_VAL             0x8D

#define MAX_GMSL_DP_DES_NVID_B0                 0xE7A8
#define MAX_GMSL_DP_DES_NVID_B1                 0xE7A9
#define MAX_GMSL_DP_DES_NVID_B0_VAL             0x00
#define MAX_GMSL_DP_DES_NVID_B1_VAL             0x80

#define MAX_GMSL_DP_DES_TUC_VALUE_B0            0xE7AA
#define MAX_GMSL_DP_DES_TUC_VALUE_B1            0xE7AB
#define MAX_GMSL_DP_DES_TUC_VALUE_B0_VAL        0x40
#define MAX_GMSL_DP_DES_TUC_VALUE_B1_VAL        0x00

#define MAX_GMSL_DP_DES_HVPOL_B0                0xE7AC
#define MAX_GMSL_DP_DES_HVPOL_B1                0xE7AD
#define MAX_GMSL_DP_DES_HVPOL_B0_VAL            0x00
#define MAX_GMSL_DP_DES_HVPOL_B1_VAL            0x00

#define MAX_GMSL_DP_DES_SS_ENABLE_B0            0xE7B0
#define MAX_GMSL_DP_DES_SS_ENABLE_B1            0xE7B1
#define MAX_GMSL_DP_DES_SS_ENABLE_B0_VAL        0x01
#define MAX_GMSL_DP_DES_SS_ENABLE_B1_VAL        0x00

#define MAX_GMSL_DP_DES_SPREAD_BIT_RATIO        0x6003  //not found in spec
#define MAX_GMSL_DP_DES_SPREAD_BIT_RATIO_VAL    0x82

#define MAX_GMSL_DP_DES_CLK_REF_B0              0xE7B2
#define MAX_GMSL_DP_DES_CLK_REF_B1              0xE7B3
#define MAX_GMSL_DP_DES_CLK_REF_B2              0xE7B4
#define MAX_GMSL_DP_DES_CLK_REF_B3              0xE7B5
#define MAX_GMSL_DP_DES_CLK_REF_B4              0xE7B6
#define MAX_GMSL_DP_DES_CLK_REF_B5              0xE7B7
#define MAX_GMSL_DP_DES_CLK_REF_B6              0xE7B8
#define MAX_GMSL_DP_DES_CLK_REF_B7              0xE7B9
#define MAX_GMSL_DP_DES_CLK_REF_B8              0xE7BA
#define MAX_GMSL_DP_DES_CLK_REF_B9              0xE7BB
#define MAX_GMSL_DP_DES_CLK_REF_B10             0xE7BC
#define MAX_GMSL_DP_DES_CLK_REF_B11             0xE7BD
#define MAX_GMSL_DP_DES_CLK_REF_B12             0xE7BE
#define MAX_GMSL_DP_DES_CLK_REF_B13             0xE7BF

#define MAX_GMSL_DP_DES_CLK_REF_B0_VAL          0x50
#define MAX_GMSL_DP_DES_CLK_REF_B1_VAL          0x00
#define MAX_GMSL_DP_DES_CLK_REF_B2_VAL          0x00
#define MAX_GMSL_DP_DES_CLK_REF_B3_VAL          0x40
#define MAX_GMSL_DP_DES_CLK_REF_B4_VAL          0x6C
#define MAX_GMSL_DP_DES_CLK_REF_B5_VAL          0x20
#define MAX_GMSL_DP_DES_CLK_REF_B6_VAL          0x07
#define MAX_GMSL_DP_DES_CLK_REF_B7_VAL          0x00
#define MAX_GMSL_DP_DES_CLK_REF_B8_VAL          0x01
#define MAX_GMSL_DP_DES_CLK_REF_B9_VAL          0x00
#define MAX_GMSL_DP_DES_CLK_REF_B10_VAL         0x00
#define MAX_GMSL_DP_DES_CLK_REF_B11_VAL         0x00
#define MAX_GMSL_DP_DES_CLK_REF_B12_VAL         0x52
#define MAX_GMSL_DP_DES_CLK_REF_B13_VAL         0x00

#define MAX_GMSL_DP_DES_USER_CMD_B0             0xE776
#define MAX_GMSL_DP_DES_USER_CMD_B1             0xE777
#define MAX_GMSL_DP_DES_USER_CMD_B0_VAL         0x02
#define MAX_GMSL_DP_DES_USER_CMD_B1_VAL         0x80


#endif

