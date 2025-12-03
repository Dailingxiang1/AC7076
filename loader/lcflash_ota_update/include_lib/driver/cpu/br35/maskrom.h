#ifndef  __MASKROM_H__
#define  __MASKROM_H__

struct os_argv_table {
    void *f;
    void *m;
    void *stack_free;
    void *stack_malloc;
    void *log_e;
    void *enter_critical;
    void *exit_critical;
    void *get_ms;
    void *cpu_task_sw;
    int (*cpu_irq_disabled)(void);
    void *jiffies_addr;
    void *jiffies_unit_addr;
    void *task_info_update_runtime;

    struct xbuf_lock_api {
        void *lock_init;
        void *lock;
        void *unlock;
    } lock;

};

struct maskrom_argv {
    void (*pchar)(char);
    void (*exp_hook)(u32 *);
    void (*local_irq_enable)();
    void (*local_irq_disable)();

    char *(*flt)(char **str, char *end, double num, int size, int precision, char fmt, int flags);
    struct os_argv_table os_argv;
};

void mask_init(const struct maskrom_argv *argv);

#endif  /*MASKROM_H*/
