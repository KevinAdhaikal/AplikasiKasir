#ifndef HTTPLIBRARY_H
#define HTTPLIBRARY_H

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/select.h>
#endif

typedef struct {
    char method[16];
    char path[128];
    char query[512];
    short question_pos;
    char* raw_header;
    int raw_len;
    int body_pos;
} http_header;

typedef struct {
    char *data;
    int len;
    int capacity;
} http_buffer;

typedef struct {
    char state;
    int client_sock;
    http_header headers;
    http_buffer server_buffer;
} http_event;

typedef void (*http_callback)(http_event*);

typedef struct {
    http_callback callback;
    int client_socket;
    fd_set read_fds;
} http_thread;

int http_init(short port);
void http_start(int server_fd, http_callback callback);
void http_buffer_init(http_event* e, int initial_capacity);
void http_buffer_resize(http_buffer* b, int new_capacity);
void http_buffer_append(http_event* e, const char *data, int len);
void http_buffer_free(http_buffer* b);
void http_send_status(http_event* e, int status, const char *val);
void http_send_header(http_event* e, const char *name, const char *val);
void http_write(http_event* e, const char *data, int len);
void http_get_header(http_event* e, const char* header_name, char* dest, size_t dest_len);
void http_get_cookie(http_event* e, const char *cookie_name, char *dest, size_t dest_len);
int http_send_file(http_event* e, const char* filename, char using_cache);
void http_get_query(http_event* e, const char* param, char* value, size_t value_size);
int http_get_query_to_int(http_event* e, const char* param);

#endif // HTTPLIBRARY_H
