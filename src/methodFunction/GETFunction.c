#include <stdio.h> 
#include <string.h>
#include <stdlib.h>

#include "../../vendor/sandbird/sandbird.h"
#include "methodFunction.h"
#include "../utils/utils.h"

int GETFunction(sb_Event* e) {
    int len;
    char* result;
    
    if (!strlen(e->path)) {
        sb_send_status(e->stream, 302, "Found");
        sb_send_header(e->stream, "Location", "/index.html");
        return SB_RES_OK;
    }

    if (checkFile(CatchMessage("html%s", e->path), 1)) {
        result = readFile(CatchMessage("html%s", e->path), &len, 1);
        sb_send_header(e->stream, "Content-Type", MIMETypes(e->path));
        sb_write(e->stream, result, len);
        free(result);
    }
    else {
        if (checkFile("html/404/index.html", 0)) {
            result = readFile("html/404/index.html", &len, 0);
            sb_send_status(e->stream, 404, "Not Found");
            sb_send_header(e->stream, "Content-Type", "text/html");
            sb_write(e->stream, result, len);
                    
            free(result);
        }
        else {
            sb_send_status(e->stream, 404, "Not Found");
            sb_send_header(e->stream, "Content-Type", "text/plain");
            sb_writef(e->stream, "Not Found");
        }
    }
    return SB_RES_OK;
}