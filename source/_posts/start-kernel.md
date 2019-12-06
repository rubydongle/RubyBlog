---
title: start_kernel
categories:
  - 技术文章
date: 2019-12-02 15:01:29
tags:
---

start_kernel是过了引导阶段，进入到了内核启动阶段的入口。在init/main.c中。

操作系统的第一个进程是init。每个进程在内核中都对应一个struct task_struct的结构,而init进程通过init_task来标记，其在init/init_task.c中初始化。
```c
/* Initial task structure */
struct task_struct init_task = INIT_TASK(init_task);
EXPORT_SYMBOL(init_task);

```
INIT_TASK是在include/linux/init_task.h中定义的宏
```c
/*
 *  INIT_TASK is used to set up the first task table, touch at
 * your own risk!. Base=0, limit=0x1fffff (=2MB)
 */
#define INIT_TASK(tsk)  \
{                                                                       \
        INIT_TASK_TI(tsk)                                               \
        .state          = 0,                                            \
        .stack          = init_stack,                                   \
        .usage          = ATOMIC_INIT(2),                               \
        .flags          = PF_KTHREAD,                                   \
        .prio           = MAX_PRIO-20,                                  \
        .static_prio    = MAX_PRIO-20,                                  \
        .normal_prio    = MAX_PRIO-20,                                  \
        .policy         = SCHED_NORMAL,                                 \
        .cpus_allowed   = CPU_MASK_ALL,                                 \
        .nr_cpus_allowed= NR_CPUS,                                      \
        .mm             = NULL,                                         \
        .active_mm      = &init_mm,                                     \
        .restart_block = {                                              \
                .fn = do_no_restart_syscall,                            \
        },                                                              \
        .se             = {                                             \
                .group_node     = LIST_HEAD_INIT(tsk.se.group_node),    \
        },                                                              \
        .rt             = {                                             \
                .run_list       = LIST_HEAD_INIT(tsk.rt.run_list),      \
                .time_slice     = RR_TIMESLICE,                         \
        },                                                              \
        .tasks          = LIST_HEAD_INIT(tsk.tasks),                    \
        INIT_PUSHABLE_TASKS(tsk)                                        \
        INIT_CGROUP_SCHED(tsk)                                          \
        .ptraced        = LIST_HEAD_INIT(tsk.ptraced),                  \
        .ptrace_entry   = LIST_HEAD_INIT(tsk.ptrace_entry),             \
        .real_parent    = &tsk,                                         \
        .parent         = &tsk,                                         \
        .children       = LIST_HEAD_INIT(tsk.children),                 \
        .sibling        = LIST_HEAD_INIT(tsk.sibling),                  \
        .group_leader   = &tsk,                                         \
        RCU_POINTER_INITIALIZER(real_cred, &init_cred),                 \
        RCU_POINTER_INITIALIZER(cred, &init_cred),                      \
        .comm           = INIT_TASK_COMM,                               \
        .thread         = INIT_THREAD,                                  \
        .fs             = &init_fs,                                     \
        .files          = &init_files,                                  \
        .signal         = &init_signals,                                \
        .sighand        = &init_sighand,                                \
        .nsproxy        = &init_nsproxy,                                \
        .pending        = {                                             \
                .list = LIST_HEAD_INIT(tsk.pending.list),               \
                .signal = {{0}}},                                       \
        .blocked        = {{0}},                                        \
        .alloc_lock     = __SPIN_LOCK_UNLOCKED(tsk.alloc_lock),         \
        .journal_info   = NULL,                                         \
        .cpu_timers     = INIT_CPU_TIMERS(tsk.cpu_timers),              \
        .pi_lock        = __RAW_SPIN_LOCK_UNLOCKED(tsk.pi_lock),        \
        .timer_slack_ns = 50000, /* 50 usec default slack */            \
        .pids = {                                                       \
                [PIDTYPE_PID]  = INIT_PID_LINK(PIDTYPE_PID),            \
                [PIDTYPE_PGID] = INIT_PID_LINK(PIDTYPE_PGID),           \
                [PIDTYPE_SID]  = INIT_PID_LINK(PIDTYPE_SID),            \
        },                                                              \
        .thread_group   = LIST_HEAD_INIT(tsk.thread_group),             \
        .thread_node    = LIST_HEAD_INIT(init_signals.thread_head),     \
        INIT_IDS                                                        \
        INIT_PERF_EVENTS(tsk)                                           \
        INIT_TRACE_IRQFLAGS                                             \
        INIT_LOCKDEP                                                    \
        INIT_FTRACE_GRAPH                                               \
        INIT_TRACE_RECURSION                                            \
        INIT_TASK_RCU_PREEMPT(tsk)                                      \
        INIT_TASK_RCU_TASKS(tsk)                                        \
        INIT_CPUSET_SEQ(tsk)                                            \
        INIT_RT_MUTEXES(tsk)                                            \
        INIT_PREV_CPUTIME(tsk)                                          \
        INIT_VTIME(tsk)                                                 \
        INIT_NUMA_BALANCING(tsk)                                        \
        INIT_KASAN(tsk)                                                 \
        INIT_RSC_ALLOC_LOCK(tsk)                                \
        INIT_RSC_CPUSET_LOCK(tsk)                               \
}

```

## set_task_stack_end_magic

## smp_setup_processor_id

## debug_objects_early_init

## cgroup_init_early

## 参考
https://www.cnblogs.com/yjf512/p/5999532.html
