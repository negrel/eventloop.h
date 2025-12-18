/**
 * Event loop configuration options.
 */
struct evloop_config;

/**
 * Event loop.
 */
struct evloop;

/**
 * Event loop I/O operation code.
 */
enum evloop_op_code {
	EVLOOP_OP_NOOP,
	EVLOOP_OP_SLEEP,
};

struct evloop_op_base;

/**
 * An asynchronous I/O operation completion callback.
 */
typedef void (*evloop_callback)(struct evloop *l, struct evloop_op_base *op);

/**
 * Base struct for thread pool I/O operation. It is always safe to cast an
 * evloop_op_xxx pointer to this type. Casting this type into an evloop_op_xxx
 * pointer is safe if you check the code field before.
 */
struct evloop_op_base {
	enum evloop_op_code code;
	evloop_callback callback;
};

/**
 * No op I/O operation. This is mainly used to measure overhead of submitting an
 * operation.
 */
struct evloop_op_noop;

/**
 * Sleep I/O operation with millisecond resolution.
 */
struct evloop_op_sleep;
struct evloop_sleep_data {
	unsigned int ms;
};

/**
 * Initializes the event loop with the given configuration.
 * A negative errno is returned in case of error.
 */
int evloop_init(struct evloop *l, struct evloop_config cfg);

/**
 * Deinitializes the event loop and clean up associated resources.
 * A negative errno is returned in case of error.
 */
int evloop_deinit(struct evloop *l);

/**
 * Queue I/O operation to be submitted on next evloop_poll(). This function is
 * implemented as a _Generic macros or function by the underlying
 * implementation.
 *
 * int evloop_queue(struct evloop *l, struct evloop_op_xxx *op);
 */
#define evloop_queue(evloop, op)                                               \
	_Generic(op,                                                           \
	    struct evloop_op_noop *: evloop_queue_op(evloop, op),              \
	    struct evloop_op_sleep *: evloop_queue_op(evloop, op))

/**
 * Event loop poll modes:
 * * EVLOOP_POLL_ALL: poll all pending I/O completions.
 * * EVLOOP_POLL_ONE: poll at least one I/O completions.
 * * EVLOOP_POLL_NOWAIT: poll all completed I/O completions.
 */
enum evloop_poll_mode {
	EVLOOP_POLL_ALL,
	EVLOOP_POLL_ONE,
	EVLOOP_POLL_NOWAIT,
};

/**
 * Poll pending I/O operations for completion and then submit all queued I/O
 * operations. To process all I/O operations you can call evloop_poll with
 * EVLOOP_POLL_ALL twice.
 * A negative errno is returned in case of error.
 */
int evloop_poll(struct evloop *l, enum evloop_poll_mode mode);

#undef OP_INIT
#undef OP_QUEUE
