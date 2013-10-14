/* vi:set ts=8 sts=4 sw=4:
 *
 * VIM - Vi IMproved	by Bram Moolenaar
 *
 * Do ":help uganda"  in Vim to read copying and usage conditions.
 * Do ":help credits" in Vim to see a list of people who contributed.
 * See README.txt for an overview of the Vim source code.
 */

#include "vim.h"

#ifdef FEAT_TIMERS

/*
 * Return monotonic time, if available. Fall back to gettimeofday otherwise.
 */
	unsigned long long
get_monotonic_time(void)
{
    unsigned long long tm;
# ifdef MCH_MONOTONIC_TIME
    tm = mch_monotonic_time();
# else
    struct timeval now;
    gettimeofday(&now, NULL);
    tm = now.tv_sec * 1000 + now.tv_usec / 1000;
# endif
    return tm;
}


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

    if (timeouts == NULL)
    {
	timeouts = to;
	return;
    }

    while (cur != NULL)
    {
	if (cur->tm > to->tm)
	{
	    if (prev)
	    {
		prev->next = to;
	    }
	    else
	    {
		timeouts = to;
	    }
	    to->next = cur;
	    return;
	}
	prev = cur;
	cur = cur->next;
    }
    prev->next = to;
    to->next = NULL;
}

/*
 * Execute timeouts that are due.
 * Return the amount of time before call_timeouts() should be run again.
 */
	long
call_timeouts(max_to_wait)
    long max_to_wait;
{
    unsigned long long now = get_monotonic_time();
    unsigned long long towait = p_tt;
    timeout_T *tmp;
    int retval;

    if (calling_timeouts)
    {
	return max_to_wait;
    }

    if (p_tt == -1)
    {
	return max_to_wait;
    }
    calling_timeouts = TRUE;

    while (timeouts != NULL && timeouts->tm < now)
    {
	retval = do_cmdline_cmd(timeouts->cmd);
	tmp = timeouts;
	timeouts = timeouts->next;
	if (tmp->interval == -1 || retval == FAIL || did_throw || did_emsg)
	{
	    if (got_int)
	    {
		if (tmp->sourcing_lnum)
		{
		    EMSG(_("E881: An interval was canceled because of an interrupt"));
		    EMSG3(_("%s:%s"), tmp->sourcing_name, tmp->sourcing_lnum);
		}
		else
		{
		    EMSG(_("E881: An interval was canceled because of an interrupt"));
		    EMSG2(_("%s"), tmp->sourcing_name);
		}
	    }
	    free(tmp->cmd);
	    free(tmp->sourcing_name);
	    free(tmp);
	}
	else
	{
	    tmp->tm = now + tmp->interval;
	    insert_timeout(tmp);
	}
    }

    calling_timeouts = FALSE;

    /* if there is not a timer, change towait so that it will get called */
    if (timeouts != NULL && max_to_wait != 0)
    {
	now = get_monotonic_time();
	if (now > timeouts->tm)
	    return p_tt;

	towait = timeouts->tm - now;

	/* don't wake up every 1 ms ... limit to p_tt */
	if (towait < (unsigned long long)p_tt)
	    towait = p_tt;

	/* don't overshoot the wait time */
	if (max_to_wait > 0 && towait > (unsigned long long)max_to_wait)
	    towait = max_to_wait;

        return (long)towait;
    }

    return max_to_wait;
}

#endif
