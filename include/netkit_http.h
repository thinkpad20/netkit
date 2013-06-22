#include "../include/netkit.h"
#include "../include/strlist.h"

typedef struct http_request_s {
	List initial_headers,
		 general_headers,
		 request_headers,
		 entity_headers;
} http_request_t;

typedef struct kv_pair_s {
	char *key, *value;
} kv_pair_t;

typedef struct url_s {
	const char *path;
	List queries;
} url_t;

http_request_t *nk_parse_request(connection_t *con);
void free_request(void *req_vptr);
char *nk_lookup_key(http_request_t *req, const char *key);
void print_http_request(http_request_t *hdr);