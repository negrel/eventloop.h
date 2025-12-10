#define _POSIX_C_SOURCE 199309L
#include <time.h>

#define EVENTLOOP_THREADPOOL
#include "eventloop.h"

#define BATCH 10000

static void callback(struct evloop *l, struct evloop_op_base *b)
{
	(void)l;
	(void)b;
}

static int noops_bench(void)
{
	struct evloop l;
	struct evloop_config cfg = {0};
	struct evloop_op_noop noops[BATCH];
	int err;
	struct timespec start, end;

	clock_gettime(CLOCK_MONOTONIC, &start);
	err = evloop_init(&l, cfg);
	if (err)
		goto end;

	for (int i = 0; i < BATCH; i++) {
		noops[i] = evloop_op_noop_init(callback);
		evloop_queue(&l, &noops[i]);
	}
	err = evloop_poll(&l, EVLOOP_POLL_NOWAIT);
	if (err)
		goto end;
	err = evloop_poll(&l, EVLOOP_POLL_ALL);
	if (err)
		goto end;

	clock_gettime(CLOCK_MONOTONIC, &end);

	printf("executed %d noops in %ld sec %ld ns\n", BATCH,
	       end.tv_sec - start.tv_sec, end.tv_nsec - start.tv_nsec);

end:
	evloop_deinit(&l);
	return err;
}

int main(void)
{
	int err = noops_bench();
	if (err)
		goto end;

end:
	return err;
}
