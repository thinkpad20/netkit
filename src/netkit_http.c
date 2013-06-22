#include "../include/netkit_http.h"

typedef struct http_option_s {
	const char *name, *val;
} http_option_t;

const char *request_headers[] = {
	"Accept",
   "Accept-Charset",
   "Accept-Encoding",
   "Accept-Language",
   "Authorization",
   "Expect",
   "From",
   "Host",
   "If-Match",
   "If-Modified-Since",
   "If-None-Match",
   "If-Range",
   "If-Unmodified-Since",
   "Max-Forwards",
   "Proxy-Authorization",
   "Range",
   "Referer",
   "TE",
   "User-Agent",
};

const char *general_headers[] = {
	"Cache-Control",
	"Connection",
	"Date",
	"Pragma",
	"Trailer",
	"Transfer-Encoding",
	"Upgrade",
	"Via",
	"Warning",
};

const char *entity_headers[] = {
	"Allow",
	"Content-Encoding",
	"Content-Language",
	"Content-Length",
	"Content-Location",
	"Content-MD5",
	"Content-Range",
	"Content-Type",
	"Expires",
	"Last-Modified",
};

const size_t n_req_hdrs = sizeof(request_headers)/sizeof(char *);
const size_t n_gen_hdrs = sizeof(general_headers)/sizeof(char *);
const size_t n_ent_hdrs = sizeof(entity_headers)/sizeof(char *);

static int contains(const char **word_list, size_t num, const char *word);
static void free_pair(void *pair_vptr);
static List read_request_line(http_request_t *req, const char *line, size_t len);
static kv_pair_t *get_header(const char *buf, size_t len);
static http_request_t *new_request(void);

static List 
read_request_line(http_request_t *req, const char *line, size_t len) {
	printf("Parsing request line: %.*s\n", (int)len, line);
	List res;
	strlist_t words = sl_split(line, " ", len);
	list_init(&res, free_pair);
	kv_pair_t *mhdr = (kv_pair_t *)malloc(sizeof(kv_pair_t)),
				*uhdr = (kv_pair_t *)malloc(sizeof(kv_pair_t)),
				*vhdr = (kv_pair_t *)malloc(sizeof(kv_pair_t));
	mhdr->key = strdup("Method"); mhdr->value = sl_dequeue(&words);
	uhdr->key = strdup("URL"); uhdr->value = sl_dequeue(&words);
	vhdr->key = strdup("Version"); vhdr->value = sl_dequeue(&words);
	assert(mhdr->value && "Missing HTTP method");
	assert(uhdr->value && "Missing url");
	assert(vhdr->value && "Missing HTTP version");
	list_addBack(&res, mhdr);
	list_addBack(&res, uhdr);
	list_addBack(&res, vhdr);
	return res;
}

static
kv_pair_t *get_header(const char *buf, size_t len) {
	strlist_t pair = sl_split(buf, " ", len);
	kv_pair_t *hdr = (kv_pair_t *)malloc(sizeof(kv_pair_t));
	char *key = sl_dequeue(&pair);
	/* remove : from key */
	if (key) key[strlen(key) - 1] = '\0';
	hdr->key = key;
	hdr->value = sl_dequeue(&pair);
	if (!hdr->key || !hdr->value) { free(hdr); return NULL; }
	// printf("Found key/value pair: '%s': '%s'\n", hdr->key, hdr->value);
	return hdr;
}

static http_request_t *
new_request(void) {
	http_request_t *req = (http_request_t *)calloc(1, sizeof(http_request_t));
	list_init(&req->general_headers, free);
	list_init(&req->request_headers, free);
	list_init(&req->entity_headers, free);
	assert(req && "Error on memory allocation");
	return req;
}

void
free_request(void *req_vptr) {
	http_request_t *req = (http_request_t *)req_vptr;
	list_destroy(&req->initial_headers);
	list_destroy(&req->general_headers);
	list_destroy(&req->request_headers);
	list_destroy(&req->entity_headers);
	free(req);
}

/* for passing to list_findByString */
static void
str_to_str(char *buf, void *data) {
	const char *value = ((kv_pair_t *)data)->value;
	strcpy(buf, value);
}

char *
nk_lookup_key(http_request_t *req, const char *key) {
	char *res;
	return (res = list_findByString(&req->initial_headers, str_to_str, key)) ?
	 		res : (res = list_findByString(&req->general_headers, str_to_str, key)) ?
			res : (res = list_findByString(&req->request_headers, str_to_str, key)) ?
			res : (res = list_findByString(&req->entity_headers, str_to_str, key)) ?
			res : NULL;
}

static void 
free_pair(void *pr) {
	kv_pair_t *pair = (kv_pair_t *)pr;
	free(pair->key);
	free(pair->value);
	free(pair);
}

http_request_t *
nk_parse_request(connection_t *con) {
	http_request_t *req = new_request();
	int n, first_line = 1;
	char *buf = (char *)malloc(MAX_BUF);
	assert(buf && "Error on memory allocation");
	while (1) {
		n = nk_recv_crlf(con, buf, MAX_BUF);
		if (n <= 0) break;
		if (first_line) { 
			req->initial_headers = read_request_line(req, buf, n);
			first_line = 0; 
		} else {
			kv_pair_t *hdr;
			if (!strncmp(buf, "\r\n", n)) break;
			hdr = get_header(buf, n);
			if (hdr) {
				if (contains(general_headers, n_gen_hdrs, hdr->key))
					list_addBack(&req->general_headers, hdr);
				else if (contains(request_headers, n_req_hdrs, hdr->key))
					list_addBack(&req->request_headers, hdr);
				else if (contains(entity_headers, n_ent_hdrs, hdr->key))
					list_addBack(&req->entity_headers, hdr);
				else
					puts("Unknown header key.");
			}
		}
	}
	return req;
}

http_request_t *
_nk_parse_request_list(const char *str) {
	strlist_t lines = sl_split(str, "\r\n", strlen(str));
	http_request_t *req = new_request();
	int first_line = 1;
	char *line;
	while (sl_size(&lines) > 0) {
		line = sl_dequeue(&lines);
		if (first_line) { 
			read_request_line(req, line, strlen(line)); 
			first_line = 0; 
		} else {
			kv_pair_t *hdr = get_header(line, strlen(line));
			if (hdr) {
				if (contains(general_headers, n_gen_hdrs, hdr->key))
					list_addBack(&req->general_headers, hdr);
				else if (contains(request_headers, n_req_hdrs, hdr->key))
					list_addBack(&req->request_headers, hdr);
				else if (contains(entity_headers, n_ent_hdrs, hdr->key))
					list_addBack(&req->entity_headers, hdr);
				else
					puts("Unknown header key.");
			}
		}
	}
	return req;
}

void 
print_http_request(http_request_t *hdr) {
	ListNode *node;
	if (hdr) {
		printf("Request line:\n");
		for (node = hdr->initial_headers.front; node; node = node->next) {
			printf("\t%s: %s\n", ((kv_pair_t *)node->data)->key,
										((kv_pair_t *)node->data)->value);
		}

		printf("General headers:\n");
		for (node = hdr->general_headers.front; node; node = node->next) {
			printf("\t%s: %s\n", ((kv_pair_t *)node->data)->key,
										((kv_pair_t *)node->data)->value);
		}
		printf("Request headers:\n");
		for (node = hdr->request_headers.front; node; node = node->next) {
			printf("\t%s: %s\n", ((kv_pair_t *)node->data)->key,
										((kv_pair_t *)node->data)->value);
		}
		printf("Entity headers:\n");
		for (node = hdr->entity_headers.front; node; node = node->next) {
			printf("\t%s: %s\n", ((kv_pair_t *)node->data)->key,
										((kv_pair_t *)node->data)->value);
		}
	}
}

static int
contains(const char **word_list, size_t num, const char *word) {
	int i;
	for (i=0; i<num; ++i) {
		if (!strcmp(word_list[i], word))
			return 1;
	}
	return 0;
}

// int main(int argc, char const *argv[])
// {
// 	print_http_request(_nk_parse_request("GET / HTTP/1.1\r\n"
// 		"Host: localhost:7890\r\n"
// 		"Connection: keep-alive\r\n"
// 		"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
// 		"User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_8_4) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/27.0.1453.116 Safari/537.36\r\n"
// 		"Accept-Encoding: gzip,deflate,sdch\r\n"
// 		"Accept-Language: en-US,en;q=0.8\r\n\r\n"));
// 	return 0;
// }