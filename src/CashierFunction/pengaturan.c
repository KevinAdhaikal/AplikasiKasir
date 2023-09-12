#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../funcVarPub.h"
#include "../../vendor/httplib/httplibrary.h"
#include "../sqliteFunction.h"
#include "../../vendor/sqlite3/sqlite3.h"
#include "../utils/utils.h"
#include "../telegramClient/telegramClient.h"

#include "cashierFunction.h"

void pengaturan(http_event* e) {
    char tempString[1024];
    sqlite3* db;
    switch(http_get_query_to_int(e, "pengaturanArgs")) {
        case 1: {
            SQLRow row = {0};
            sqlite3_open("database/settings.db", &db);
            sqlite3_exec(db, "SELECT value FROM settings", RowBack, &row, NULL);
            http_write(e, row.rows, row.totalChar);
            freeRowBack(&row);
            sqlite3_close(db);
            break;
            // check configurasi dan mengirimkan ke Client
        }
        case 2: {
            if (e->headers.body_pos == -1) return;
            sqlite3_open("database/settings.db", &db);
            char** valueSplit = strsplit(e->headers.raw_header + e->headers.body_pos, "\n", 0);

            if (!strlen(valueSplit[0])) {
                http_send_status(e, 403, "Memakai Telegram Bot tidak boleh kosong!");
                return;
            }
            if (!valueSplit[1][0] && valueSplit[0][1] == '1') {
                http_send_status(e, 403, "Telegram Token ID tidak boleh kosong!");
                return;
            }
            if (!valueSplit[2][0] && valueSplit[0][1] == '1') {
                http_send_status(e, 403, "Telegram User ID tidak boleh kosong!");
                return;
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
            http_send_status(e, 200, "OK");
            break;
        }
        case 3: {
            int sendRet = sendMessage("Hello World from AplikasiKasir");
            if (!sendRet) http_send_status(e, 403, "Tidak bisa terkoneksi ke telegram, apakah internet anda menyala?");
            else if (sendRet == -1) http_send_status(e, 403, "Token Bot dan User ID yang kamu masukan itu salah! mohon masukan Token Bot dan User ID yang benar");
            else http_send_status(e, 200, "OK");
            // telegram testing
            break;
        }
    }
    return;
}