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

/**
 * Initializes the event loop with the given configuration.
 * A negative errno is returned in case of error.
 */
int evloop_init(struct evloop *l, struct evloop_config cfg);

/**
 * Deinitializes the event loop and clean up associated resources.
 */
int evloop_deinit(struct evloop *l);
