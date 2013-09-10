#include "vim.h"

#ifdef FEAT_ASYNC

/*
 * Insert a new timeout into the timeout linked list.
 * This is called by set_timeout() in eval.c
 */
    void
insert_timeout(to)
    timeout_T *to;  /* timeout to insert */
{
    timeout_T *cur = timeouts;
    timeout_T *prev = NULL;

    if (timeouts == NULL) {
        timeouts = to;
        return;
    }
    while (cur != NULL) {
        if (cur->tm > to->tm) {
            if (prev) {
                prev->next = to;
            } else {
                timeouts = to;
            }
            to->next = cur;
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

/*
 * Execute timeouts that are due.
 * This is called every ticktime milliseconds by low-level input functions.
 */
    void
call_timeouts() {
    struct timeval now;
    gettimeofday(&now, NULL);
    unsigned long tm = now.tv_sec * 1000 + now.tv_usec/1000;
    timeout_T *tmp;

    while (timeouts != NULL && timeouts->tm < tm) {
        do_cmdline_cmd(timeouts->cmd);
        tmp = timeouts;
        timeouts = timeouts->next;
        if (tmp->interval == -1) {
            free(tmp->cmd);
            free(tmp);
        } else {
            tmp->tm = tm + tmp->interval;
            insert_timeout(tmp);
        }
    }
}

#endif
