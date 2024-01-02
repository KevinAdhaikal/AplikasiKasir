#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#ifdef _WIN32
    #include <winsock2.h>
    #define socklen_t int
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netdb.h>
#endif
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "../funcVarPub.h"
#include "../utils/utils.h"
#include "../../vendor/tlse/tlse.h"

#include "telegramClient.h"

int send_pending(int client_sock, struct TLSContext *context) {
    unsigned int out_buffer_len = 0;
    const unsigned char *out_buffer = tls_get_write_buffer(context, &out_buffer_len);
    unsigned int out_buffer_index = 0;
    int send_res = 0;
    while ((out_buffer) && (out_buffer_len > 0)) {
        int res = send(client_sock, (char *)&out_buffer[out_buffer_index], out_buffer_len, 0);
        if (res <= 0) {
            send_res = res;
            break;
        }
        out_buffer_len -= res;
        out_buffer_index += res;
    }
    tls_buffer_clear(context);
    return send_res;
}

int connect_server(int* sockfd) {
    struct sockaddr_in serv_addr;
    struct hostent *server;

    *sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        printf("[HTTPService Client] Error: opening socket\n");
        #ifdef _WIN32
        WSACleanup();
        #endif
        return 0;
    }

    server = gethostbyname("api.telegram.org");
    if (server == NULL) {
        printf("[HTTPService Client] Error: no such host\n");
        #ifdef _WIN32
        WSACleanup();
        #endif
        return 0;
    }

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
    serv_addr.sin_port = htons(443);

    if (connect(*sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
        printf("[HTTPService Client] Error: connecting\n");
        #ifdef _WIN32
        WSACleanup();
        #endif
        return 0;
    }

    return 1;
}

int sendMessage(void* message, const char* token, const char* userid) {
    unsigned char read_buffer[0xFFFF];
    unsigned char client_message[0xFFFF];

    int sockfd;

    #ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
    #endif

    if (token != NULL && userid != NULL) {
        printf("using here!\n");
        size_t id_len;
        char** split_id = strsplit(userid, ",", &id_len);

        for (int a = 0; a < id_len; a++) {
            if(!connect_server(&sockfd)) return 0;

            struct TLSContext *context = tls_create_context(0, TLS_V12);
            tls_make_exportable(context, 1);
            tls_client_connect(context);
            send_pending(sockfd, context);

            char* request;
            char* urlEncoded = urlEncode((char*)message);
            asprintf(&request, "GET /bot%s/sendMessage?chat_id=%s&text=%s HTTP/1.1\r\nAccept: application/json, text/plain, */*\r\nContent-Type: application/x-www-form-urlencoded\r\nUser-Agent: axios/0.27.2\r\nHost: api.telegram.org\r\nConnection: close\r\nContent-Length: 0\r\n\r\n", token, split_id[a], urlEncoded);
            free(urlEncoded);

            int read_size;
            while ((read_size = recv(sockfd, client_message, sizeof(client_message), 0)) > 0) {
                tls_consume_stream(context, client_message, read_size, NULL);
                send_pending(sockfd, context);
                if (tls_established(context)) {
                    tls_write(context, (unsigned char *)request, strlen(request));
                    send_pending(sockfd, context);
                    int tempLen = tls_read(context, read_buffer, 0xFFFF - 1);
                }
            }

            free(request);
            SSL_CTX_free(context);
            #ifdef _WIN32
            closesocket(sockfd);
            #else
            close(sockfd);
            #endif

            if (isStr(read_buffer + 9, "404", 0)) {
                #ifdef _WIN32
                WSACleanup();
                #endif
                free(split_id);
                return -1;
            }
        }

        free(split_id);
        return 1;
    }

    for (int a = 0; a < teleBot.userIDsize; a++) {
        if(!connect_server(&sockfd)) return 0;

        struct TLSContext *context = tls_create_context(0, TLS_V12);
        tls_make_exportable(context, 1);
        tls_client_connect(context);
        send_pending(sockfd, context);

        char* request;
        char* urlEncoded = urlEncode((char*)message);
        asprintf(&request, "GET /bot%s/sendMessage?chat_id=%s&text=%s HTTP/1.1\r\nAccept: application/json, text/plain, */*\r\nContent-Type: application/x-www-form-urlencoded\r\nUser-Agent: axios/0.27.2\r\nHost: api.telegram.org\r\nConnection: close\r\nContent-Length: 0\r\n\r\n", teleBot.tokenBot, teleBot.userID[a], urlEncoded);
        free(urlEncoded);

        int read_size;
        while ((read_size = recv(sockfd, client_message, sizeof(client_message), 0)) > 0) {
            tls_consume_stream(context, client_message, read_size, NULL);
            send_pending(sockfd, context);
            if (tls_established(context)) {
                tls_write(context, (unsigned char *)request, strlen(request));
                send_pending(sockfd, context);
                int tempLen = tls_read(context, read_buffer, 0xFFFF - 1);
            }
        }

        free(request);
        SSL_CTX_free(context);
        #ifdef _WIN32
        closesocket(sockfd);
        #else
        close(sockfd);
        #endif

        if (isStr(read_buffer + 9, "404", 0)) {
            #ifdef _WIN32
            WSACleanup();
            #endif
            return -1;
        }
    }

    return 1;
}

void* sendMessageThread(void* message) {
    if (!teleBot.usingTelegramBot) return NULL;
    unsigned char read_buffer[0xFFFF];
    unsigned char client_message[0xFFFF];

    int sockfd;

    #ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
    #endif

    for (int a = 0; a < teleBot.userIDsize; a++) {
        if(!connect_server(&sockfd)) return NULL;

        struct TLSContext *context = tls_create_context(0, TLS_V12);
        tls_make_exportable(context, 1);
        tls_client_connect(context);
        send_pending(sockfd, context);

        char* request;
        char* urlEncoded = urlEncode((char*)message);
        asprintf(&request, "GET /bot%s/sendMessage?chat_id=%s&text=%s HTTP/1.1\r\nAccept: application/json, text/plain, */*\r\nContent-Type: application/x-www-form-urlencoded\r\nUser-Agent: axios/0.27.2\r\nHost: api.telegram.org\r\nConnection: close\r\nContent-Length: 0\r\n\r\n", teleBot.tokenBot, teleBot.userID[a], urlEncoded);
        free(urlEncoded);

        int read_size;
        while ((read_size = recv(sockfd, client_message, sizeof(client_message), 0)) > 0) {
            tls_consume_stream(context, client_message, read_size, NULL);
            send_pending(sockfd, context);
            if (tls_established(context)) {
                tls_write(context, (unsigned char *)request, strlen(request));
                send_pending(sockfd, context);
                int tempLen = tls_read(context, read_buffer, 0xFFFF - 1);
            }
        }

        free(request);
        SSL_CTX_free(context);
        #ifdef _WIN32
        closesocket(sockfd);
        #else
        close(sockfd);
        #endif

        if (isStr(read_buffer + 9, "404", 0)) {
            #ifdef _WIN32
            WSACleanup();
            #endif
        }
    }

    free(message);

    return NULL;
}
