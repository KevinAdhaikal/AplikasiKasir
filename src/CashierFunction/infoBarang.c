#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

#include "../../vendor/sqlite3/sqlite3.h"
#include "../../vendor/sandbird/sandbird.h"
#include "../sqliteFunction.h"
#include "cashierFunction.h"

int infoBarang(sb_Event* e) {
    char barangID[32];
    sqlite3* db;
    sqlite3_stmt* statement;
    SQLRow allRow = {0};

    sb_get_header(e->stream, "barangID", barangID, 31);

    if (!barangID[0]) {
        sb_send_status(e->stream, 403, "Mohon input ID Barang yang benar!");
        return SB_RES_OK;
    }

    sqlite3_open("database/daftarBarang.db", &db);
    sqlite3_prepare_v2(db, "SELECT rowid,* from daftarBarang where rowid = ?", -1, &statement, NULL);
    sqlite3_bind_int64(statement, 1, atoll(barangID));
    statement_get_row(statement, &allRow, 0);
    sqlite3_finalize(statement);
    sqlite3_close(db);

    if (!allRow.rows) {
        freeRowBack(&allRow);
        sb_send_status(e->stream, 403, "Barang tersebut tidak tersedia di Daftar Barang, mohon input ID Barang yang benar!");
        return SB_RES_OK;
    }

    sb_write(e->stream, allRow.rows, allRow.totalChar);
    freeRowBack(&allRow);
    sqlite3_close(db);

    return SB_RES_OK;
}
