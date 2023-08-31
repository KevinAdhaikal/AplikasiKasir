#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#endif

#include "httplibrary.h"

typedef void (*http_callback)(http_event*);

int find_crlfcrlf_num(const char* str) {
    const char* result = strstr(str, "\r\n\r\n");
    if (result != NULL) return result - str;
    return -1;
}

int find_char_num(const char* str, char ch_find) {
    if (!str) return -1;
    for (int a = 0; a < strlen(str); a++) {
        if (str[a] == ch_find) return a;
    }
    return -1;
}

void http_buffer_init(http_event* e, int initial_capacity) {
    http_buffer* b = &e->server_buffer;
    b->data = (char *)malloc(initial_capacity);
    if (!b->data) {
        perror("Memory allocation error");
        return;
    }
    b->len = 0;
    b->capacity = initial_capacity;
    e->state++;
}

void http_buffer_resize(http_buffer* b, int new_capacity) {
    char *new_data = (char *)realloc(b->data, new_capacity);
    if (!new_data) {
        perror("Memory allocation error");
        return;
    }

    b->data = new_data;
    b->capacity = new_capacity;
}

void http_buffer_append(http_event* e, const char *data, int len) {
    if (e->state == 0) http_buffer_init(e, 1024);
    http_buffer* b = &e->server_buffer;

    if (b->len + len > b->capacity) {
        int new_capacity = b->capacity == 0 ? len : b->capacity * 2;
        while (b->len + len > new_capacity) {
            new_capacity *= 2;
        }
        http_buffer_resize(b, new_capacity);
    }

    memcpy(b->data + b->len, data, len + 1);
    b->len += len;
}

void http_buffer_free(http_buffer* b) {
    free(b->data);
    b->data = NULL;
    b->len = 0;
    b->capacity = 0;
}

void http_send_status(http_event* e, int status, const char *val) {
    if (e->state == 0) http_buffer_init(e, 1024);
    char response[128];
    int response_length = snprintf(response, sizeof(response), "HTTP/1.1 %d %s\r\n", status, val);
    http_buffer_append(e, response, response_length);
    e->state++;
}

void http_send_header(http_event* e, const char *name, const char *val) {
    if (e->state == 0) http_buffer_init(e, 1024);
    if (e->state == 1) http_send_status(e, 200, "OK");
    char header[256];
    int header_length = snprintf(header, sizeof(header), "%s: %s\r\n", name, val);
    http_buffer_append(e, header, header_length);
    if (e->state < 1) e->state++;
}

void http_write(http_event* e, const char *data, int len) {
    if (e->state == 0) http_buffer_init(e, 1024);
    if (e->state == 1) http_send_status(e, 200, "OK");
    if (e->state == 2) http_buffer_append(e, "\r\n", 2), e->state++;
    http_buffer_append(e, data, !len ? strlen(data) : len);
}

int http_send_file(http_event* e, const char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) return -1;
    else {
        char toString[32];
        fseek(fp, 0, SEEK_END);
        if (e->state == 0) http_buffer_init(e, 1024);
        if (e->state == 1) http_send_status(e, 200, "OK");
        sprintf(toString, "%u", (unsigned)ftell(fp));

        http_send_header(e, "Content-Length", toString);
        http_buffer_append(e, "\r\n", 2);
        fseek(fp, 0, SEEK_SET);

        char tempBuffer[1024];
        size_t bufferSize;
        while(bufferSize = fread(tempBuffer, 1, 1024, fp)) http_buffer_append(e, tempBuffer, bufferSize);

        send(e->client_sock, e->server_buffer.data, e->server_buffer.len, 0);
        http_buffer_free(&e->server_buffer);
        fclose(fp);
    }

    return 1;
}

void http_get_header(http_event* e, const char* header_name, char* dest, size_t dest_len) {
    const char* header_start = strstr(e->headers.raw_header, header_name);

    if (header_start) {
        header_start += strlen(header_name);
        header_start = strchr(header_start, ':');
        if (header_start) {
            header_start++;
            while (*header_start == ' ') header_start++;
            const char* header_end = strchr(header_start, '\r');
            if (header_end) {
                size_t value_len = header_end - header_start;
                if (value_len < dest_len) {
                    strncpy(dest, header_start, value_len);
                    dest[value_len] = '\0';
                }
            }
        }
    }
}

void http_get_cookie(http_event* e, const char *cookie_name, char *dest, size_t dest_len) {
    const char *cookie_start = strstr(e->headers.raw_header, cookie_name);

    if (cookie_start) {
        cookie_start += strlen(cookie_name);

        if (cookie_start[0] == '=') {
            cookie_start++;

            const char *cookie_value_end = strchr(cookie_start, ';');

            if (!cookie_value_end) cookie_value_end = strchr(cookie_start, '\r');

            if (cookie_value_end) {
                size_t cookie_value_len = cookie_value_end - cookie_start;

                if (cookie_value_len < dest_len) {
                    strncpy(dest, cookie_start, cookie_value_len);
                    dest[cookie_value_len] = '\0';
                    return;
                }
            }
        }
    }

    dest[0] = '\0';
}

void http_get_query(http_event* e, const char* param, char* value, size_t value_size) {
    if (!e->headers.question_pos) return;
    char* query = e->headers.path + e->headers.question_pos;
    char query_copy[strlen(query) + 1];
    memcpy(query_copy, query, strlen(query) + 1);

    char* token = strtok(query_copy, "&");
    while (token != NULL) {
        if (strstr(token, param) == token) {
            const char* value_start = strchr(token, '=');
            if (value_start != NULL) {
                strncpy(value, value_start + 1, value_size);
                value[value_size - 1] = '\0';
                return;
            }
        }
        token = strtok(NULL, "&");
    }

    value[0] = '\0';
}

int http_get_query_to_int(http_event* e, const char* param) {
    char temp_query[5];
    http_get_query(e, param, temp_query, 4);
    return atoi(temp_query);
}

void *handle_client(void *arg) {
    http_thread* thread_data = (http_thread*)arg;
    http_event event = {0};
    char temp_req[4096];
    int temp_len = 0;

    while(1) {
        temp_len = recv(thread_data->client_socket, temp_req, 4095, 0);
        if (temp_len == 0) {
            if (event.headers.raw_header) free(event.headers.raw_header);
            #ifdef _WIN32
            closesocket(thread_data->client_socket);
            #else
            close(thread_data->client_socket);
            #endif
            free(thread_data);
            return NULL;
        } else if (temp_len == -1) break;

        event.headers.raw_header = realloc(event.headers.raw_header, event.headers.raw_len + temp_len + 1);
        memcpy(event.headers.raw_header + event.headers.raw_len, temp_req, temp_len + 1);
        event.headers.raw_len += temp_len;
    }

    if (event.headers.raw_len > 0) {
        int method_len = find_char_num(event.headers.raw_header, ' ');
        int path_len = find_char_num(event.headers.raw_header + method_len + 1, ' ');
        memcpy(event.headers.method, event.headers.raw_header, method_len);
        memcpy(event.headers.path, event.headers.raw_header + method_len + 1, path_len);
        event.headers.method[method_len] = '\0';
        event.headers.path[path_len] = '\0';
        event.headers.question_pos = find_char_num(event.headers.path, '?') + 1;
        if (event.headers.question_pos) event.headers.path[event.headers.question_pos - 1] = '\0';
        event.headers.body_pos = find_crlfcrlf_num(event.headers.raw_header);
    } else {
        if (event.headers.raw_header) free(event.headers.raw_header);
        #ifdef _WIN32
        closesocket(thread_data->client_socket);
        #else
        close(thread_data->client_socket);
        #endif
        free(thread_data);
        return NULL;
    }

    event.client_sock = thread_data->client_socket;
    thread_data->callback(&event);

    if (event.server_buffer.len) {
        send(event.client_sock, event.server_buffer.data, event.server_buffer.len, 0);
        http_buffer_free(&event.server_buffer);
    }

    if (event.headers.raw_header) free(event.headers.raw_header);

    #ifdef _WIN32
    closesocket(thread_data->client_socket);
    #else
    close(thread_data->client_socket);
    #endif

    free(thread_data);

    return NULL;
}

int http_init(short port) {
    #ifdef _WIN32
    WSADATA dat;
    if (WSAStartup(MAKEWORD(2, 2), &dat) != 0) {
        perror("WSAStartup failed");
        return -1;
    }
    #endif

    int server_socket;
    struct sockaddr_in server_addr;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        return -1;
    }

    #ifdef _WIN32
    u_long mode = 1;
    if (ioctlsocket(server_socket, FIONBIO, &mode) != 0) {
        perror("ioctlsocket failed");
        return -1;
    }
    #else
    if (fcntl(server_socket, F_SETFL, fcntl(server_socket, F_GETFL, 0) | O_NONBLOCK) == -1) {
        perror("fcntl failed");
        return -1;
    }
    #endif

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Socket binding failed");
        return -1;
    }

    if (listen(server_socket, 10) == -1) {
        perror("Listening failed");
        return -1;
    }

    return server_socket;
}

void http_start(int server_socket, http_callback callback) {
    fd_set read_fds, master_fds;
    int max_fd;

    FD_ZERO(&master_fds);
    FD_SET(server_socket, &master_fds);
    max_fd = server_socket;

    while (1) {
        read_fds = master_fds;

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            break;
        }

        if (FD_ISSET(server_socket, &read_fds)) {
            int client_socket = accept(server_socket, NULL, NULL);
            if (client_socket > 0) {
                FD_SET(client_socket, &master_fds);
                if (client_socket > max_fd) max_fd = client_socket;
            }

            #ifndef _WIN32
            fcntl(client_socket, F_SETFL, fcntl(client_socket, F_GETFL, 0) | O_NONBLOCK); // non blocking untuk client
            #endif
        }

        for (int fd = 0; fd <= max_fd; fd++) {
            if (FD_ISSET(fd, &read_fds)) {
                if (fd == server_socket) continue;

                pthread_t thread;
                http_thread* thread_arg = malloc(sizeof(http_thread));
                thread_arg->client_socket = fd;
                thread_arg->callback = callback;
                pthread_create(&thread, NULL, handle_client, thread_arg);
                pthread_detach(thread);
                FD_CLR(fd, &master_fds);

            }
        }
    }

    for (int fd = 0; fd <= max_fd; fd++) {
        if (FD_ISSET(fd, &master_fds)) {
            close(fd);
        }
    }

    close(server_socket);

    #ifdef _WIN32
    WSACleanup();
    #endif
}
