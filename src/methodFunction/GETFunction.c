#include <stdio.h> 
#include <string.h>
#include <stdlib.h>

#include "../../vendor/sandbird/sandbird.h"
#include "../utils/utils.h"
#include "../../vendor/sqlite3/sqlite3.h"

#include "methodFunction.h"

int GETFunction(sb_Event* e) {
    char tempPath[1024];
    int res = 0;

    if (e->path[0] == '\0') {
        sb_send_status(e->stream, 302, "Found");
        sb_send_header(e->stream, "Location", "/dashboard");
        return SB_RES_OK;
    }

    if (findCharNum(e->path, '.') == -1) {
        sqlite3* db;
        sqlite3_stmt* statement;
        char username[65];
        char password[65];

        sb_get_cookie(e->stream, "username", username, 64);
        sb_get_cookie(e->stream, "password", password, 64);

        sqlite3_open("database/user.db", &db);
        sqlite3_prepare_v2(db, "SELECT 1 FROM user_table WHERE username = ? AND password = ?", -1, &statement, NULL);
        
        sqlite3_bind_text(statement, 1, username, -1, SQLITE_STATIC);
        sqlite3_bind_text(statement, 2, password, -1, SQLITE_STATIC);

        char res = sqlite3_step(statement) != SQLITE_ROW;
        
        sqlite3_finalize(statement);
        sqlite3_close(db);
        
        if (res && !isStr(e->path, "/login", 1)) {
            sb_send_status(e->stream, 302, "Found");
            sb_send_header(e->stream, "Location", "/login");
            return SB_RES_OK;
        }

        static_string_format(tempPath, "html%s.html", e->path);
        res = sb_send_file(e->stream, tempPath, 1, "text/html");
    } else {
        static_string_format(tempPath, "html%s", e->path);
        res = sb_send_file(e->stream, tempPath, 1, NULL);
    }

    if (res == SB_ECANTOPEN) {
        sb_send_status(e->stream, 404, "Not Found");
        if (sb_send_file(e->stream, "html/404/index.html", 0, 0) == SB_ECANTOPEN) {
            sb_send_header(e->stream, "Content-Type", "text/plain");
            sb_writef(e->stream, "Not Found");
        }
    }
    return SB_RES_OK;
}