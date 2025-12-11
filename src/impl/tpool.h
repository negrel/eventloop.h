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

/**
 * Sleep I/O operation completes after `ms` milliseconds.
 */
struct OP(sleep);

/**
 * Do not use directly, use evloop_queue instead.
 */
int evloop_queue_op(struct evloop *l, void *op);
#endif /* EVLOOP_TPOOL_H_INCLUDE */
