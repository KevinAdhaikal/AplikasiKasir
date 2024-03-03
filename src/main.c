#define _GNU_SOURCE // Disable implicit declaration warning

#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>

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

void prepare_db_func(sqlite3* db, sqlite3_stmt* statement, char* key, char* text) {
    char tempString[2048];
    SQLRow rows = {0};

    sqlite3_prepare_v2(db, "SELECT name FROM sqlite_schema", -1, &statement, NULL);
    statement_get_row(statement, &rows, 1);
    sqlite3_finalize(statement);
    if  (!rows.totalChar) return;

    size_t split_total;
    char** string_split = strsplit(rows.rows, "\x01", &split_total);
    freeRowBack(&rows);
    char is_exist = 0;

    for (int a = 0; a < split_total; a++) {
        sprintf(tempString, "SELECT name from pragma_table_info('%s') where name = 'waktu'", string_split[a]);
        sqlite3_exec(db, tempString, sqlTOF, &is_exist, NULL);

        sprintf(tempString, "PRAGMA foreign_keys=off;"
                            "BEGIN TRANSACTION;"
                            "CREATE TABLE %s_temp AS SELECT * FROM %s;"
                            "DROP TABLE %s;"
                            "CREATE TABLE IF NOT EXISTS %s (%s);"
                            "INSERT INTO %s SELECT %s FROM %s_temp;"
                            "DROP TABLE %s_temp;"
                            "COMMIT;", string_split[a], string_split[a], string_split[a], string_split[a], key, string_split[a], is_exist ? "*" : text, string_split[a], string_split[a]);

        sqlite3_exec(db, tempString, 0, NULL, NULL);
    }

    free(string_split);
}

void prepare_db(char check_db_ver) {
    DIR* dir = opendir("database");
    if (!dir) system("mkdir database");
    else closedir(dir);
    sqlite3* db;
    sqlite3_stmt* statement;

    int db_ver = 0;
    FILE* fp;

    if (check_db_ver) {
        fp = fopen("database/db_ver.bin", "rb");
        if (fp) {
            fread(&db_ver, 4, 1, fp);
            if (db_ver == 1) {
                fclose(fp);
                return;
            }
            fclose(fp);
        }
    }

    printf("[LOGS] Mempersiapkan Database...\n");

    // daftarBarang.db
    sqlite3_open("database/daftarBarang.db", &db);
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS daftarBarang (nama TEXT, jumlah BIGINT, modal BIGINT, jual BIGINT, barcode TEXT);", 0, NULL, NULL);

    sqlite3_prepare_v2(db, "PRAGMA table_info(daftarBarang);", -1, &statement, NULL);

    char is_int_type[] = {0, 0, 0};
    
    while(sqlite3_step(statement) == SQLITE_ROW) {
        const char *column_name = (const char *)sqlite3_column_text(statement, 1);
        const char *column_type = (const char *)sqlite3_column_text(statement, 2);

        if (!strcmp(column_name, "jumlah") && !strcmp(column_type, "INT")) is_int_type[0] = 1;
        else if (!strcmp(column_name, "modal") && !strcmp(column_type, "INT")) is_int_type[1] = 1;
        else if (!strcmp(column_name, "jual") && !strcmp(column_type, "INT")) is_int_type[2] = 1;
    }

    sqlite3_finalize(statement);

    if (is_int_type[0] && is_int_type[1] && is_int_type[2]) {
        sqlite3_exec(db, "PRAGMA foreign_keys=off;"
                         "BEGIN TRANSACTION;"
                         "CREATE TABLE daftarBarang_temp AS SELECT * FROM daftarBarang;"
                         "DROP TABLE daftarBarang;"
                         "CREATE TABLE IF NOT EXISTS daftarBarang (nama TEXT, jumlah BIGINT, modal BIGINT, jual BIGINT, barcode TEXT);"
                         "INSERT INTO daftarBarang SELECT * FROM daftarBarang_temp;"
                         "DROP TABLE daftarBarang_temp;"
                         "COMMIT;", 0, NULL, NULL);
    }

    sqlite3_close(db);

    // pembukuan.db
    sqlite3_open("database/pembukuan.db", &db);
    prepare_db_func(db, statement, "nama TEXT, jumlah BIGINT, modal BIGINT, jual BIGINT, waktu TEXT DEFAULT (strftime('%%H:%%M:%%S', 'now', 'localtime'))", "nama, jumlah, modal, jual, '-' AS waktu");
    sqlite3_close(db);
    
    // pengeluaran.db
    sqlite3_open("database/pengeluaran.db", &db);
    prepare_db_func(db, statement, "desc TEXT, uang BIGINT, waktu TEXT DEFAULT (strftime('%%H:%%M:%%S', 'now', 'localtime'))", "desc, uang, '-' AS waktu");
    sqlite3_close(db);

    // barangMasuk.db
    sqlite3_open("database/barangMasuk.db", &db);
    prepare_db_func(db, statement, "nama TEXT, jumlah BIGINT, harga BIGINT, waktu TEXT DEFAULT (strftime('%%H:%%M:%%S', 'now', 'localtime'))", "nama, jumlah, harga, '-' AS waktu");
    sqlite3_close(db);

    // user.db
    sqlite3_open("database/user.db", &db);
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS user_table (username TEXT, password TEXT, role_id INT, is_super BOOLEAN DEFAULT 0, dibuat_waktu TEXT DEFAULT (strftime('%d-%m-%Y %H:%M:%S', 'now', 'localtime')), dimodif_waktu TEXT DEFAULT (strftime('%d-%m-%Y %H:%M:%S', 'now', 'localtime')));"
                     "INSERT INTO user_table(username, password, role_id, is_super) VALUES ('admin', 'admin', 1, 1);", 0, NULL, NULL);
    sqlite3_close(db);

    // role.db
    sqlite3_open("database/role.db", &db);
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS role_table (name_role TEXT, role_perm TEXT, is_super BOOLEAN DEFAULT 0, dibuat_waktu TEXT DEFAULT (strftime('%d-%m-%Y %H:%M:%S', 'now', 'localtime')), dimodif_waktu TEXT DEFAULT (strftime('%d-%m-%Y %H:%M:%S', 'now', 'localtime')));"
                     "INSERT INTO role_table(name_role, role_perm, is_super) VALUES ('Administrator', '1', 1);", 0, NULL, NULL);
    sqlite3_close(db);

    // database kasir version
    fp = fopen("database/db_ver.bin", "wb");
    db_ver = 1;
    fwrite(&db_ver, 4, 1, fp);
    fclose(fp);

    printf("[LOGS] Database sudah Disiapkan!\n");
}

void loadSettings() {
    sqlite3 *db;
    sqlite3_stmt* statement;
    SQLRow row = {0};

    sqlite3_open("database/settings.db", &db);

    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS settings (name TEXT PRIMARY KEY, value TEXT)", 0, 0, NULL);
    sqlite3_prepare_v2(db, "PRAGMA table_info(settings)", -1, &statement, NULL);
    sqlite3_step(statement);
    if (sqlite3_column_int(statement, 5) == 0) {
        sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS new_settings (name TEXT PRIMARY KEY, value TEXT)", 0, 0, NULL);
        sqlite3_exec(db, "INSERT INTO new_settings (name, value) SELECT name, value FROM settings", 0, 0, NULL);
        sqlite3_exec(db, "DROP TABLE IF EXISTS settings", 0, 0, NULL);
        sqlite3_exec(db, "ALTER TABLE new_settings RENAME TO settings", 0, 0, NULL);
    }
    sqlite3_finalize(statement);

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
    "('AutoSelectFilterDate', 0),"
    "('perbedaanInfoHariIniDenganKemarin', 0)", 0, 0, NULL);

    sqlite3_prepare_v2(db, "SELECT * FROM settings", -1, &statement, NULL);
    statement_get_row(statement, &row, 1);
    sqlite3_finalize(statement);

    char** strSplit = strsplit(row.rows, "\x01", 0);
    if (strSplit[0][14] == '1') {
        teleBot.usingTelegramBot = 1;
        char** valueSplit = strsplit(strSplit[1], "\x02", 0);
        asprintf(&teleBot.tokenBot, "%s", valueSplit[1]);
        free(valueSplit);
        valueSplit = strsplit(strSplit[2], "\x02", 0);
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
        sqlite3* db;
        sqlite3_stmt* statement;

        char username[65];
        char password[65];
        sb_get_var(e->stream, "username", username, 65);
        sb_get_var(e->stream, "password", password, 65);
        
        sqlite3_open("database/user.db", &db);
        sqlite3_prepare_v2(db, "SELECT 1 FROM user_table WHERE username = ? AND password = ?", -1, &statement, NULL);
        sqlite3_bind_text(statement, 1, username, -1, SQLITE_STATIC);
        sqlite3_bind_text(statement, 2, password, -1, SQLITE_STATIC);

        char res = sqlite3_step(statement) == SQLITE_ROW;
        sqlite3_finalize(statement);
        sqlite3_close(db);

        if (res) {
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
    memset(&teleBot, 0, sizeof(telegramBot));
    
    prepare_db(1);
    loadSettings();

    sb_Server *srv[2];
    sb_Options opt[2];

    // HTTP Server
    memset(&opt[0], 0, sizeof(sb_Options));
    opt[0].host = "0.0.0.0";
    opt[0].port = "80"; // anda boleh ganti port selain 8081, karena port 8081 adalah port Telegram API
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
