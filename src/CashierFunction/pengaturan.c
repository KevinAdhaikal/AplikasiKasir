#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>

#include "../alarmFunction/alarmFunction.h"
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
        }
        case 2: {
            sb_Body bodyClient = {0};
            sqlite3_open("database/settings.db", &db);
            sb_get_body(e->stream, &bodyClient);
            size_t valueLen;
            char** valueSplit = strsplit(bodyClient.data, "\n", &valueLen);
            free(bodyClient.data);

            // melakukan pengecekan data dari client
            if (valueLen < 9) {
                free(valueSplit);
                sb_send_status(e->stream, 403, "Invalid Value");
                sqlite3_close(db);
                return SB_RES_OK;
            }

            if (atoi(valueSplit[7]) > 65535) {
                free(valueSplit);
                sb_send_status(e->stream, 403, "Tidak bisa lebih dari 65535");
                sqlite3_close(db);
                return SB_RES_OK;
            }

            for (int a = 0; a < strlen(valueSplit[7]); a++) {
                if (!isdigit(valueSplit[7][a])) {
                    sb_send_status(e->stream, 403, "Value tersebut tidak berbentuk nomor!");
                    return SB_RES_OK;
                }
            }

            if (!isdigit(valueSplit[0][0]) || !isdigit(valueSplit[3][0]) || !isdigit(valueSplit[4][0]) || !isdigit(valueSplit[5][0]) || !isdigit(valueSplit[6][0]) || !isdigit(valueSplit[8][0])) {
                sb_send_status(e->stream, 403, "Value tersebut tidak berbentuk nomor");
                return SB_RES_OK;
            }

            if (valueSplit[8][0] == '1' && !is_valid_time_format(valueSplit[9])) {
                sb_send_status(e->stream, 403, "Value jam tersebut tidak valid!");
                return SB_RES_OK;
            }

            if (teleBot.usingTelegramBot) {
                free(teleBot.tokenBot);
                free(teleBot.userID);
            }

            // setting telegram
            sprintf(tempString, "UPDATE settings SET value = '%c' WHERE name = 'usingTelegram'", valueSplit[0][0]);
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

            // setting block barang kosong
            sprintf(tempString, "UPDATE settings SET value = '%c' WHERE name = 'blockBarangKosong'", valueSplit[3][0]);
            sqlite3_exec(db, tempString, 0, 0, NULL);

            // setting notify barang kosong
            sprintf(tempString, "UPDATE settings SET value = '%c' WHERE name = 'notifyBarangKosongTGram'", valueSplit[4][0]);
            sqlite3_exec(db, tempString, 0, 0, NULL);
            teleBot.notifyBarangKosongTGram = atoi(&valueSplit[4][0]);

            // setting notify kasir
            sprintf(tempString, "UPDATE settings SET value = '%c' WHERE name = 'notifyKasirTGram'", valueSplit[5][0]);
            sqlite3_exec(db, tempString, 0, 0, NULL);
            teleBot.notifyKasirTGram = atoi(&valueSplit[5][0]);

            // setting jika notify dibawah stock barang
            sprintf(tempString, "UPDATE settings SET value = '%c' WHERE name = 'isNotifyDibawahStockBarangTGram'", valueSplit[6][0]);
            sqlite3_exec(db, tempString, 0, 0, NULL);
            teleBot.isNotifyBarangDibawahJumlah = atoi(&valueSplit[6][0]); 

            // setting jumlah notify dibawah stock barang
            sprintf(tempString, "UPDATE settings SET value = %d WHERE name = 'jumlahNotifyDibawahStockBarangTGram'", atoi(valueSplit[7]));
            sqlite3_exec(db, tempString, 0, 0, NULL);

            // setting jika memakai notify alarm pembukuan
            sprintf(tempString, "UPDATE settings SET value = '%c' WHERE name = 'isNotifyAlarmPembukuan'", valueSplit[8][0]);
            sqlite3_exec(db, tempString, 0, 0, NULL);
            teleBot.isNotifyAlarmPembukuan = atoi(&valueSplit[8][0]);

            // setting waktu notify alarm pembukuan
            sprintf(tempString, "UPDATE settings SET value = '%s' WHERE name = 'waktuNotifyAlarmPembukuan'", valueSplit[9]);
            sqlite3_exec(db, tempString, 0, 0, NULL);

            sprintf(tempString, "UPDATE settings SET value = '%c' WHERE name = 'isAutoRefreshBarangTotalTerjual'", valueSplit[10][0]);
            sqlite3_exec(db, tempString, 0, 0, NULL);

            sprintf(tempString, "UPDATE settings SET value = '%c' WHERE name = 'AutoSelectFilterDate'", valueSplit[11][0]);
            sqlite3_exec(db, tempString, 0, 0, NULL);

            if (teleBot.isNotifyAlarmPembukuan) {
                int to_second = convertToSeconds(valueSplit[9]);
                
                if (teleThreadInfo.isAlarmPembukuanRun) {
                    #ifdef _WIN32
                    TerminateThread(tele_thread[0], 0);
                    #else
                    pthread_cancel(tele_thread[0]);
                    #endif
                }
                #ifdef _WIN32
                tele_thread[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pembukuanAlarm, &to_second, 0, NULL);
                #else
                pthread_create(&tele_thread[0], NULL, pembukuanAlarm, &to_second);
                #endif
            } else {
                if (teleThreadInfo.isAlarmPembukuanRun) {
                    #ifdef _WIN32
                    TerminateThread(tele_thread, 0);
                    #else
                    pthread_cancel(tele_thread[0]);
                    #endif
                }
            }

            for (int a = 0; a < 5; a++) {
                if (valueSplit[7][a]) {
                    if (!isdigit(valueSplit[7][a])) goto JUMP;
                } else break;
            }
            
            teleBot.targetNotifyBarangDibawahJumlah = atoi(valueSplit[7]); 

            JUMP:
            free(valueSplit);
            sqlite3_close(db);
            sb_send_status(e->stream, 200, "OK");
            
            break;
            // simpan konfigurasi
        }
        case 3: {
            char token[256];
            char userid[512];
            sb_get_header(e->stream, "teleToken", token, 255);
            sb_get_header(e->stream, "teleID", userid, 511);

            int sendRet = sendMessage("Hello World from AplikasiKasir", token, userid);
            if (!sendRet) sb_send_status(e->stream, 403, "Tidak bisa terkoneksi ke telegram, apakah internet anda menyala?");
            else if (sendRet == -1) sb_send_status(e->stream, 403, "Token Bot dan User ID yang kamu masukan itu salah! mohon masukan Token Bot dan User ID yang benar");
            else sb_send_status(e->stream, 200, "OK");
            
            break;
            // telegram testing
        }
        case 4: {
            SQLRow row = {0};
            sqlite3_open("database/settings.db", &db);
            sqlite3_exec(db, "SELECT value FROM settings where name IN ('isAutoRefreshBarangTotalTerjual', 'AutoSelectFilterDate')", RowBack, &row, NULL);
            sb_write(e->stream, row.rows, row.totalChar);
            freeRowBack(&row);
            sqlite3_close(db);
            // Mendapatkan konfigurasi (Dashboard)
            break;
        }
    }
    return SB_RES_OK;
}