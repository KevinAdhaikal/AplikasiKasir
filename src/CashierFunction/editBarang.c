#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

#include "../../vendor/sqlite3/sqlite3.h"
#include "../../vendor/sandbird/sandbird.h"
#include "../sqliteFunction.h"
#include "../utils/utils.h"
#include "cashierFunction.h"

int editBarang(sb_Event* e) {
    char* errMsg;
    char strTemp[1024];
    sqlite3* db;
    sqlite3_stmt* statement;
    SQLRow checkRow = {0};
    char barangID[11], namaBarang[255], jumlahBarang[11], hargaModal[11], hargaJual[11], barcodeBarang[255];

    sb_get_header(e->stream, "barangID", barangID, 10);
    sb_get_header(e->stream, "namaBarang", namaBarang, 254);
    sb_get_header(e->stream, "jumlahBarang", jumlahBarang, 10);
    sb_get_header(e->stream, "hargaModal", hargaModal, 10);
    sb_get_header(e->stream, "hargaJual", hargaJual, 10);
    sb_get_header(e->stream, "barcodeBarang", barcodeBarang, 254);

    uint16_t str_len = strlen(barangID);

    for (uint16_t a = 0; a < str_len; a++) if (!isdigit(barangID[a])) {
        sb_send_status(e->stream, 403, "ID Barang tidak berbentuk nomor! Mohon input ID Barang yang benar");
        return SB_RES_OK;
    }

    str_len = strlen(jumlahBarang);

    for (uint16_t a = 0; a < str_len; a++) if (!isdigit(jumlahBarang[a])) {
        sb_send_status(e->stream, 403, "Jumlah Barang tidak berbentuk nomor! Mohon input Jumlah Barang yang benar");
        return SB_RES_OK;
    }

    str_len = strlen(hargaModal);

    for (uint16_t a = 0; a < str_len; a++) if (!isdigit(hargaModal[a])) {
        sb_send_status(e->stream, 403, "Harga Modal tidak berbentuk nomor! Mohon input Harga Barang yang benar");
        return SB_RES_OK;
    }
    
    str_len = strlen(hargaJual);

    for (uint16_t a = 0; a < str_len; a++) if (!isdigit(hargaJual[a])) {
        sb_send_status(e->stream, 403, "Harga Jual tidak berbentuk nomor! Mohon input Harga Barang yang benar");
        return SB_RES_OK;
    }

    sqlite3_open("database/daftarBarang.db", &db);

    sqlite3_prepare_v2(db, "SELECT nama FROM daftarBarang WHERE nama = ?", -1, &statement, NULL);
    sqlite3_bind_text(statement, 1, namaBarang, -1, SQLITE_STATIC);
    statement_get_row(statement, &checkRow, 1);
    sqlite3_finalize(statement);
    
    if (checkRow.rows && !isStr(checkRow.rows, namaBarang, 1)) {
        freeRowBack(&checkRow);
        sb_send_status(e->stream, 403, "Nama Barang tersebut sudah ada, mohon ganti Nama Barang!");
        sqlite3_close(db);
        return SB_RES_OK;
    }

    freeRowBack(&checkRow);

    if (barcodeBarang[0]) {
        sqlite3_prepare_v2(db, "SELECT barcode FROM daftarBarang WHERE barcode = ?", -1, &statement, NULL);
        sqlite3_bind_text(statement, 1, barcodeBarang, -1, SQLITE_STATIC);
        statement_get_row(statement, &checkRow, 1);
        sqlite3_finalize(statement);

        if (checkRow.rows && !isStr(checkRow.rows, barcodeBarang, 1)) {
            freeRowBack(&checkRow);
            sb_send_status(e->stream, 403, "Barcode Barang tersebut sudah ada, mohon ganti Nama Barang!");
            sqlite3_close(db);
            return SB_RES_OK;
        }

        freeRowBack(&checkRow);
    }

    sqlite3_prepare_v2(db, "UPDATE daftarBarang SET nama = ?, jumlah = ?, modal = ?, jual = ?, barcode = ? WHERE rowid = ?", -1, &statement, NULL);
    sqlite3_bind_text(statement, 1, namaBarang, -1, SQLITE_STATIC);
    sqlite3_bind_int(statement, 2, atoi(jumlahBarang));
    sqlite3_bind_int(statement, 3, atoi(hargaModal));
    sqlite3_bind_int(statement, 4, atoi(hargaJual));
    sqlite3_bind_text(statement, 5, barcodeBarang, -1, SQLITE_STATIC);
    sqlite3_bind_int(statement, 6, atoi(barangID));

    sqlite3_step(statement);
    sqlite3_finalize(statement);
    sqlite3_close(db);
    
    sb_send_status(e->stream, 200, "OK");
    
    return SB_RES_OK;
}
