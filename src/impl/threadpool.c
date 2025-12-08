#include "eventloop.h"

#define THREADPOOL_IMPLEMENTATION
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
};

int evloop_init(struct evloop *l, struct evloop_config cfg)
{
	tpool_init(&l->tpool, cfg.tpool);
	return 0;
}

int evloop_deinit(struct evloop *l)
{
	tpool_deinit(&l->tpool);
	return 0;
}
