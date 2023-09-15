#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../vendor/sqlite3/sqlite3.h"
#include "../../vendor/httplib/httplibrary.h"
#include "../sqliteFunction.h"
#include "cashierFunction.h"
#include "../utils/utils.h"

void cashierFindBarang(http_event* e) {
    char inputBarang[255], strTemp[1024];
    char* errMsg;
    sqlite3* db;
    SQLRow allrows = {0};

    http_get_header(e, "inputBarang", inputBarang, 254);

    if (!inputBarang[0]) {
        http_send_status(e, 403, "Input barang tidak bisa kosong! Mohon input barang yang benar");
        return;
    }
    
    sqlite3_open("database/daftarBarang.db", &db);

    sprintf(strTemp, "SELECT nama,jumlah,modal,jual from daftarBarang where nama LIKE '%%%s%%' OR barcode = '%s'", inputBarang, inputBarang);

    if (sqlite3_exec(db, strTemp, RowBack, &allrows, &errMsg) != SQLITE_OK) {
        freeRowBack(&allrows);
        
        if (!isStr(errMsg, "no such table", 0)) {
            http_send_status(e, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
            printf("[ERROR] Something wrong in SQLite at casirLogic.c: %s\n", errMsg);
        }
        else http_send_status(e, 403, "Barang tersebut tidak tersedia di dalam database!"); // tidak tau mengapa, code ini tidak berjalan, jadi saya tambahkan lagi di bawah
        
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return;
    }

    if (!isStr(errMsg, "no such table", 0)) {
        http_send_status(e, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
        printf("[ERROR] Something wrong in SQLite at casirLogic.c: %s\n", errMsg);
        sqlite3_close(db);
        return;
    }

    http_write(e, allrows.rows, allrows.totalChar);
    sqlite3_close(db);
    
    return;
}
