#define _POSIX_C_SOURCE 199309L
#include <stdatomic.h>
#include <stdio.h>
#include <time.h>

#define EVENTLOOP_THREADPOOL
#include "eventloop.h"

#define TRY(fn)                                                                \
	do {                                                                   \
		int err = fn();                                                \
		if (err) {                                                     \
			printf("KO	%s: %d\n", #fn, err);                  \
			return err;                                            \
		} else {                                                       \
			printf("OK 	%s\n", #fn);                           \
		}                                                              \
	} while (0)

static int init_deinit(void)
{
	struct evloop l;
	struct evloop_config cfg = {0};
	int err;

	err = evloop_init(&l, cfg);
	if (err)
		return err;

	return evloop_deinit(&l);
}

static void sleep_callback(struct evloop *l, struct evloop_op_base *op)
{
	(void)l;
	(void)op;
}

static int sleep_op(void)
{
	struct evloop l;
	struct evloop_config cfg = {0};
	struct evloop_op_sleep sleep = evloop_op_sleep_init(
	    (struct evloop_sleep_data){
		.ms = 10, // 10ms.
	    },
	    &sleep_callback);
	;
	int err;
	struct timespec start, end;

	err = evloop_init(&l, cfg);
	if (err)
		goto end;

	err = evloop_queue(&l, &sleep);
	if (err)
		goto end;

	// Submit.
	err = evloop_poll(&l, EVLOOP_POLL_NOWAIT);
	if (err)
		goto end;
	// Poll.
	clock_gettime(CLOCK_MONOTONIC, &start);
	err = evloop_poll(&l, EVLOOP_POLL_ALL);
	if (err)
		goto end;
	clock_gettime(CLOCK_MONOTONIC, &end);

	if ((end.tv_nsec - start.tv_nsec) < (10 * 1000 * 1000)) {
		err = 1;
		goto end;
	}

end:
	evloop_deinit(&l);
	return err;
}

int main(void)
{
	TRY(init_deinit);
	TRY(sleep_op);
	printf("all tests are OK\n");
	return 0;
}
