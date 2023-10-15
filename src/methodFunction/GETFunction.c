#include <stdio.h> 
#include <string.h>
#include <stdlib.h>

#include "../../vendor/sandbird/sandbird.h"
#include "methodFunction.h"
#include "../utils/utils.h"

int GETFunction(sb_Event* e) {
    char tempPath[1024];

    if (e->path[0] == '\0') {
        sb_send_status(e->stream, 302, "Found");
        sb_send_header(e->stream, "Location", "/index.html");
        return SB_RES_OK;
    }

    if (findStrLast(e->path, ".html")) {
        char username[20];
        char password[20];

        sb_get_cookie(e->stream, "username", username, 19);
        sb_get_cookie(e->stream, "password", password, 19);

        if ((!isStr(username, "admin", 1) && !isStr(password, "admin", 1)) && !isStr(e->path, "/login.html", 1)) {
            sb_send_status(e->stream, 302, "Found");
            sb_send_header(e->stream, "Location", "/login.html");
            return SB_RES_OK;
        }
    }

    CatchMessage(tempPath, "html%s", e->path);

    if (sb_send_file(e->stream, tempPath, 1) == SB_ECANTOPEN) {
        sb_send_status(e->stream, 404, "Not Found");
        if (sb_send_file(e->stream, "html/404/index.html", 0) == SB_ECANTOPEN) {
            sb_send_header(e->stream, "Content-Type", "text/plain");
            sb_writef(e->stream, "Not Found");
        }
    }
    return SB_RES_OK;
}
