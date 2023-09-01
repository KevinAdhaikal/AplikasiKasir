#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "../../vendor/httplib/httplibrary.h"
#include "methodFunction.h"
#include "../utils/utils.h"
#include "../sqliteFunction.h"
#include "../CashierFunction/cashierFunction.h"
#include "../teleFunction/teleFunction.h"

void POSTFunction(http_event* e) {
    if (e->headers.path[0] != '/' || e->headers.path[1] == '\0') return;

    if (isStr(e->headers.path, "/userLogin", 1)) {
        char username[20], password[20];

        http_get_query(e, "username", username, 20);
        http_get_query(e, "password", password, 20);
        
        if (isStr(username, "admin", 1) && isStr(password, "admin", 1)) http_send_status(e, 200, "OK");
        else http_send_status(e, 403, "Forbidden");
    }
    else {
        char username[20], password[20];

        http_get_cookie(e, "username", username, 20);
        http_get_cookie(e, "password", password, 20);

        if (isStr(username, "admin", 1) && isStr(password, "admin", 1)) {
            switch(http_get_query_to_int(e, "teleArgs")) {
                case 1: return teleTotalPembukuan(e);
                case 2: return teleKasir(e);
                default: break;
            }

            if (isStr(e->headers.path, "/checkLogin", 1)) http_send_status(e, 200, "OK");
            else if (isStr(e->headers.path, "/tambahBarang", 1)) tambahBarang(e);
            else if (isStr(e->headers.path, "/daftarBarang", 1)) daftarBarang(e);
            else if (isStr(e->headers.path, "/hapusDaftarBarang", 1)) hapusDaftarBarang(e);
            else if (isStr(e->headers.path, "/infoBarang", 1)) infoBarang(e);
            else if (isStr(e->headers.path, "/editBarang", 1)) editBarang(e);
            else if (isStr(e->headers.path, "/cashierFindBarang", 1)) cashierFindBarang(e);
            else if (isStr(e->headers.path, "/pembukuan", 1)) pembukuan(e);
            else if (isStr(e->headers.path, "/dashboardLogic", 1)) dashboardLogic(e);
            else if (isStr(e->headers.path, "/barangMasukLogic", 1)) barangMasukLogic(e);
            else if (isStr(e->headers.path, "/pengaturan", 1)) pengaturan(e);
            else http_send_status(e, 404, "Not Found");
            
        }
        else http_send_status(e, 403, "Forbidden");
    }

    return;
}