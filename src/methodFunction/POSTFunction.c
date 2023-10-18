#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "../../vendor/sandbird/sandbird.h"
#include "methodFunction.h"
#include "../utils/utils.h"
#include "../sqliteFunction.h"
#include "../CashierFunction/cashierFunction.h"

int POSTFunction(sb_Event* e) {
    if (!strlen(e->path)) return SB_RES_CLOSE;

    if (isStr(e->path, "/userLogin", 1)) {
        char username[20], password[20];

        sb_get_var(e->stream, "username", username, 19);
        sb_get_var(e->stream, "password", password, 19);
        
        if (isStr(username, "admin", 1) && isStr(password, "admin", 1)) sb_send_status(e->stream, 200, "OK");
        else sb_send_status(e->stream, 403, "Forbidden");
    }
    else {
        char username[20], password[20];

        sb_get_cookie(e->stream, "username", username, 19);
        sb_get_cookie(e->stream, "password", password, 19);

        if (isStr(username, "admin", 1) && isStr(password, "admin", 1)) {
            if (isStr(e->path, "/checkLogin", 1)) sb_send_status(e->stream, 200, "OK");
            else if (isStr(e->path, "/tambahBarang", 1)) tambahBarang(e);
            else if (isStr(e->path, "/daftarBarang", 1)) daftarBarang(e);
            else if (isStr(e->path, "/hapusDaftarBarang", 1)) hapusDaftarBarang(e);
            else if (isStr(e->path, "/infoBarang", 1)) infoBarang(e);
            else if (isStr(e->path, "/editBarang", 1)) editBarang(e);
            else if (isStr(e->path, "/cashierFindBarang", 1)) cashierFindBarang(e);
            else if (isStr(e->path, "/cashierSettings", 1)) cashierSettings(e);
            else if (isStr(e->path, "/cashierStockChecker", 1)) cashierStockChecker(e);
            else if (isStr(e->path, "/pembukuan", 1)) pembukuan(e);
            else if (isStr(e->path, "/dashboardLogic", 1)) dashboardLogic(e);
            else if (isStr(e->path, "/barangMasukLogic", 1)) barangMasukLogic(e);
            else if (isStr(e->path, "/pengaturan", 1)) pengaturan(e);
            else sb_send_status(e->stream, 404, "Not Found");
        }
        else sb_send_status(e->stream, 403, "Forbidden");
    }

    return SB_RES_OK;
}