#include <stdatomic.h>
#include <stdio.h>

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
	} while (0);

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

int main(void)
{
	TRY(init_deinit)
	printf("all tests are OK\n");
	return 0;
}
