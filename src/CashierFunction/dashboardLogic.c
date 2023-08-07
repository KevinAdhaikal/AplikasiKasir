#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../../vendor/sqlite3/sqlite3.h"
#include "../../vendor/sandbird/sandbird.h"
#include "../sqliteFunction.h"
#include "../utils/utils.h"

int dashboardLogic(sb_Event* e) {
    char tempString[512];
    char* errMsg;
    sqlite3* db;
    SQLRow rowBack = {0};
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    switch(sb_convert_var_to_int(e->stream, "dashboardType")) {
        case 1: {
            sqlite3_open("database/pembukuan.db", &db);
            sprintf(tempString, "SELECT SUM(jumlah) from barangTerjual_%d_%d_%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
            // total barang terjual hari ini
            break;
        }
        case 2: {
            sqlite3_open("database/pembukuan.db", &db);
            sprintf(tempString, "SELECT SUM(jual) from barangTerjual_%d_%d_%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
            // total harga jual
            break;
        }
        case 3: {
            sqlite3_open("database/pengeluaran.db", &db);
            sprintf(tempString, "SELECT SUM(uang) from pengeluaran_%d_%d_%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
            // total pengeluaran
            break;
        }
        case 4: {
            sqlite3_open("database/pembukuan.db", &db);
            sprintf(tempString, "SELECT SUM(jual) - SUM(modal) from barangTerjual_%d_%d_%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
            // total keuntungan
            break;
        }
        case 5: {
            sqlite3_open("database/daftarBarang.db", &db);
            strcpy(tempString, "SELECT nama FROM daftarBarang WHERE jumlah < 1");
            // mencari barang kosong 
            break;
        }
        default: {
            return SB_RES_OK;
            break;
        }
    }

    if (sqlite3_exec(db, tempString, RowBack, &rowBack, &errMsg) != SQLITE_OK) {
        if (isStr(errMsg, "no such table", 0)) sb_send_status(e->stream, 200, "OK");
        else {
            sb_send_status(e->stream, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
            printf("[ERROR] Something wrong in SQLite at dashboardLogic.c: %s\n", errMsg);
        }
        sqlite3_free(errMsg);
        freeRowBack(&rowBack);
        sqlite3_close(db);
        return SB_RES_OK;
    }

    sb_send_status(e->stream, 200, "OK");
    sb_write(e->stream, rowBack.rows, rowBack.totalChar);
    freeRowBack(&rowBack);
    sqlite3_close(db);

    return SB_RES_OK;
}