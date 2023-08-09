#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../vendor/sqlite3/sqlite3.h"
#include "../../vendor/sandbird/sandbird.h"
#include "../sqliteFunction.h"
#include "cashierFunction.h"
#include "../utils/utils.h"

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

    sprintf(strTemp, "SELECT nama,jumlah,modal,jual from daftarBarang where nama LIKE '%%%s%%' OR barcode = '%s'", inputBarang, inputBarang);

    if (sqlite3_exec(db, strTemp, RowBack, &allrows, &errMsg) != SQLITE_OK) {
        freeRowBack(&allrows);
        
        if (!isStr(errMsg, "no such table", 0)) {
            sb_send_status(e->stream, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
            printf("[ERROR] Something wrong in SQLite at casirLogic.c: %s\n", errMsg);
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