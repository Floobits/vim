
EXTERN timeout_T *timeouts INIT(= NULL);

void insert_timeout(timeout_T *to);
void call_timeouts();
