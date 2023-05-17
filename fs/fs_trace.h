#if !defined(_FS_TRACE_H_) || defined(TRACE_HEADER_MULTI_READ)
#define _FS_TRACE_H_

#include <linux/stringify.h>
#include <linux/types.h>
#include <linux/tracepoint.h>

#undef TRACE_SYSTEM
#define TRACE_SYSTEM fs
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE fs_trace

#undef __string
#define __string(item, src) __dynamic_array(char, item,\
    strlen((src) ? (const char *)(src) : "(null)") + 64)

#define TYPE_BEGIN 0
#define TYPE_END   1
#define TYPE_VALUE 2

//tracing_mark_write: B|364|HIDL::IAtraceDevice::listCategories::server
//tracing_mark_write: E|364
//tracing_mark_write: C|3274|Transaction::apply_count|2

TRACE_EVENT(tracing_mark_write,
    TP_PROTO(int pid, const char *name, int trace_type, int value),
    TP_ARGS(pid, name, trace_type, value),
    TP_STRUCT__entry(
            __field(int, pid)
            __string(trace_name, name)
            __field(int, trace_type)
            __field(int, value)
    ),
    TP_fast_assign(
            __entry->pid = pid;
            __entry->trace_type = trace_type;
            __entry->value = value;
            switch(trace_type) {
                case TYPE_BEGIN:
                    sprintf(__get_str(trace_name),
                        "%s|%d|%s",
                        "B", __entry->pid, name ? (const char *)(name) : "(null)");
                    break;
                case TYPE_END:
                    sprintf(__get_str(trace_name),
                        "%s|%d",
                        "E", __entry->pid);
                    break;
                case TYPE_VALUE:
                    sprintf(__get_str(trace_name),
                        "%s|%d|%s|%d",
                        "C", __entry->pid, name ? (const char *)(name) : "(null)", __entry->value);
                    break;
            }
    ),
    TP_printk("%s", __get_str(trace_name))
)



#define FS_ATRACE_BEGIN(name) trace_tracing_mark_write(current->tgid, name, 1, 0)
#define FS_ATRACE_END(name) trace_tracing_mark_write(current->tgid, name, 0, 0)
#define FS_ATRACE_FUNC_BEGIN() FS_ATRACE_BEGIN(__func__)
#define FS_ATRACE_FUNC_END() FS_ATRACE_END(__func__)

#define FS_ATRACE_INT(name, value) \
    trace_tracing_mark_write(current->tgid, name, 2, value)

#endif /* _FS_TRACE_H_ */

/* This part must be outside protection */
#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH ../../fs/
#include <trace/define_trace.h>
