#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../funcVarPub.h"
#include "../../vendor/sandbird/sandbird.h"
#include "../sqliteFunction.h"
#include "../../vendor/sqlite3/sqlite3.h"
#include "../utils/utils.h"
#include "../telegramClient/telegramClient.h"

#include "cashierFunction.h"

int pengaturan(sb_Event* e) {
    char tempString[1024];
    sqlite3* db;
    switch(sb_convert_var_to_int(e->stream, "pengaturanArgs")) {
        case 1: {
            SQLRow row = {0};
            sqlite3_open("database/settings.db", &db);
            sqlite3_exec(db, "SELECT value FROM settings", RowBack, &row, NULL);
            sb_send_status(e->stream, 200, "OK");
            sb_write(e->stream, row.rows, row.totalChar);
            freeRowBack(&row);
            sqlite3_close(db);
            break;
            // check configurasi dan mengirimkan ke Client
        }
        case 2: {
            sb_Body bodyClient = {0};
            sqlite3_open("database/settings.db", &db);
            sb_get_body(e->stream, &bodyClient);
            char** valueSplit = strsplit(bodyClient.data, "\n", 0);
            free(bodyClient.data);
            
            if (!strlen(valueSplit[0])) {
                sb_send_status(e->stream, 403, "Memakai Telegram Bot tidak boleh kosong!");
                return SB_RES_OK;
            }
            if (!valueSplit[1][0] && valueSplit[0][1] == '1') {
                sb_send_status(e->stream, 403, "Telegram Token ID tidak boleh kosong!");
                return SB_RES_OK;
            }
            if (!valueSplit[2][0] && valueSplit[0][1] == '1') {
                sb_send_status(e->stream, 403, "Telegram User ID tidak boleh kosong!");
                return SB_RES_OK;
            }

            if (teleBot.usingTelegramBot) {
                free(teleBot.tokenBot);
                free(teleBot.userID);
            }

            sprintf(tempString, "UPDATE settings SET value = '%s' WHERE name = 'usingTelegram'", valueSplit[0]);
            sqlite3_exec(db, tempString, 0, 0, NULL);
            
            if (valueSplit[0][0] == '1') {
                teleBot.usingTelegramBot = 1;
                asprintf(&teleBot.tokenBot, "%s", valueSplit[1]);
                teleBot.userID = strsplit(valueSplit[2], ",", &teleBot.userIDsize);
                sprintf(tempString, "UPDATE settings SET value = '%s' WHERE name = 'telegramTokenID'", valueSplit[1]);
                sqlite3_exec(db, tempString, 0, 0, NULL);
                sprintf(tempString, "UPDATE settings SET value = '%s' WHERE name = 'telegramUserID'", valueSplit[2]);
                sqlite3_exec(db, tempString, 0, 0, NULL);
            } else teleBot.usingTelegramBot = 0;
            
            free(valueSplit);
            sqlite3_close(db);
            sb_send_status(e->stream, 200, "OK");
            break;
        }
        case 3: {
            int sendRet = sendMessage("Hello World from AplikasiKasir");
            if (!sendRet) sb_send_status(e->stream, 403, "Tidak bisa terkoneksi ke telegram, apakah internet anda menyala?");
            else if (sendRet == -1) sb_send_status(e->stream, 403, "Token Bot dan User ID yang kamu masukan itu salah! mohon masukan Token Bot dan User ID yang benar");
            else sb_send_status(e->stream, 200, "OK");
            // telegram testing
            break;
        }
    }
    return SB_RES_OK;
}