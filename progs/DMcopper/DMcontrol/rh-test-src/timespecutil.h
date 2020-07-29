/* convenience functions */

#define NSECS_PER_SEC 1000000000

#define timespec_normalize(t) {\
        if ((t)->tv_nsec >= NSECS_PER_SEC) { \
                (t)->tv_nsec -= NSECS_PER_SEC; \
                (t)->tv_sec++; \
        } else if ((t)->tv_nsec < 0) { \
                (t)->tv_nsec += NSECS_PER_SEC; \
                (t)->tv_sec--; \
        } \
}

#define timespec_add(t1, t2) do { \
        (t1)->tv_nsec += (t2)->tv_nsec;  \
        (t1)->tv_sec += (t2)->tv_sec; \
        timespec_normalize(t1);\
} while (0)

#define timespec_sub(t1, t2) do { \
        (t1)->tv_nsec -= (t2)->tv_nsec;  \
        (t1)->tv_sec -= (t2)->tv_sec; \
        timespec_normalize(t1);\
} while (0)

#define timespec_add_ns(t,n) do { \
        (t)->tv_nsec += (n);  \
        timespec_normalize(t); \
        } while (0)

#define timespec_nz(t) ((t)->tv_sec != 0 || (t)->tv_nsec != 0)

#define timespec_lt(t1, t2) ((t1)->tv_sec < (t2)->tv_sec || ((t1)->tv_sec == (t2)->tv_sec && (t1)->tv_nsec < (t2)->tv_nsec))

#define timespec_gt(t1, t2) (timespec_lt(t2, t1))

#define timespec_ge(t1, t2) (!timespec_lt(t1, t2))

#define timespec_le(t1, t2) (!timespec_gt(t1, t2))

#define timespec_eq(t1, t2) ((t1)->tv_sec == (t2)->tv_sec && (t1)->tv_nsec == (t2)->tv_nsec)

