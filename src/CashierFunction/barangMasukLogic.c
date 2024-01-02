#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "../sqliteFunction.h"
#include "../../vendor/sandbird/sandbird.h"
#include "../../vendor/sqlite3/sqlite3.h"
#include "../utils/utils.h"

#include "cashierFunction.h"

int barangMasukLogic(sb_Event* e) {
    char tanggalBarangMasuk[32];
    char tempString[1024];
    char* errMsg;

    sqlite3* db;
    sqlite3_stmt* statement;
    SQLRow rowBack = {0};
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    switch(sb_convert_var_to_int(e->stream, "barangMasukArgs")) {
        case 1: {
            char is_exist;
            sb_get_header(e->stream, "tanggalBarangMasuk", tanggalBarangMasuk, 31);
            sqlite3_open("database/barangMasuk.db", &db);

            if (tanggalBarangMasuk[0]) {
                if (!is_valid_date(tanggalBarangMasuk)) {
                    sb_send_status(e->stream, 403, "Tanggal tidak valid!");
                    return SB_RES_OK;
                }
                
                sprintf(tempString, "SELECT name from pragma_table_info('barangMasuk_%s') where name = 'waktu'", tanggalBarangMasuk);
                sqlite3_exec(db, tempString, sqlTOF, &is_exist, NULL);
                if (is_exist) sprintf(tempString, "SELECT rowid,waktu,nama,jumlah,harga from barangMasuk_%s", tanggalBarangMasuk);
                else sprintf(tempString, "SELECT rowid,'-',nama,jumlah,harga from barangMasuk_%s", tanggalBarangMasuk);
            }
            else {
                sprintf(tempString, "SELECT name from pragma_table_info('barangMasuk_%d_%d_%d') where name = 'waktu'", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
                sqlite3_exec(db, tempString, sqlTOF, &is_exist, NULL);
                if (is_exist) sprintf(tempString, "SELECT rowid,waktu,nama,jumlah,harga from barangMasuk_%d_%d_%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
                else sprintf(tempString, "SELECT rowid,'-',nama,jumlah,harga from barangMasuk_%d_%d_%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
            }

            sqlite3_prepare_v2(db, tempString, -1, &statement, NULL);
            statement_get_row(statement, &rowBack, 0);
            sqlite3_finalize(statement);
            sqlite3_close(db);

            sb_write(e->stream, rowBack.rows, rowBack.totalChar);
            sb_writef(e->stream, "%d-%d-%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
            freeRowBack(&rowBack);

            // list barang masuk
            return SB_RES_OK;
        }
        case 2: {
            char namaBarang[255];
            sb_get_header(e->stream, "findBarang", namaBarang, 254);

            if (!namaBarang[0]) {
                sb_send_status(e->stream, 403, "Nama/Barcode Barang tidak boleh kosong!");
                return SB_RES_OK;
            }

            sqlite3_open("database/daftarBarang.db", &db);
            sqlite3_prepare_v2(db, "SELECT nama FROM daftarBarang WHERE nama LIKE ? OR barcode = ?", -1, &statement, NULL);
            
            sprintf(tempString, "%%%s%%", namaBarang);
            sqlite3_bind_text(statement, 1, tempString, -1, SQLITE_STATIC);
            sqlite3_bind_text(statement, 2, namaBarang, -1, SQLITE_STATIC);

            statement_get_row(statement, &rowBack, 0);
            sqlite3_finalize(statement);
            sqlite3_close(db);

            if (!rowBack.totalChar) {
                sb_send_status(e->stream, 403, "Nama/Barcode Barang Tidak ada di dalam database!");
                return SB_RES_OK;
            }

            sb_write(e->stream, rowBack.rows, rowBack.totalChar);
            freeRowBack(&rowBack);
            // check barang masuk
            return SB_RES_OK;
        }
        case 3: {
            char is_exist;
            char namaBarang[255];
            char jumlahBarang[32];
            char hargaBarang[32];

            sb_get_header(e->stream, "namaBarang", namaBarang, 254);
            sb_get_header(e->stream, "jumlahBarang", jumlahBarang, 31);
            sb_get_header(e->stream, "hargaBarang", hargaBarang, 31);

            if (!namaBarang[0]) {
                sb_send_status(e->stream, 403, "Nama/Barcode barang tidak boleh kosong!");
                return SB_RES_OK;
            } else if (!jumlahBarang[0]) {
                sb_send_status(e->stream, 403, "Jumlah Barang tidak boleh kosong!");
                return SB_RES_OK;
            } else if (!hargaBarang[0]) {
                sb_send_status(e->stream, 403, "Harga Barang tidak boleh kosong!");
                return SB_RES_OK;
            }

            sqlite3_open("database/barangMasuk.db", &db);

            sprintf(tempString, "CREATE TABLE IF NOT EXISTS barangMasuk_%d_%d_%d (nama TEXT, jumlah INT, harga INT, waktu TEXT DEFAULT (strftime('%%H:%%M:%%S', 'now', 'localtime')));", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
            if (sqlNormalExec(e, db, tempString) == SB_RES_OK) return SB_RES_OK;

            sprintf(tempString, "SELECT name from pragma_table_info('barangMasuk_%d_%d_%d') where name = 'waktu'", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
            sqlite3_exec(db, tempString, sqlTOF, &is_exist, NULL);
            if (!is_exist) {
                sprintf(tempString, "CREATE TABLE IF NOT EXISTS barangMasuk_%d_%d_%d_new (nama TEXT, jumlah INT, harga INT, waktu TEXT DEFAULT (strftime('%%H:%%M:%%S', 'now', 'localtime')));", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
                sqlite3_exec(db, tempString, 0, 0, NULL);
                sprintf(tempString, "INSERT INTO barangMasuk_%d_%d_%d_new (nama, jumlah, harga) SELECT nama, jumlah, harga FROM barangMasuk_%d_%d_%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
                sqlite3_exec(db, tempString, 0, 0, NULL);
                sprintf(tempString, "DROP TABLE barangMasuk_%d_%d_%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
                sqlite3_exec(db, tempString, 0, 0, NULL);
                sprintf(tempString, "ALTER TABLE barangMasuk_%d_%d_%d_new rename to barangMasuk_%d_%d_%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
                sqlite3_exec(db, tempString, 0, 0, NULL);
            }

            sprintf(tempString, "INSERT INTO barangMasuk_%d_%d_%d (nama, jumlah, harga) VALUES (?, ?, ?)", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
            sqlite3_prepare_v2(db, tempString, -1, &statement, NULL);

            sqlite3_bind_text(statement, 1, namaBarang, -1, SQLITE_STATIC);
            sqlite3_bind_int(statement, 2, atoi(jumlahBarang));
            sqlite3_bind_int(statement, 3, atoi(hargaBarang));

            sqlite3_step(statement);
            sqlite3_finalize(statement);
            sqlite3_close(db);

            sqlite3_open("database/daftarBarang.db", &db);

            sqlite3_prepare_v2(db, "UPDATE daftarBarang SET jumlah=jumlah + ? where nama = ?", -1, &statement, NULL);
            sqlite3_bind_int(statement, 1, atoi(jumlahBarang));
            sqlite3_bind_text(statement, 2, namaBarang, -1, SQLITE_STATIC);

            sqlite3_step(statement);
            sqlite3_finalize(statement);
            sqlite3_close(db);

            sb_send_status(e->stream, 200, "OK");
            // tambah barang masuk
            return SB_RES_OK;
        }
        case 4: {
            char namaBarang[255];
            char barangMasukID[11];

            sb_get_header(e->stream, "namaBarang", namaBarang, 254);
            sb_get_header(e->stream, "tanggalBarangMasuk", tanggalBarangMasuk, 11);
            sb_get_header(e->stream, "barangMasukID", barangMasukID, 11);

            if (!namaBarang[0]) {
                sb_send_status(e->stream, 403, "Nama/Barcode barang tidak boleh kosong!");
                return SB_RES_OK;
            }

            if (!barangMasukID[0]) {
                sb_send_status(e->stream, 403, "ID Barang Masuk tidak boleh kosong!");
                return SB_RES_OK;
            }

            sqlite3_open("database/barangMasuk.db", &db);

            if (tanggalBarangMasuk[0]) {
                if (!is_valid_date(tanggalBarangMasuk)) {
                    sb_send_status(e->stream, 403, "Tanggal tidak valid!");
                    return SB_RES_OK;
                }
                sprintf(tempString, "SELECT jumlah FROM barangMasuk_%s where rowid = ?", tanggalBarangMasuk, atoi(barangMasukID));
            }
            else sprintf(tempString, "SELECT jumlah FROM barangMasuk_%d_%d_%d WHERE rowid = ?", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, atoi(barangMasukID));

            sqlite3_prepare_v2(db, tempString, -1, &statement, NULL);
            statement_get_row(statement, &rowBack, 0);
            sqlite3_finalize(statement);

            if (tanggalBarangMasuk[0]) sprintf(tempString, "DELETE FROM barangMasuk_%s where rowid = ?", tanggalBarangMasuk);
            else sprintf(tempString, "DELETE FROM barangMasuk_%d_%d_%d where rowid = ?", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);

            sqlite3_prepare_v2(db, tempString, -1, &statement, NULL);
            sqlite3_bind_int(statement, 1, atoi(barangMasukID));
            sqlite3_step(statement);
            sqlite3_finalize(statement);
            sqlite3_close(db);

            sqlite3_open("database/daftarBarang.db", &db);
            sqlite3_prepare_v2(db, "UPDATE daftarBarang SET jumlah=jumlah - ? where nama = ?", -1, &statement, NULL);
            
            sqlite3_bind_int(statement, 1, atoi(rowBack.rows));
            sqlite3_bind_text(statement, 2, namaBarang, -1, SQLITE_STATIC);
            sqlite3_step(statement);
            sqlite3_finalize(statement);
            
            freeRowBack(&rowBack);
            sqlite3_close(db);

            sb_send_status(e->stream, 200, "OK");
            // hapus barang masuk
            return SB_RES_OK;
        }
        default: return SB_RES_OK;
    }

    sqlite3_close(db);
    freeRowBack(&rowBack);

    return SB_RES_OK;
}
