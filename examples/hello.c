#include "api.h"
#include <assert.h>
#include <malloc.h>
#include <stdio.h>
static void my_storage(size_t n, struct virtbuffer[n], size_t);

int main(int argc, char **argv)
{
	printf("Hello from '%s'! Storage=%s\n", argv[1], argv[2]);
}

__attribute__((used))
extern void my_backend(const char *arg)
{
	const char data[] = "Hello World!";
	char result[256];
	const long rlen =
		storage_call(my_storage, data, sizeof(data), result, sizeof(result));

	const char ctype[] = "text/plain";
	backend_response(200, ctype, sizeof(ctype)-1, result, rlen-1);
	//backend_response(404, NULL, 0, NULL, 0);
}

extern void __attribute__((used))
my_post_backend(const char *arg, void *data, size_t len)
{
	const char ctype[] = "text/plain";
	backend_response(201, ctype, sizeof(ctype)-1, data, len);
}

char* gdata = NULL;

extern long __attribute__((used))
my_streaming_function(void *data, size_t len, size_t processed, int last)
{
	gdata = realloc(gdata, processed + len);
	memcpy(&gdata[processed], data, len);
	return len; /* Required */
}
extern void __attribute__((used))
my_streaming_response(const char *arg, size_t len)
{
	char result[512];
	int bytes = snprintf(result, sizeof(result),
		"Streaming ended, len=%zu", len);

	const char ctype[] = "text/plain";
	backend_response(201, ctype, sizeof(ctype)-1, gdata, len);
}

static int counter = 0;
void my_storage(size_t n, struct virtbuffer buffers[n], size_t reslen)
{
	struct virtbuffer *hello_string = &buffers[0];
	counter ++;
	((char *)hello_string->data)[11] = '0' + (counter % 10);
	/* Data contains the inputs */
	storage_return(hello_string->data, hello_string->len);
}

__attribute__((used))
extern void on_live_update()
{
	/* Serialize data into ptr, len */
	int *data = (int *)malloc(sizeof(counter));
	*data = counter;
	storage_return(data, sizeof(*data));
}

__attribute__((used))
extern void on_resume_update(size_t len)
{
	assert(len == sizeof(counter));
	/* Allocate room for state, return ptr, len */
	storage_return(&counter, sizeof(counter));

	/* Restore state here */
	printf("Counter state restored: %d\n", counter);
}
