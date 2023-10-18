#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../vendor/sqlite3/sqlite3.h"
#include "../../vendor/sandbird/sandbird.h"
#include "../sqliteFunction.h"
#include "../utils/utils.h"
#include "../funcVarPub.h"

#include "cashierFunction.h"

int cashierFindBarang(sb_Event* e) {
    char inputBarang[255], strTemp[1024];
    char* errMsg;
    sqlite3* db;
    SQLRow allrows = {0};

    sb_get_header(e->stream, "inputBarang", inputBarang, 254);

    if (!inputBarang[0]) {
        sb_send_status(e->stream, 403, "Input barang tidak bisa kosong! Mohon input barang yang benar");
        return SB_RES_OK;
    }
    
    sqlite3_open("database/daftarBarang.db", &db);

    sprintf(strTemp, "SELECT nama,jumlah,modal,jual FROM daftarBarang where nama LIKE '%%%s%%' OR barcode = '%s'", inputBarang, inputBarang);

    if (sqlite3_exec(db, strTemp, RowBack, &allrows, &errMsg) != SQLITE_OK) {
        freeRowBack(&allrows);
        
        if (!isStr(errMsg, "no such table", 0)) {
            sb_send_status(e->stream, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
            printf("[ERROR] Something wrong in SQLite at cashierLogic.c: %s\n", errMsg);
        }
        else sb_send_status(e->stream, 403, "Barang tersebut tidak tersedia di dalam database!");
        
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return SB_RES_OK;
    }

    sb_send_status(e->stream, 200, "OK");
    sb_write(e->stream, allrows.rows, allrows.totalChar);
    sqlite3_close(db);
    
    return SB_RES_OK;
}

int cashierSettings(sb_Event* e) {
    sqlite3* db;
    SQLRow row = {0};
    sqlite3_open("database/settings.db", &db);
    sqlite3_exec(db, "SELECT value FROM settings where name IN ('blockBarangKosong', 'notifyKasirTGram')", RowBack, &row, NULL);
    sb_write(e->stream, row.rows, row.totalChar);
    sqlite3_close(db);
    freeRowBack(&row);
    return SB_RES_OK;
}

int cashierStockChecker(sb_Event* e) {
    sqlite3* db;
    SQLRow row = {0};
    char inputBarang[255], strTemp[1024], *errMsg;
    sb_get_header(e->stream, "inputBarang", inputBarang, 254);
    sqlite3_open("database/daftarBarang.db", &db);

    sprintf(strTemp, "SELECT jumlah FROM daftarBarang where nama = '%s'", inputBarang);
    if (sqlite3_exec(db, strTemp, RowBack, &row, &errMsg) != SQLITE_OK) {
        sb_send_status(e->stream, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
        printf("[ERROR] Something wrong in SQLite at cashierLogic.c: %s\n", errMsg);
        sqlite3_close(db);
        sqlite3_free(errMsg);
    }

    sb_write(e->stream, row.rows, row.totalChar);
    sqlite3_close(db);
    return SB_RES_OK;

}