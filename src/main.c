#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <pthread.h>

#include "utils/utils.h"
#include "../vendor/sandbird/sandbird.h"
#include "methodFunction/methodFunction.h"
#include "../vendor/sqlite3/sqlite3.h"
#include "../vendor/str/str.h"
#include "teleFunction/teleFunction.h"

#include "sqliteFunction.h"
#include "funcVarPub.h"

pthread_t serverThread;

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
    sqlite3_exec(db, "SELECT * from settings", RowBack, &row, NULL);

    if (!row.totalChar) {
        freeRowBack(&row);
        printf("[Config] Configuration not found, Creating...\n");
        sqlite3_exec(db, "INSERT INTO settings (name, value) VALUES ('usingTelegram', 0), ('telegramTokenID', ''), ('telegramUserID', '')", 0, 0, NULL);
        printf("[Config] Configuration has been created!\n");
        sqlite3_exec(db, "SELECT * from settings", RowBack, &row, NULL);
    }

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
    else if (isStr(strSplit[0], "usingTelegram|0", 1)) teleBot.usingTelegramBot = 0;

    free(strSplit);
    freeRowBack(&row);
    sqlite3_close(db);

    printf("[Config] Configuration has been loaded!\n");
    return;
}

int thread_handler(sb_Event* e) {
    if (e->type == SB_EV_REQUEST && isStr(e->method, "POST", 1)) {
        char username[254];
        char password[254];
        sb_get_var(e->stream, "username", username, 255);
        sb_get_var(e->stream, "password", password, 255);
        if (isStr(username, "admin", 1) && isStr(password, "admin", 1)) {
            switch(sb_convert_var_to_int(e->stream, "teleArgs")) {
                case 1: return teleTotalPembukuan(e);
                case 2: return teleKasir(e);
            }
        }
    }
    return SB_RES_OK;
}

int event_handler(sb_Event *e) {
    if (e->type == SB_EV_REQUEST) {
        printf("[%s] %s\n", e->method, e->path);
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
    opt[1].port = "8080";
    opt[1].handler = thread_handler;

    srv[1] = sb_new_server(&opt[1]);

    pthread_create(&serverThread, NULL, runServer, srv[1]);
    printf("[LOGS] AplikasiKasir run at port %s!\n", opt[0].port);
    runServer(srv[0]);
    return 0;
}
