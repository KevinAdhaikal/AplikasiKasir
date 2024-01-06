#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#else
#include <pthread.h>
#endif

#include "sqliteFunction.h"
#include "funcVarPub.h"
#include "utils/utils.h"
#include "methodFunction/methodFunction.h"
#include "teleFunction/teleFunction.h"
#include "alarmFunction/alarmFunction.h"

#include "../vendor/sandbird/sandbird.h"
#include "../vendor/sqlite3/sqlite3.h"
#include "../vendor/str/str.h"

#ifdef _WIN32
HANDLE serverThread;
#else
pthread_t serverThread;
#endif

void* runServer(void* ptr) {
    for (;;) {
        sb_poll_server((sb_Server*)ptr, 1000);
    }
    sb_close_server((sb_Server*)ptr);
}

void loadSettings() {
    sqlite3 *db;
    SQLRow row = {0};
    sqlite3_open("database/settings.db", &db);

    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS settings (name TEXT, value TEXT)", 0, 0, NULL);
    sqlite3_exec(db, "PRAGMA table_info(settings);", RowBack, &row, NULL);

    if (row.rows[19] == '0') {
        sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS new_settings (name TEXT PRIMARY KEY, value TEXT)", 0, 0, NULL);
        sqlite3_exec(db, "INSERT INTO new_settings (name, value) SELECT name, value FROM settings", 0, 0, NULL);
        sqlite3_exec(db, "DROP TABLE IF EXISTS settings", 0, 0, NULL);
        sqlite3_exec(db, "ALTER TABLE new_settings RENAME TO settings", 0, 0, NULL);
    }

    freeRowBack(&row);

    sqlite3_exec(db, "INSERT OR IGNORE INTO settings (name, value) VALUES ('usingTelegram', 0),"
    "('telegramTokenID', ''),"
    "('telegramUserID', ''),"
    "('blockBarangKosong', 1),"
    "('notifyBarangKosongTGram', 0),"
    "('notifyKasirTGram', 1),"
    "('isNotifyDibawahStockBarangTGram', 0),"
    "('jumlahNotifyDibawahStockBarangTGram', 0),"
    "('isNotifyAlarmPembukuan', 0),"
    "('waktuNotifyAlarmPembukuan', 0),"
    "('isAutoRefreshBarangTotalTerjual', 0),"
    "('AutoSelectFilterDate', 0)", 0, 0, NULL);

    sqlite3_exec(db, "SELECT * from settings", RowBack, &row, NULL);
    char** strSplit = strsplit(row.rows, "\n", 0);

    if (isStr(strSplit[0], "usingTelegram|1", 1)) {
        teleBot.usingTelegramBot = 1;
        char** valueSplit = strsplit(strSplit[1], "|", 0);
        asprintf(&teleBot.tokenBot, "%s", valueSplit[1]);
        free(valueSplit);
        valueSplit = strsplit(strSplit[2], "|", 0);
        teleBot.userID = strsplit(valueSplit[1], ",", &teleBot.userIDsize);
        free(valueSplit);
    }
    else teleBot.usingTelegramBot = 0;
    
    teleBot.notifyBarangKosongTGram = strSplit[4][24] - 48;
    teleBot.notifyKasirTGram = strSplit[5][17] - 48;
    teleBot.isNotifyBarangDibawahJumlah = strSplit[6][32] - 48;
    teleBot.targetNotifyBarangDibawahJumlah = atoi(strSplit[7] + 36);
    teleBot.isNotifyAlarmPembukuan = atoi(strSplit[8] + 23);

    if (teleBot.isNotifyAlarmPembukuan) {
        int to_second = convertToSeconds(strSplit[9] + 26);
        #ifdef _WIN32
        tele_thread[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pembukuanAlarm, &to_second, 0, NULL);
        #else
        pthread_create(&tele_thread[0], NULL, pembukuanAlarm, &to_second);
        #endif
    }

    free(strSplit);
    freeRowBack(&row);
    sqlite3_close(db);

    printf("[LOGS] Configuration has been loaded!\n");
    return;
}

int thread_handler(sb_Event* e) {
    if (e->type == SB_EV_REQUEST && isStr(e->method, "POST", 1)) {
        char username[255];
        char password[255];
        sb_get_var(e->stream, "username", username, 254);
        sb_get_var(e->stream, "password", password, 254);
        if ((isStr(username, "admin", 1) && isStr(password, "admin", 1))) {
            if (!teleBot.usingTelegramBot) {
                sb_send_status(e->stream, 403, "Anda belum menyalakan Telegram di Setting! Mohon nyalakan Telegram di Setting");
                sb_send_header(e->stream, "Access-Control-Allow-Origin", "*");
                return SB_RES_OK;
            }
            switch(sb_convert_var_to_int(e->stream, "teleArgs")) {
                case 1: return teleTotalPembukuan(e);
                case 2: return teleKasir(e);
                default: return SB_RES_OK;
            }
        }
    }
    return SB_RES_OK;
}

int event_handler(sb_Event *e) {
    if (e->type == SB_EV_REQUEST) {
        if (includeStr(e->path, "../", strlen(e->path))) return SB_RES_CLOSE;
        if (!strcmp(e->method, "POST")) return POSTFunction(e);
        else if (!strcmp(e->method, "GET")) return GETFunction(e);
        else return SB_RES_CLOSE;
    }
    return SB_RES_OK;
}

int main() {
    DIR* dir = opendir("database");
    if (!dir) {
        printf("[LOGS] Database folder not found! creating...\n");
        system("mkdir database");
        printf("[LOGS] Database folder has been created!\n");
    } else closedir(dir);

    memset(&teleBot, 0, sizeof(telegramBot));
    loadSettings();

    sb_Server *srv[2];
    sb_Options opt[2];

    // HTTP Server
    memset(&opt[0], 0, sizeof(sb_Options));
    opt[0].host = "0.0.0.0";
    opt[0].port = "80";
    opt[0].handler = event_handler;

    srv[0] = sb_new_server(&opt[0]);

    // HTTP Server (Telegram)
    memset(&opt[1], 0, sizeof(sb_Options));
    opt[1].host = "0.0.0.0";
    opt[1].port = "8081";
    opt[1].handler = thread_handler;

    srv[1] = sb_new_server(&opt[1]);

    #ifdef _WIN32
    serverThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)runServer, srv[1], 0, NULL);
    #else
    pthread_create(&serverThread, NULL, runServer, srv[1]);
    #endif
    printf("[LOGS] AplikasiKasir run at port %s!\n", opt[0].port);
    runServer(srv[0]);
    return 0;
}
