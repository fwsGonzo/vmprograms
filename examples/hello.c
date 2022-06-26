#include "varnish.h"
#include <assert.h>
#include <malloc.h>
#include <stdio.h>
static void my_post_storage(size_t n, struct virtbuffer[n], size_t);
#define SILENT_START

static char data[6000];
static int  datalen = 0;

static void
handle_get(const char *url, const char *arg, int a, int b)
{
	set_cacheable(0, 1.0f);
	static const char ctype[] = "text/plain";
	backend_response(200, ctype, sizeof(ctype)-1, data, datalen);
}

static void
handle_post(const char *arg, const uint8_t *indata, size_t inlen)
{
	/* We are in a sandbox and don't need bounds-checking, but
	   it's a nice habit to check inputs. */
	if (inlen < sizeof(data)) {
		const char ctype[] = "text/plain";
		backend_response(500, ctype, sizeof(ctype)-1, "Nay", 3);
	}

	/* Send POST data to storage. */
	storage_call(my_post_storage, indata, inlen, NULL, 0);

	/* Data is a global visible to everyone
	   Once data has been modified in storage, it is
	   immediately visible in every request VM. */
	const char ctype[] = "text/plain";
	backend_response(201, ctype, sizeof(ctype)-1, data, datalen);
}

void my_post_storage(size_t n, struct virtbuffer buffers[n], size_t reslen)
{
	/* Copy POST data into our global data array */
	char* ptr = data;
	datalen = 0;
	for (size_t i = 0; i < n; i++) {
		memcpy(ptr, buffers[0].data, buffers[0].len);
		ptr += buffers[0].len;
		datalen += buffers[0].len;
	}

	storage_return_nothing();
}

static void on_live_update()
{
	/* Serialize data into ptr, len */
	storage_return(data, datalen);
}
static void on_resume_update(size_t len)
{
	assert(len < sizeof(data));
	datalen = len;
	/* Allocate room for state, return ptr, len */
	storage_return(data, sizeof(data));

	/* Do something with restored state here */
	printf("Data state restored\n");
}

int main(int argc, char **argv)
{
	datalen = snprintf(data, sizeof(data),
		"Hello World!");
#ifndef SILENT_START
	printf("Hello from '%s'! Storage=%s\n", argv[1], argv[2]);
#endif

	set_backend_get(handle_get);
	set_backend_post(handle_post);
	set_on_live_update(on_live_update);
	set_on_live_restore(on_resume_update);
	wait_for_requests();
}
