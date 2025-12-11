#ifndef EVLOOP_TPOOL_H_INCLUDE
#define EVLOOP_TPOOL_H_INCLUDE

#include <stdatomic.h>

#include "eventloop.h"
#include "mpsc.h"
#include "threadpool.h"

/**
 * Configuration options for the thread pool based event loop implementation.
 */
struct evloop_config {
	struct tpool_config tpool;
};

/**
 * Thread pool based event loop implementation.
 */
struct evloop {
	struct tpool tpool;
	struct tpool_batch batch;
	struct mpsc completion_q;
	atomic_ulong pending;
	pthread_cond_t cond;
};

#define OP(name)                                                               \
	evloop_op_##name                                                       \
	{                                                                      \
		struct evloop_op_base base;                                    \
		struct evloop_private private;                                 \
		struct evloop_##name##_data data;                              \
	}

#define OP_INIT(name, NAME)                                                    \
	struct evloop_op_##name evloop_op_##name##_init(                       \
	    struct evloop_##name##_data data, evloop_callback callback)        \
	{                                                                      \
		struct evloop_op_##name op = {0};                              \
		op.base.code = EVLOOP_OP_##NAME;                               \
		op.base.callback = callback;                                   \
		op.data = data;                                                \
		op.private.task.next = NULL;                                   \
		op.private.task.work = evloop_op_work;                         \
		return op;                                                     \
	}

struct evloop_private {
	struct evloop *loop;
	struct tpool_task task;
	struct mpsc_node node;
};

/**
 * Main function of thread pool threads.
 */
void evloop_op_work(struct tpool_task *task);

/**
 * Noop I/O operation. This is mainly used to measure overhead of the loop.
 */
struct evloop_op_noop {
	struct evloop_op_base base;
	struct evloop_private private;
};

struct evloop_op_noop evloop_op_noop_init(evloop_callback callback)
{
	struct evloop_op_noop op = {0};
	op.base.code = EVLOOP_OP_NOOP;
	op.base.callback = callback;
	op.private.task.next = ((void *)0);
	op.private.task.work = evloop_op_work;
	return op;
}

/**
 * Sleep I/O operation completes after `ms` milliseconds.
 */
struct OP(sleep);
struct evloop_op_sleep evloop_op_sleep_init(struct evloop_sleep_data data,
					    evloop_callback callback)
{
	struct evloop_op_sleep op = {0};
	op.base.code = EVLOOP_OP_SLEEP;
	op.base.callback = callback;
	op.data = data;
	return op;
}

/**
 * Do not use directly, use evloop_queue instead.
 */
int evloop_queue_op(struct evloop *l, void *op);
int evloop_queue_op(struct evloop *l, void *op)
{
	// Cast as noop to access private field.
	struct evloop_op_noop *noop = op;
	noop->private.loop = l;
	noop->private.task.next = NULL;
	noop->private.task.next = NULL;
	noop->private.task.work = evloop_op_work;

	struct tpool_batch batch = tpool_batch_from_task(&noop->private.task);
	tpool_batch_push(&l->batch, batch);

	return 0;
}

#endif /* EVLOOP_TPOOL_H_INCLUDE */
