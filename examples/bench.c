#include "varnish.h"
#include <assert.h>
#include <malloc.h>
#include <stdio.h>

static void
handle_get(const char *url, const char *arg, int a, int b)
{
	set_cacheable(0, 1.0f);
	static const char ctype[] = "text/plain";
	static const char data[] = "Hello World!";
	backend_response(200, ctype, sizeof(ctype)-1, data, sizeof(data)-1);
}

int main(int argc, char **argv)
{
	printf("Hello from '%s'!\n", argv[1]);
	fflush(stdout);

	set_backend_get(handle_get);
	wait_for_requests();
}
