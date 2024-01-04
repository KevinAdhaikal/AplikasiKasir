#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
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

int send_error(sb_Event* e, const char* text, char** valueSplit, sqlite3* db) {
    free(valueSplit);
    sb_send_status(e->stream, 403, text);
    sqlite3_close(db);
    return SB_RES_OK;
}

int pengaturan(sb_Event* e) {
    char tempString[1024];
    sqlite3* db;
    sqlite3_stmt* statement;

    switch(sb_convert_var_to_int(e->stream, "pengaturanArgs")) {
        case 1: {
            SQLRow row = {0};
            sqlite3_open("database/settings.db", &db);
            sqlite3_prepare_v2(db, "SELECT value FROM settings", -1, &statement, NULL);
            statement_get_row(statement, &row, 0);
            sqlite3_finalize(statement);
            sqlite3_close(db);
            sb_write(e->stream, row.rows, row.totalChar);
            freeRowBack(&row);
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
            if (valueLen < 9) return send_error(e, "Invalid Value", valueSplit, db);

            uint32_t str_len = strlen(valueSplit[7]);
            for (uint32_t a = 0; a < str_len; a++) {
                if (!isdigit(valueSplit[7][a])) return send_error(e, "Value tersebut tidak berbentuk nomor!", valueSplit, db);
            }
            if (atoi(valueSplit[7]) > 65535) return send_error(e, "Tidak bisa lebih dari 65535", valueSplit, db);

            if (!isdigit(valueSplit[0][0]) || !isdigit(valueSplit[3][0]) || !isdigit(valueSplit[4][0]) || !isdigit(valueSplit[5][0]) || !isdigit(valueSplit[6][0]) || !isdigit(valueSplit[8][0])) return send_error(e, "Value tersebut tidak berbentuk nomor!", valueSplit, db);
            if (valueSplit[8][0] == '1' && !is_valid_time_format(valueSplit[9])) return send_error(e, "Value jam tersebut tidak valid!", valueSplit, db);;

            if (teleBot.usingTelegramBot) {
                free(teleBot.tokenBot);
                free(teleBot.userID);
            }

            // setting telegram
            sqlite3_prepare_v2(db, "UPDATE settings SET value = ? WHERE name = ?", -1, &statement, NULL);
            sqlite3_bind_text(statement, 1, &valueSplit[0][0], -1, SQLITE_STATIC);
            sqlite3_bind_text(statement, 2, "usingTelegram", -1, SQLITE_STATIC);
            sqlite3_step(statement);
            sqlite3_reset(statement);
            sqlite3_clear_bindings(statement);

            if (valueSplit[0][0] == '1') {
                teleBot.usingTelegramBot = 1;
                asprintf(&teleBot.tokenBot, "%s", valueSplit[1]);
                teleBot.userID = strsplit(valueSplit[2], ",", &teleBot.userIDsize);

                sqlite3_bind_text(statement, 1, valueSplit[1], -1, SQLITE_STATIC);
                sqlite3_bind_text(statement, 2, "telegramTokenID", -1, SQLITE_STATIC);
                sqlite3_step(statement);
                sqlite3_reset(statement);
                sqlite3_clear_bindings(statement);
                
                sqlite3_bind_text(statement, 1, valueSplit[2], -1, SQLITE_STATIC);
                sqlite3_bind_text(statement, 2, "telegramUserID", -1, SQLITE_STATIC);
                sqlite3_step(statement);
                sqlite3_reset(statement);
                sqlite3_clear_bindings(statement);
            } else teleBot.usingTelegramBot = 0;

            // setting block barang kosong
            sqlite3_bind_text(statement, 1, &valueSplit[3][0], -1, SQLITE_STATIC);
            sqlite3_bind_text(statement, 2, "blockBarangKosong", -1, SQLITE_STATIC);
            sqlite3_step(statement);
            sqlite3_reset(statement);
            sqlite3_clear_bindings(statement);

            // setting notify barang kosong
            sqlite3_bind_text(statement, 1, &valueSplit[4][0], -1, SQLITE_STATIC);
            sqlite3_bind_text(statement, 2, "notifyBarangKosongTGram", -1, SQLITE_STATIC);
            sqlite3_step(statement);
            sqlite3_reset(statement);
            sqlite3_clear_bindings(statement);
            teleBot.notifyBarangKosongTGram = atoi(&valueSplit[4][0]);

            // setting notify kasir
            sqlite3_bind_text(statement, 1, &valueSplit[5][0], -1, SQLITE_STATIC);
            sqlite3_bind_text(statement, 2, "notifyKasirTGram", -1, SQLITE_STATIC);
            sqlite3_step(statement);
            sqlite3_reset(statement);
            sqlite3_clear_bindings(statement);
            teleBot.notifyKasirTGram = atoi(&valueSplit[5][0]);

            // setting jika notify dibawah stock barang
            sqlite3_bind_text(statement, 1, &valueSplit[6][0], -1, SQLITE_STATIC);
            sqlite3_bind_text(statement, 2, "isNotifyDibawahStockBarangTGram", -1, SQLITE_STATIC);
            sqlite3_step(statement);
            sqlite3_reset(statement);
            sqlite3_clear_bindings(statement);
            teleBot.isNotifyBarangDibawahJumlah = atoi(&valueSplit[6][0]); 

            // setting jumlah notify dibawah stock barang
            sqlite3_bind_int(statement, 1, atoi(valueSplit[7]));
            sqlite3_bind_text(statement, 2, "jumlahNotifyDibawahStockBarangTGram", -1, SQLITE_STATIC);
            sqlite3_step(statement);
            sqlite3_reset(statement);
            sqlite3_clear_bindings(statement);

            // setting jika memakai notify alarm pembukuan
            sqlite3_bind_text(statement, 1, &valueSplit[8][0], -1, SQLITE_STATIC);
            sqlite3_bind_text(statement, 2, "isNotifyAlarmPembukuan", -1, SQLITE_STATIC);
            sqlite3_step(statement);
            sqlite3_reset(statement);
            sqlite3_clear_bindings(statement);
            teleBot.isNotifyAlarmPembukuan = atoi(&valueSplit[8][0]);

            // setting waktu notify alarm pembukuan
            sqlite3_bind_text(statement, 1, valueSplit[9], -1, SQLITE_STATIC);
            sqlite3_bind_text(statement, 2, "waktuNotifyAlarmPembukuan", -1, SQLITE_STATIC);
            sqlite3_step(statement);
            sqlite3_reset(statement);
            sqlite3_clear_bindings(statement);

            // apakah menggunakan otomatis refresh barang total saat (saat anda memasuki dashboard, anda akan data barang total terjual)
            sqlite3_bind_text(statement, 1, &valueSplit[10][0], -1, SQLITE_STATIC);
            sqlite3_bind_text(statement, 2, "isAutoRefreshBarangTotalTerjual", -1, SQLITE_STATIC);
            sqlite3_step(statement);
            sqlite3_reset(statement);
            sqlite3_clear_bindings(statement);

            // apakah perlu otomatis select filter date (untuk di dashboard, bagian barang total terjual)
            sqlite3_bind_text(statement, 1, &valueSplit[11][0], -1, SQLITE_STATIC);
            sqlite3_bind_text(statement, 2, "AutoSelectFilterDate", -1, SQLITE_STATIC);
            sqlite3_step(statement);
            sqlite3_finalize(statement);

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

            sqlite3_prepare_v2(db, "SELECT value FROM settings where name IN ('isAutoRefreshBarangTotalTerjual', 'AutoSelectFilterDate')", -1, &statement, NULL);
            statement_get_row(statement, &row, 0);
            sqlite3_finalize(statement);
            sqlite3_close(db);

            sb_write(e->stream, row.rows, row.totalChar);
            freeRowBack(&row);
            // Mendapatkan konfigurasi (Dashboard)
            break;
        }
    }
    return SB_RES_OK;
}