#include "varnish.h"
#include <assert.h>
#include <malloc.h>
#include <stdio.h>
static void my_storage(size_t n, struct virtbuffer[n], size_t);
static void my_post_storage(size_t n, struct virtbuffer[n], size_t);
#define SILENT_START

static char data[6000];
static int  datalen = 0;

static void nada_processing(void* arg)
{
}

static void on_client_request(const char *url)
{
	static const char hf[] = "X-KVM-Front: 1";
	// NOTE: This header field will not appear in cached results
	// For that we will have to append to RESP in DELIVER
	http_append(1, hf, sizeof(hf)-1);
	storage_return_nothing();
}

static void
handle_get(const char *arg, int a, int b)
{
//	multiprocess(8, nada_processing, NULL);
//	multiprocess_wait();
	set_cacheable(0, 1.0f);
	static const char ctype[] = "text/plain";
	backend_response(200, ctype, sizeof(ctype)-1, data, datalen);
}

static void
handle_post(const char *arg, const uint8_t *indata, size_t inlen)
{
	//char result[sizeof(data)];
	//const long rlen =
	//	storage_call(my_post_storage, indata, inlen, result, sizeof(result));

	memcpy(data, indata, inlen);
	datalen = inlen;
	assert(vmcommit() == 0);

	const char ctype[] = "text/plain";
	backend_response(201, ctype, sizeof(ctype)-1, data, datalen);
}

char* gdata = NULL;
size_t glen = 0;

static long
handle_streaming_post(const uint8_t *data, size_t len)
{
	gdata = realloc(gdata, glen + len);
	memcpy(&gdata[glen], data, len);
	glen += len;
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
void my_post_storage(size_t n, struct virtbuffer buffers[n], size_t reslen)
{
	char* ptr = data;
	datalen = 0;
	for (size_t i = 0; i < n; i++) {
		memcpy(ptr, buffers[0].data, buffers[0].len);
		ptr += buffers[0].len;
		datalen += buffers[0].len;
	}

	assert(vmcommit() == 0);

	storage_return(data, datalen);
}

__attribute__((used))
extern void on_live_update()
{
	/* Serialize data into ptr, len */
	storage_return(data, datalen);
}

__attribute__((used))
extern void on_resume_update(size_t len)
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

	set_on_recv(on_client_request);
	set_backend_get(handle_get);
	set_backend_post(handle_post);
	set_backend_stream_post(handle_streaming_post);
	wait_for_requests();
}
