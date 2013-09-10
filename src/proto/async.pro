/* async.c */
unsigned long long get_monotonic_time __ARGS((void));
void insert_timeout __ARGS((timeout_T *to));
void call_timeouts __ARGS((void));
/* vim: set ft=c : */
