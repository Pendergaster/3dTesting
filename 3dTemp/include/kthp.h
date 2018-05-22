#ifndef KTHP_H
#define KTHP_H

#include <stdint.h>

#define KTHP_SYNC_VARS_BUF_SIZE 128

typedef struct kth_job_t        kth_job_t;
typedef struct kth_job_grp_t    kth_job_grp_t;
typedef struct kth_work_entry_t kth_work_entry_t;
typedef struct kth_pool_t       kth_pool_t;

struct kth_job_t
{
    void (*func)(void*);
    void *args;
};

struct kth_job_grp_t
{
    kth_job_t   *jobs;
    int32_t     num;
    int32_t     num_started;
    int32_t     num_completed;
    int32_t     index;
};

struct kth_pool_t
{
    int                 running;
    void                *threads;
    int                 threads_max;
    int                 threads_num;
    kth_work_entry_t    *work;
    int                 work_max;
    int                 work_first;
    int                 work_num_unstarted;
    char                sync_vars[KTHP_SYNC_VARS_BUF_SIZE];
    char                flags;
};

/* With all functions, a return value of 0 indicates success unless otherwise
 * stated. */

int
kth_pool_init(kth_pool_t *tp, int num_threads, int num_initial_work_slots);

int
kth_pool_run(kth_pool_t *tp);

int
kth_pool_shutdown(kth_pool_t *tp, int graceful);
/* If graceful is non-zero, threads will first complete all pending work. */

int
kth_pool_destroy(kth_pool_t *tp);

int
kth_pool_add_job(kth_pool_t *tp, void (*func)(void*), void *args);

int
kth_pool_add_grp(kth_pool_t *tp, kth_job_grp_t *grp);

int
kth_pool_do_work(kth_pool_t *tp);
/* Returns 1 if work was done, 0 otherwise */

int
kth_job_grp_do_work(kth_job_grp_t *grp, kth_pool_t *tp);
/* Returns 1 if work was done, 0 otherwise */

kth_job_grp_t
kth_create_job_grp(kth_job_t *jobs, int num);

int
kth_job_grp_wait(kth_job_grp_t *grp, kth_pool_t *tp);

int
kth_job_grp_run_and_wait(kth_job_grp_t *grp, kth_pool_t *tp);

#define kth_job_grp_is_completed(grp_) (grp->num_completed >= grp->num)

#endif /* KTHP_H */

#ifdef THREAD_IMPELEMTATION
#include <stdlib.h>
#include <string.h>

//#include "kthp.h"

#if defined(__GNUC__)
    #include <pthread.h>
    #include <signal.h>
    typedef pthread_mutex_t     kth_mtx_t;
    typedef pthread_t           kth_thread_t;
    typedef pthread_cond_t      kth_cond_t;
    typedef void*(*kth_thread_func_t)(void*);
    typedef void* kth_thread_ret_t;
#elif defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    typedef HANDLE              kth_thread_t;
    typedef CRITICAL_SECTION    kth_mtx_t;
    typedef CONDITION_VARIABLE  kth_cond_t;
    #define kth_thread_func_t   LPTHREAD_START_ROUTINE
    #define kth_thread_ret_t    DWORD WINAPI
#else
    #error "Unsupported compiler!"
#endif

#define KTHP_SYNC_VARS_SIZE \
    (sizeof(kth_mtx_t) + sizeof(kth_cond_t))

typedef char _kthp_size_test[KTHP_SYNC_VARS_SIZE <= KTHP_SYNC_VARS_BUF_SIZE];
/* Test if the current buffer size is enough to hold the pool's
 * synchronization variables. Increase the size in kthp.h if necessary -
 * the size required is sizeof(kth_mtx_t) + sizeof(kth_cond_t).
 * Note: with GCC the -pedantic flag is required for this test to work. */

#define kthp_cv(tp_)     ((kth_cond_t*)(tp_)->sync_vars)
#define kthp_mtx(tp_)    ((kth_mtx_t*)((tp_)->sync_vars + sizeof(kth_cond_t)))
#define kthp_th(tp_, i_) (((kth_thread_t*)(tp_)->threads) + (i_))

#define KTH_WORK_ENTRY_JOB 0
#define KTH_WORK_ENTRY_GRP 1
#define KTH_WORK_ENTRY_EMP 2

#define KTHP_CVAR_INITIALIZED   (1 << 0)
#define KTHP_MTX_INITIALIZED    (1 << 1)
#define KTHP_GRACEFUL_SHUTDOWN  (1 << 2)

#define KTH_INCREMENT_TP_FIRST() \
    tp->work_first = (tp->work_first + 1) % tp->work_max; \
    tp->work_num_unstarted--;

#define KTH_GET_NEXT_COUNTER_FUNC_AND_ARGS(c_, f_, a_) \
    kth_work_entry_t *w_ = &tp->work[tp->work_first]; \
\
    switch (w_->type) \
    { \
        case KTH_WORK_ENTRY_JOB: \
        { \
            f_ = w_->data.job.func; \
            a_ = w_->data.job.args; \
            c_ = 0; \
            KTH_INCREMENT_TP_FIRST(); \
        } \
            break; \
        case KTH_WORK_ENTRY_GRP: \
        { \
            kth_job_grp_t *grp  = w_->data.grp; \
\
            int num = grp->num; \
            int i   = grp->num_started++; \
            f_      = grp->jobs[i].func; \
            a_      = grp->jobs[i].args; \
            c_      = &grp->num_completed; \
\
            if (i >= num - 1) \
                {KTH_INCREMENT_TP_FIRST();} \
\
        } \
            break; \
        case KTH_WORK_ENTRY_EMP: \
        { \
            f_ = 0; \
            KTH_INCREMENT_TP_FIRST(); \
        } \
    }

struct kth_work_entry_t
{
    int type;

    union
    {
        kth_job_t        job;
        kth_job_grp_t    *grp;
    } data;
};

static inline int       kth_thread_create(kth_thread_t *thread,
    kth_thread_func_t func, void *args);
static inline int       kth_thread_join(kth_thread_t *thread);
static inline int       kth_thread_detach(kth_thread_t *thread);
static inline void      kth_mtx_init(kth_mtx_t *mutex);
static inline int       kth_mtx_destroy(kth_mtx_t *mutex);
static inline int       kth_mtx_lock(kth_mtx_t *mutex);
static inline int       kth_mtx_unlock(kth_mtx_t *mutex);
static inline void      kth_cond_init(kth_cond_t *cond);
static inline int       kth_cond_destroy(kth_cond_t *cv);
static inline int       kth_cond_wait(kth_cond_t *cond, kth_mtx_t *mutex);
static inline int       kth_cond_signal_one(kth_cond_t *cvar);
static inline int       kth_cond_signal_all(kth_cond_t *cvar);
static inline int32_t   kth_interlocked_increment_int32(int32_t volatile *tar);
static inline int32_t   kth_interlocked_compare_exchange_int32(
    int32_t volatile *tar, int32_t exchange, int32_t comparand);

static inline int
_kth_pool_ensure_capacity_for_n_more_work(kth_pool_t *tp, int n);

static kth_thread_ret_t
_kth_pool_main(void *tp);

int
kth_pool_init(kth_pool_t *tp, int num_threads, int num_initial_jobs_slots)
{
    int ret = 0;

    memset(tp, 0, sizeof(kth_pool_t));

    tp->work = malloc(num_initial_jobs_slots * sizeof(kth_work_entry_t));
    if (!tp->work)
        {ret = 1; goto cleanup;}

    tp->threads = malloc(num_threads * sizeof(kth_thread_t));
    if (!tp->threads)
        {ret = 2; goto cleanup;}

    tp->threads_max = num_threads;

    kth_mtx_init(kthp_mtx(tp));
    kth_cond_init(kthp_cv(tp));

    tp->flags |= KTHP_CVAR_INITIALIZED;
    tp->flags |= KTHP_MTX_INITIALIZED;

    tp->work_max            = num_initial_jobs_slots;
    tp->work_first          = 0;
    tp->work_num_unstarted  = 0;
    tp->threads_num         = 0;

    cleanup:

    if (ret != 0)
    {
        kth_pool_destroy(tp);
        return ret;
    }

    return 0;
}

int
kth_pool_run(kth_pool_t *tp)
{
    if (tp->running)            return 1;
    if (tp->threads_max < 1)    return 2;

    int ret     = 0;
    tp->running = 1;
    int max_ths = tp->threads_max;

    for (int i = 0; i < max_ths; ++i)
    {
        if (kth_thread_create(kthp_th(tp, i), _kth_pool_main, (void*)tp) == 0)
            tp->threads_num++;
        else
            {ret = 3; break;}
    }

    if (ret != 0)
        kth_pool_shutdown(tp, 0);

    return ret;
}

int
kth_pool_destroy(kth_pool_t *tp)
{
    if (!tp)            return 1;
    if (tp->running)    return 2;
    if (tp->flags & KTHP_CVAR_INITIALIZED)
        kth_cond_destroy(kthp_cv(tp));
    if (tp->flags & KTHP_MTX_INITIALIZED)
        kth_mtx_destroy(kthp_mtx(tp));
    free(tp->threads);
    free(tp->work);
    memset(tp, 0, sizeof(kth_pool_t));
    return 0;
}

int
kth_pool_shutdown(kth_pool_t *tp, int graceful)
{
    if (!tp->running) return 0;

    tp->flags |= graceful ? KTHP_GRACEFUL_SHUTDOWN : 0;
    tp->running = 0;

    int num_errs = 0;

    if (kth_cond_signal_all(kthp_cv(tp)) == 0)
    {
        int i;
        if (!graceful)
        {
            for (i = 0; i < tp->threads_num; ++i)
                if (kth_thread_detach(kthp_th(tp, i)) != 0)
                    num_errs++;
        } else
        {
            for (i = 0; i < tp->threads_num; ++i)
                if (kth_thread_join(kthp_th(tp, i)) != 0)
                    num_errs++;
        }
    }
    if (!num_errs)
        tp->threads_num = 0;
    return num_errs;
}

int
kth_pool_add_job(kth_pool_t *tp, void (*func)(void*), void *args)
{
    kth_mtx_lock(kthp_mtx(tp));

    if (_kth_pool_ensure_capacity_for_n_more_work(tp, 1) != 0)
    {
        kth_mtx_unlock(kthp_mtx(tp));
        return 1;
    }

    int num_unstarted         = tp->work_num_unstarted;
    int i                   = (tp->work_first + num_unstarted) % tp->work_max;
    kth_work_entry_t *w     = &tp->work[i];

    w->type                 = KTH_WORK_ENTRY_JOB;
    w->data.job.func        = func;
    w->data.job.args        = args;
    tp->work_num_unstarted  = num_unstarted + 1;

    kth_mtx_unlock(kthp_mtx(tp));
    kth_cond_signal_one(kthp_cv(tp));
    return 0;
}

int
kth_pool_add_grp(kth_pool_t *tp, kth_job_grp_t *grp)
{
    int num_jobs = grp->num;
    kth_mtx_lock(kthp_mtx(tp));

    if (_kth_pool_ensure_capacity_for_n_more_work(tp, 1) != 0)
    {
        kth_mtx_unlock(kthp_mtx(tp));
        return 1;
    }

    int num_unstarted       = tp->work_num_unstarted;
    int i                   = (tp->work_first + num_unstarted) % tp->work_max;
    kth_work_entry_t *w     = &tp->work[i];

    w->type                 = KTH_WORK_ENTRY_GRP;
    w->data.grp             = grp;
    tp->work_num_unstarted  = num_unstarted + 1;
    grp->index              = i;

    kth_mtx_unlock(kthp_mtx(tp));

    if (num_jobs > 1)
        kth_cond_signal_all(kthp_cv(tp));
    else
        kth_cond_signal_one(kthp_cv(tp));
    return 0;
}

int
kth_pool_do_work(kth_pool_t *tp)
{
    if (tp->work_num_unstarted <= 0)
        return 0;

    kth_mtx_lock(kthp_mtx(tp));

    if (tp->work_num_unstarted <= 0)
    {
        kth_mtx_unlock(kthp_mtx(tp));
        return 0;
    }

    int32_t *counter;
    void    (*func)(void*);
    void    *args;

    KTH_GET_NEXT_COUNTER_FUNC_AND_ARGS(counter, func, args);

    kth_mtx_unlock(kthp_mtx(tp));

    if (func)
    {
        func(args);

        if (counter)
            kth_interlocked_increment_int32(counter);
    }

    return 1;
}

int
kth_job_grp_do_work(kth_job_grp_t *grp, kth_pool_t *tp)
{
    if (grp->num_started >= grp->num) return 0;

    kth_mtx_lock(kthp_mtx(tp));

    int num = grp->num;
    int i   = grp->num_started++;

    if (i >= grp->num)
    {
        kth_mtx_unlock(kthp_mtx(tp));
        return 0;
    }

    kth_job_t *j        = &grp->jobs[i];
    void (*func)(void*) = j->func;
    void *args          = j->args;

    if (i >= num - 1)
    {
        if (tp->work_first == grp->index)
            {KTH_INCREMENT_TP_FIRST();}
        else
            tp->work[grp->index].type = KTH_WORK_ENTRY_EMP;
    }

    kth_mtx_unlock(kthp_mtx(tp));

    func(args);
    kth_interlocked_increment_int32(&grp->num_completed);

    return 1;
}

kth_job_grp_t
kth_create_job_grp(kth_job_t *jobs, int num)
{
    kth_job_grp_t grp;
    grp.jobs            = jobs;
    grp.num             = num;
    grp.num_started     = 0;
    grp.num_completed   = 0;
    return grp;
}

int
kth_job_grp_wait(kth_job_grp_t *grp, kth_pool_t *tp)
{
    while (kth_interlocked_compare_exchange_int32(&grp->num_completed, -1, -1)
        < grp->num)
        kth_job_grp_do_work(grp, tp);
    return 0;
}

int
kth_job_grp_run_and_wait(kth_job_grp_t *grp, kth_pool_t *tp)
{
    if (kth_pool_add_grp(tp, grp) != 0)
        return 1;
    return kth_job_grp_wait(grp, tp) == 0 ? 0 : 2;
}

static inline int
_kth_pool_ensure_capacity_for_n_more_work(kth_pool_t *tp, int n)
{
    if (tp->work_num_unstarted + n >= tp->work_max)
    {
        /* Grow by at least 4 slots */
        int old_sz  = tp->work_max;
        int enlarge = n > 4 ? n : 4;
        int req_sz  = old_sz + enlarge;
        int sz      = old_sz * 150 / 100; /* Grow by 50 % */
        if (sz < req_sz)
            sz = req_sz;

        kth_work_entry_t *wa = realloc(tp->work, sz * sizeof(kth_work_entry_t));
        if (!wa) return 1;

        tp->work        = wa;
        tp->work_max    = sz;

        /* Move left-over work to it's new place */
        if (tp->work_first + tp->work_num_unstarted > old_sz)
        {
            int left_over = (tp->work_first + tp->work_num_unstarted) % old_sz;
            for (int i = 0; i < left_over; ++i)
                tp->work[(old_sz + i) % sz] = tp->work[i];
        }
    }
    return 0;
}

static kth_thread_ret_t
_kth_pool_main(void *pool_ptr)
{
    #define HAVE_GRACEFUL_WORK() \
        (tp->flags & KTHP_GRACEFUL_SHUTDOWN && tp->work_num_unstarted > 0)

    kth_pool_t *tp = (kth_pool_t *)pool_ptr;

    while (tp->running || (!tp->running && HAVE_GRACEFUL_WORK()))
    {
        kth_mtx_lock(kthp_mtx(tp));

        while (!tp->work_num_unstarted && tp->running)
            kth_cond_wait(kthp_cv(tp), kthp_mtx(tp));

        if (!tp->running)
        {
            if (!HAVE_GRACEFUL_WORK())
                {kth_mtx_unlock(kthp_mtx(tp)); break;}
        } else
        if (!tp->work_num_unstarted)
        {
            kth_mtx_unlock(kthp_mtx(tp));
            continue;
        }

        int32_t *counter;
        void    (*func)(void*);
        void    *args;

        KTH_GET_NEXT_COUNTER_FUNC_AND_ARGS(counter, func, args);

        kth_mtx_unlock(kthp_mtx(tp));

        if (func)
        {
            func(args);

            if (counter)
                kth_interlocked_increment_int32(counter);
        }
    }

    #undef HAVE_GRACEFUL_WORK

    return 0;
}

#if defined(__GNUC__)

static inline int
kth_thread_create(kth_thread_t *thread, kth_thread_func_t func, void *args)
    {return pthread_create(thread, 0, func, args);}

static inline int
kth_thread_join(kth_thread_t *thread) {return pthread_join(*thread, 0);}

static inline int
kth_thread_detach(kth_thread_t *thread) {return pthread_detach(*thread);}

static inline void
kth_mtx_init(kth_mtx_t *mutex) {pthread_mutex_init(mutex, 0);}

static inline int
kth_mtx_destroy(kth_mtx_t *mutex) {return pthread_mutex_destroy(mutex);}

static inline int
kth_mtx_lock(kth_mtx_t *mutex) {return pthread_mutex_lock(mutex);}

static inline int
kth_mtx_unlock(kth_mtx_t *mutex) {return pthread_mutex_unlock(mutex);}

static inline void
kth_cond_init(kth_cond_t *cv) {pthread_cond_init(cv, 0);}

static inline int
kth_cond_destroy(kth_cond_t *cv) {return pthread_cond_destroy(cv);}

static inline int
kth_cond_wait(kth_cond_t *cv, kth_mtx_t *m)
    {return pthread_cond_wait(cv, m);}

static inline int
kth_cond_signal_one(kth_cond_t *cv) {return pthread_cond_signal(cv);}

static inline int
kth_cond_signal_all(kth_cond_t *cv) {return pthread_cond_broadcast(cv);}

static inline int32_t
kth_interlocked_increment_int32(int32_t volatile *target)
    {return __sync_add_and_fetch(target, 1);}

static inline int32_t
kth_interlocked_compare_exchange_int32(int32_t volatile *tar, int32_t exchange,
    int32_t comparand)
    {return __sync_val_compare_and_swap(tar, comparand, exchange);}

#elif defined(_WIN32)

static inline int
kth_thread_create(kth_thread_t *thread, kth_thread_func_t func, void *args)
{
    *thread = CreateThread(0, 0, func, args, 0, 0);
    return *thread == NULL;
}

static inline int
kth_thread_join(kth_thread_t *thread)
{
    DWORD ra = WaitForSingleObject(*thread, INFINITE);
    DWORD rb = CloseHandle(*thread);
    int r = 0 | (ra ? (1 << 0) : 0) | (rb ? (1 << 1) : 0);
    return r;
}

static inline int
kth_thread_detach(kth_thread_t *thread)
{
    int tr = TerminateThread(*thread, THREAD_TERMINATE);
    int cr = CloseHandle(*thread);
    int ret = 0;
    if (tr) ret |= (1 << 0);
    if (cr) ret |= (1 << 1);
    return ret;
}

static inline void
kth_mtx_init(kth_mtx_t *mutex)
    {InitializeCriticalSectionAndSpinCount(mutex, 2000);}

static inline int
kth_mtx_destroy(kth_mtx_t *mutex) {DeleteCriticalSection(mutex); return 0;}

static inline int
kth_mtx_lock(kth_mtx_t *mutex) {EnterCriticalSection(mutex); return 0;}

static inline int
kth_mtx_unlock(kth_mtx_t *mutex) {LeaveCriticalSection(mutex); return 0;}

static inline void
kth_cond_init(kth_cond_t *cv) {InitializeConditionVariable(cv);}

static inline int
kth_cond_destroy(kth_cond_t *cv) { (void)cv; return 0; }

static inline int
kth_cond_wait(kth_cond_t *cv, kth_mtx_t *m)
    {return SleepConditionVariableCS(cv, m, INFINITE) ? 0 : 1;}

static inline int
kth_cond_signal_one(kth_cond_t *cv)
    {WakeConditionVariable(cv); return 0;}

static inline int
kth_cond_signal_all(kth_cond_t *cv)
    {WakeAllConditionVariable(cv); return 0;}

static inline int32_t
kth_interlocked_increment_int32(int32_t volatile *target)
    {return InterlockedIncrement(target);}

static inline int32_t
kth_interlocked_compare_exchange_int32(int32_t volatile *tar,
    int32_t exchange, int32_t comparand)
    {return InterlockedCompareExchange(tar, exchange, comparand);}

static inline int32_t
kth_interlocked_decrement_int32(int32_t volatile *target)
    {return InterlockedDecrement(target);}

#endif




#endif
