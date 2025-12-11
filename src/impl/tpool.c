#include <stdatomic.h>
#include <stddef.h>
#include <threads.h>

#include "impl/tpool.h"
#include "mpsc.h"
#include "pthread.h"

#define THREADPOOL_IMPLEMENTATION
#include "threadpool.h"

#define container_of(ptr, type, member)                                        \
	(type *)((char *)(ptr) - offsetof(type, member))

#define op_from_private(private_, name)                                        \
	container_of(private_, struct evloop_op_##name, private)

int evloop_init(struct evloop *l, struct evloop_config cfg)
{
	tpool_init(&l->tpool, cfg.tpool);
	pthread_cond_init(&l->cond, NULL);
	mpsc_init(&l->completion_q);
	return 0;
}

int evloop_deinit(struct evloop *l)
{
	tpool_deinit(&l->tpool);
	pthread_cond_destroy(&l->cond);
	return 0;
}

/**
 * Execute all completions.
 */
static void evloop_exec_completions(struct evloop *l)
{
	while (1) {
		struct mpsc_node *n = mpsc_pop(&l->completion_q);
		if (n == NULL)
			return;

		struct evloop_private *private =
		    container_of(n, struct evloop_private, node);
		struct evloop_op_noop *noop = op_from_private(private, noop);
		noop->base.callback(l, &noop->base);
	}
}

int evloop_poll(struct evloop *l, enum evloop_poll_mode mode)
{
	unsigned long target = 0;
	unsigned long pending = atomic_load(&l->pending);
	int err = 0;

	switch (mode) {
	case EVLOOP_POLL_ALL:
		target = 0;
		break;
	case EVLOOP_POLL_ONE:
		target = pending >= 1 ? pending - 1 : 0;
		break;
	case EVLOOP_POLL_NOWAIT:
		evloop_exec_completions(l);
		goto submit;
	}

	// Poll completed I/O operations.
	pthread_mutex_t mu;
	pthread_mutex_init(&mu, NULL);
	pthread_mutex_lock(&mu);
	do {
		evloop_exec_completions(l);
		pending = atomic_load(&l->pending);
		if (pending <= target)
			break;

		pthread_cond_wait(&l->cond, &mu);
	} while (1);
	pthread_mutex_unlock(&mu);

submit:
	// Submit queued I/O operations.
	err = tpool_schedule(&l->tpool, l->batch);
	if (err)
		return err;

	atomic_fetch_add(&l->pending, l->batch.size);
	l->batch = (struct tpool_batch){0};

	return 0;
}

void evloop_op_work(struct tpool_task *task)
{
	struct evloop_private *private =
	    container_of(task, struct evloop_private, task);
	struct evloop_op_noop *noop = op_from_private(private, noop);

	switch (noop->base.code) {
	case EVLOOP_OP_NOOP:
		break;
	case EVLOOP_OP_SLEEP: {
		struct evloop_op_sleep *sleep = (struct evloop_op_sleep *)noop;
		struct timespec ts = {
		    .tv_sec = sleep->data.ms / 1000,
		    .tv_nsec = (sleep->data.ms % 1000) * 1000 * 1000,
		};
		struct timespec rem = {0};
		while (1) {
			if (thrd_sleep(&ts, &rem) == 0)
				break;
			// Interrupted by a signal.
			ts = rem;
			rem = (struct timespec){0};
		}
		break;
	}
	}

	mpsc_push(&private->loop->completion_q, &private->node);
	atomic_fetch_sub(&private->loop->pending, 1);
	pthread_cond_signal(&private->loop->cond);
}
