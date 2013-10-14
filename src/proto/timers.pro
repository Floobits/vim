/* timers.c */
unsigned long long get_monotonic_time __ARGS((void));
void insert_timeout __ARGS((timeout_T *to));
long call_timeouts __ARGS((long max_wait));
/* vim: set ft=c : */
