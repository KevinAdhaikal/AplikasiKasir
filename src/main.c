#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#include "utils/utils.h"
#include "../vendor/httplib/httplibrary.h"
#include "methodFunction/methodFunction.h"
#include "../vendor/sqlite3/sqlite3.h"
#include "../vendor/str/str.h"
#include "teleFunction/teleFunction.h"

#include "sqliteFunction.h"
#include "funcVarPub.h"

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

void event_handler(http_event* e) {
    printf("[%s] %s\n", e->headers.method, e->headers.path);
    if (includeStr(e->headers.path, "./")) return;
    
    if (!strcmp(e->headers.method, "POST")) POSTFunction(e);
    else if (!strcmp(e->headers.method, "GET")) GETFunction(e);
    return;
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

    // HTTP Server
    http_start(http_init(80), event_handler);
    return 0;
}
