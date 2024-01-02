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
    sqlite3_stmt* statement;
    SQLRow allrows = {0};

    sb_get_header(e->stream, "inputBarang", inputBarang, 254);

    if (!inputBarang[0]) {
        sb_send_status(e->stream, 403, "Input barang tidak bisa kosong! Mohon input barang yang benar");
        return SB_RES_OK;
    }
    
    sqlite3_open("database/daftarBarang.db", &db);

    sqlite3_prepare_v2(db, "SELECT nama,jumlah,modal,jual FROM daftarBarang where nama LIKE ? OR barcode = ?", -1, &statement, NULL);
    sprintf(strTemp, "%%%s%%", inputBarang);
    sqlite3_bind_text(statement, 1, strTemp, -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 2, inputBarang, -1, SQLITE_STATIC);
    statement_get_row(statement, &allrows, 0);
    sqlite3_finalize(statement);
    sqlite3_close(db);

    if (!allrows.totalChar) sb_send_status(e->stream, 403, "Barang tersebut tidak tersedia di dalam database!");
    else sb_write(e->stream, allrows.rows, allrows.totalChar);
    
    freeRowBack(&allrows);
    return SB_RES_OK;
}

int cashierSettings(sb_Event* e) {
    sqlite3_stmt* statement;
    sqlite3* db;
    SQLRow row = {0};
    sqlite3_open("database/settings.db", &db);
    sqlite3_prepare_v2(db, "SELECT value FROM settings where name IN ('blockBarangKosong', 'notifyKasirTGram')", -1, &statement, NULL);
    statement_get_row(statement, &row, 0);
    sqlite3_finalize(statement);
    sqlite3_close(db);

    sb_write(e->stream, row.rows, row.totalChar);
    freeRowBack(&row);
    return SB_RES_OK;
}

int cashierStockChecker(sb_Event* e) {
    sqlite3* db;
    sqlite3_stmt* statement;
    SQLRow row = {0};

    char inputBarang[255];
    sb_get_header(e->stream, "inputBarang", inputBarang, 254);
    sqlite3_open("database/daftarBarang.db", &db);

    sqlite3_prepare_v2(db, "SELECT jumlah FROM daftarBarang where nama = ?", -1, &statement, NULL);
    sqlite3_bind_text(statement, 1, inputBarang, -1, SQLITE_STATIC);
    statement_get_row(statement, &row, 0);
    sqlite3_finalize(statement);
    sqlite3_close(db);

    sb_write(e->stream, row.rows, row.totalChar);
    freeRowBack(&row);
    return SB_RES_OK;
}