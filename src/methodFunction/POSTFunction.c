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
    sqlite3_stmt* statement;
    sqlite3* db;

    char username[65], password[65];

    sb_get_cookie(e->stream, "username", username, 64);
    sb_get_cookie(e->stream, "password", password, 64);

    sqlite3_open("database/user.db", &db);
    sqlite3_prepare_v2(db, "SELECT 1 FROM user_table WHERE username = ? AND password = ?", -1, &statement, NULL);
    sqlite3_bind_text(statement, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 2, password, -1, SQLITE_STATIC);
    char res = sqlite3_step(statement) == SQLITE_ROW;
    
    sqlite3_finalize(statement);
    sqlite3_close(db);

    if (res) {
        switch(sb_convert_var_to_int(e->stream, "api_args")) {
            case 0: {
                // TEST ONLY
                char data[1024];
                sb_get_body_static(e->stream, data, 1024);
                printf("%s\n", data);
                break;
            }
            case 1: {
                sb_send_status(e->stream, 200, "OK");
                break;
            }
            case 2: {
                tambahBarang(e);
                break;
            }
            case 3: {
                daftarBarang(e);
                break;
            }
            case 4: {
                hapusDaftarBarang(e);
                break;
            }
            case 5: {
                infoBarang(e);
                break;
            }
            case 6: {
                editBarang(e);
                break;
            }
            case 7: {
                cashierFindBarang(e);
                break;
            }
            case 8: {
                cashierSettings(e);
                break;
            }
            case 9: {
                cashierStockChecker(e);
                break;
            }
            case 10: {
                pembukuan(e);
                break;
            }
            case 11: {
                dashboardLogic(e);
                break;
            }
            case 12: {
                barangMasukLogic(e);
                break;
            }
            case 13: {
                pengaturan(e);
                break;
            }
            case 14: {
                dbBackup(e);
                break;
            }
            case 15: {
                dbImport(e);
                break;
            }
            case 16: {
                userLogic(e);
                break;
            }
            case 17: {
                roleLogic(e);
                break;
            }
            default: {
                sb_send_status(e->stream, 404, "Not Found");
                break;
            }
        }
    }
    else sb_send_status(e->stream, 403, "Forbidden");

    return SB_RES_OK;
}