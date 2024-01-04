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
    char username[20], password[20];

    sb_get_cookie(e->stream, "username", username, 19);
    sb_get_cookie(e->stream, "password", password, 19);

    if (isStr(username, "admin", 1) && isStr(password, "admin", 1)) {
        switch(sb_convert_var_to_int(e->stream, "api_args")) {
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
            default: {
                sb_send_status(e->stream, 404, "Not Found");
                break;
            }
        }
    }

    else sb_send_status(e->stream, 403, "Forbidden");

    return SB_RES_OK;
}