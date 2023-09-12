#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "../../vendor/httplib/httplibrary.h"
#include "methodFunction.h"
#include "../utils/utils.h"

void GETFunction(http_event* e) {
    char tempPath[1024];
    struct stat s;

    if (e->headers.path[0] != '/' || e->headers.path[1] == '\0') {
        http_send_status(e, 302, "Found");
        http_send_header(e, "Location", "/index.html");
        return;
    }

    CatchMessage(tempPath, "html%s", e->headers.path);

    if (http_send_file(e, tempPath) == -1) {
        http_send_status(e, 404, "Not Found");
        if (stat("html/404/index.html", &s) != -1) {
            http_send_header(e, "Content-Type", "text/html");
            http_send_file(e, "html/404/index.html");
        } else {
            http_send_header(e, "Content-Type", "text/plain");
            http_write(e, "Not Found", 0);
        }
    }

    return;
}
