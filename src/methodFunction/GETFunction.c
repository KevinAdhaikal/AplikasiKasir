#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "../../vendor/sandbird/sandbird.h"
#include "methodFunction.h"
#include "../utils/utils.h"

int GETFunction(sb_Event* e) {
    char tempPath[1024];
    struct stat s;
    
    if (e->path[0] == '\0') {
        sb_send_status(e->stream, 302, "Found");
        sb_send_header(e->stream, "Location", "/index.html");
        return SB_RES_OK;
    }

    CatchMessage(tempPath, "html%s", e->path);

    if (stat(tempPath, &s) != -1) {
        sb_send_header(e->stream, "Content-Type", MIMETypes(e->path));
        sb_send_file(e->stream, tempPath);
    }
    else {
        sb_send_status(e->stream, 404, "Not Found");
        if (stat(tempPath, &s) != -1) {
            sb_send_header(e->stream, "Content-Type", "text/html");
            sb_send_file(e->stream, "html/404/index.html");
        }
        else {
            sb_send_header(e->stream, "Content-Type", "text/plain");
            sb_writef(e->stream, "Not Found");
        }
    }
    return SB_RES_OK;
}