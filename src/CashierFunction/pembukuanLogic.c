#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "../sqliteFunction.h"
#include "../../vendor/sqlite3/sqlite3.h"
#include "../../vendor/sandbird/sandbird.h"
#include "../utils/utils.h"
#include "../../vendor/cJSON/cJSON.h"
#include "../funcVarPub.h"
#include "../telegramClient/telegramClient.h"

#include "cashierFunction.h"

void resFree(sb_Event* ev, char* data) {
    sb_writef(ev->stream, "%s", data);
    cJSON_free(data);
}

int pembukuan(sb_Event* e) {
    char tempString[1024];
    char tanggalPembukuan[11];
    char* errMsg;
    sqlite3* db;
    sb_Body bodyData = {0};
    SQLRow row = {0};
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    switch(sb_convert_var_to_int(e->stream, "pembukuanArgs")) {
        case 1: {
            size_t newLineTotal;
            sb_get_body(e->stream, &bodyData);
            char** splitData = strsplit(bodyData.data, "\n", &newLineTotal);

            sqlite3_open("database/pembukuan.db", &db);
            sprintf(tempString, "CREATE TABLE IF NOT EXISTS barangTerjual_%d_%d_%d (nama TEXT, jumlah INT, modal INT, jual INT);", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
            if (!sqlNormalExec(e, db, tempString)) {
                free(splitData);
                return SB_RES_OK;
            }

            sqlite3_close(db);

            for (int a = 0; a < newLineTotal; a++) {
                sqlite3_open("database/pembukuan.db", &db);
                char** valueData = strsplit(splitData[a], "|", 0);
                sprintf(tempString, "INSERT INTO barangTerjual_%d_%d_%d (nama, jumlah, modal, jual) values ('%s', %d,  %d, %d);", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, valueData[0],  atoi(valueData[1]), atoi(valueData[2]), atoi(valueData[3]));
                if (!sqlNormalExec(e, db, tempString)) {
                    free(valueData);
                    free(splitData);
                    free(bodyData.data);
                    return SB_RES_OK;
                }

                sqlite3_close(db);
                sqlite3_open("database/daftarBarang.db", &db);

                sprintf(tempString, "UPDATE daftarBarang SET jumlah =jumlah - %d WHERE nama = '%s';", atoi(valueData[1]), valueData[0]);
                if (!sqlNormalExec(e, db, tempString)) {
                    free(valueData);
                    free(splitData);
                    free(bodyData.data);
                    return SB_RES_OK;
                }

                free(valueData);
                sqlite3_close(db);
            }

            free(splitData);
            free(bodyData.data);

            sb_send_status(e->stream, 200, "OK");
            sqlite3_close(db);
            return SB_RES_OK;

            // barang terjual masukan ke database
        }
        case 2: {
            int isTrue = 0;
            sb_get_header(e->stream, "tanggalPembukuan", tanggalPembukuan, 10);
            sqlite3_open("database/pembukuan.db", &db);

            if (!tanggalPembukuan[0]) {
                sprintf(tempString, "SELECT EXISTS (SELECT name FROM sqlite_master WHERE type='table' AND name='barangTerjual_%d_%d_%d');", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
                if (!sqlBackExec(e, db, tempString, sqlReturnInt, &isTrue)) return SB_RES_OK;

                if (!isTrue) {
                    sb_send_status(e->stream, 200, "OK");
                    sb_writef(e->stream, "%d-%d-%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
                } else {
                    sprintf(tempString, "SELECT rowid,* from barangTerjual_%d_%d_%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);

                    if (!sqlBackExec(e, db, tempString, RowBack, &row)) {
                        freeRowBack(&row);
                        return SB_RES_OK;
                    }

                    sb_send_status(e->stream, 200, "OK");
                    sb_write(e->stream, row.rows, row.totalChar);
                    sb_writef(e->stream, "%d-%d-%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
                    freeRowBack(&row);
                }
            } else {
                sprintf(tempString, "SELECT EXISTS (SELECT name FROM sqlite_master WHERE type='table' AND name='barangTerjual_%s');", tanggalPembukuan);
                if (!sqlBackExec(e, db, tempString, sqlReturnInt, &isTrue)) return SB_RES_OK;
                
                if (!isTrue) {
                    sb_send_status(e->stream, 200, "OK");
                    sb_writef(e->stream, "%d-%d-%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
                }
                else {
                    sprintf(tempString, "SELECT rowid,* from barangTerjual_%s", tanggalPembukuan);

                    if (!sqlBackExec(e, db, tempString, RowBack, &row)) {
                        freeRowBack(&row);
                        return SB_RES_OK;
                    }

                    sb_send_status(e->stream, 200, "OK");
                    sb_write(e->stream, row.rows, row.totalChar);
                    sb_writef(e->stream, "%d-%d-%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
                    freeRowBack(&row);
                }
            }
            sqlite3_close(db);
            return SB_RES_OK;
            
            // list barang terjual
        }
        case 3: {
            char idBarang[11];
            char tanggalPembukuan[11];
            char namaBarang[255];
            int totalJumlah = 0;

            sb_get_var(e->stream, "idBarang", idBarang, 10);
            sb_get_header(e->stream, "tanggalPembukuan", tanggalPembukuan, 10);
            sb_get_header(e->stream, "namaBarang", namaBarang, 254);

            if (!idBarang[0] && !namaBarang[0]) {
                sb_send_status(e->stream, 403, "ID dan Nama Barang tidak boleh kosong!");
                return SB_RES_OK;
            }

            if (tanggalPembukuan[0]) {
                if (namaBarang[0]) {
                    sprintf(tempString, "SELECT jumlah FROM barangTerjual_%s WHERE nama='%s'", tanggalPembukuan, namaBarang);
                    sqlite3_open("database/pembukuan.db", &db);
                    if (!sqlBackExec(e, db, tempString, RowBackPlus, &totalJumlah)) return SB_RES_OK;

                    sprintf(tempString, "DELETE FROM barangTerjual_%s WHERE nama='%s'", tanggalPembukuan, namaBarang);
                    if (!sqlNormalExec(e, db, tempString)) return SB_RES_OK;

                    sqlite3_close(db);
                    sqlite3_open("database/daftarBarang.db", &db);

                    sprintf(tempString, "UPDATE daftarBarang SET jumlah=jumlah + %d WHERE nama='%s'", totalJumlah, namaBarang);
                    if (!sqlNormalExec(e, db, tempString)) return SB_RES_OK;
                    sqlite3_close(db);
                }
                else {
                    sprintf(tempString, "SELECT * FROM barangTerjual_%s WHERE rowid=%s", tanggalPembukuan, idBarang);
                    sqlite3_open("database/pembukuan.db", &db);
                    if (!sqlBackExec(e, db, tempString, RowBack, &row)) {
                        freeRowBack(&row);
                        return SB_RES_OK;
                    }

                    sprintf(tempString, "DELETE FROM barangTerjual_%s WHERE rowid=%s", tanggalPembukuan, idBarang);

                    if (!sqlBackExec(e, db, tempString, RowBack, &row)) {
                        freeRowBack(&row);
                        return SB_RES_OK;
                    }

                    sqlite3_close(db);
                    sqlite3_open("database/daftarBarang.db", &db);

                    char** splitstring = strsplit(row.rows, "|", 0);
                    sprintf(tempString, "UPDATE daftarBarang SET jumlah=jumlah + %d WHERE nama='%s'", atoi(splitstring[1]), splitstring[0]);
                    freeRowBack(&row);
                    free(splitstring);

                    if (!sqlNormalExec(e, db, tempString)) return SB_RES_OK;

                    sqlite3_close(db);
                }
            } else {
                if (namaBarang[0]) {
                    sprintf(tempString, "SELECT jumlah FROM barangTerjual_%d_%d_%d WHERE nama='%s'", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, namaBarang);
                    sqlite3_open("database/pembukuan.db", &db);

                    if (!sqlBackExec(e, db, tempString, RowBackPlus, &totalJumlah)) return SB_RES_OK;

                    sprintf(tempString, "DELETE FROM barangTerjual_%d_%d_%d where nama=%s", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, namaBarang);
                    
                    if (!sqlNormalExec(e, db, tempString)) return SB_RES_OK;
                    sqlite3_close(db);

                    sqlite3_open("database/daftarBarang.db", &db);
                    sprintf(tempString, "UPDATE daftarBarang SET jumlah=jumlah + %d WHERE nama='%s'", totalJumlah, namaBarang);
                    if (!sqlNormalExec(e, db, tempString)) return SB_RES_OK;

                    sqlite3_close(db);
                }
                else {
                    sprintf(tempString, "SELECT * FROM barangTerjual_%d_%d_%d WHERE rowid=%s", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, idBarang);
                    sqlite3_open("database/pembukuan.db", &db);

                    if (!sqlBackExec(e, db, tempString, RowBack, &row)) {
                        freeRowBack(&row);
                        return SB_RES_OK;
                    }

                    sprintf(tempString, "DELETE FROM barangTerjual_%d_%d_%d where rowid=%s", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, idBarang);
                    if (!sqlNormalExec(e, db, tempString)) return SB_RES_OK;
                    sqlite3_close(db);

                    sqlite3_open("database/daftarBarang.db", &db);
                    char** splitstring = strsplit(row.rows, "|", 0);
                    sprintf(tempString, "UPDATE daftarBarang SET jumlah=jumlah + %d WHERE nama='%s'", atoi(splitstring[1]), splitstring[0]);
                    freeRowBack(&row);
                    free(splitstring);

                    if (!sqlNormalExec(e, db, tempString)) return SB_RES_OK;
                    sqlite3_close(db);
                }
            }
            sb_send_status(e->stream, 200, "OK");
            // hapus barang terjual
            return SB_RES_OK;
        }
        case 4: {
            char descPengeluaran[255];
            char uangPengeluaran[11];

            sb_get_header(e->stream, "descPengeluaran", descPengeluaran, 254);
            sb_get_header(e->stream, "uangPengeluaran", uangPengeluaran, 10);

            if (!descPengeluaran[0]) {
                sb_send_status(e->stream, 403, "Deskripsi Pengeluaran tidak boleh dikosongkan!");
                return SB_RES_OK;
            }
            if (!uangPengeluaran[0]) {
                sb_send_status(e->stream, 403, "Uang Pengeluaran tidak boleh dikosongkan!");
                return SB_RES_OK;
            }

            sqlite3_open("database/pengeluaran.db", &db);

            sprintf(tempString, "CREATE TABLE IF NOT EXISTS pengeluaran_%d_%d_%d (desc TEXT, uang INT);", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
            if (!sqlNormalExec(e, db, tempString)) return SB_RES_OK;
            sqlite3_close(db);

            sqlite3_open("database/pengeluaran.db", &db);

            sprintf(tempString, "INSERT INTO pengeluaran_%d_%d_%d (desc, uang) VALUES ('%s', %s);", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, descPengeluaran, uangPengeluaran);
            if (!sqlNormalExec(e, db, tempString)) return SB_RES_OK;
            sqlite3_close(db);
            sb_send_status(e->stream, 200, "OK");
            return SB_RES_OK;
            // tambah pengeluaran ke dalam database
        }
        case 5: {
            char tanggalPengeluaran[11];

            sqlite3_open("database/pengeluaran.db", &db);
            sb_get_header(e->stream, "tanggalPengeluaran", tanggalPengeluaran, 10);

            if (tanggalPengeluaran[0]) sprintf(tempString, "SELECT rowid,* FROM pengeluaran_%s", tanggalPengeluaran);
            else sprintf(tempString, "SELECT rowid,* FROM pengeluaran_%d_%d_%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);

            if (sqlite3_exec(db, tempString, RowBack, &row, &errMsg) != SQLITE_OK) {
                freeRowBack(&row);
                if (isStr(errMsg, "no such table", 0)) {
                    sb_send_status(e->stream, 200, "OK");
                    sb_writef(e->stream, "%d-%d-%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
                }
                sqlite3_free(errMsg);
                sqlite3_close(db);
                return SB_RES_OK;
            }

            sb_send_status(e->stream, 200, "OK");
            sb_write(e->stream, row.rows, row.totalChar);
            sb_writef(e->stream, "%d-%d-%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);

            sqlite3_close(db);
            return SB_RES_OK;
            // list pengeluaran dalam database
        }
        case 6: {
            char rowPengeluaran[11];
            char tanggalPengeluaran[11];

            sb_get_header(e->stream, "rowPengeluaran", rowPengeluaran, 10);
            sb_get_header(e->stream, "tanggalPengeluaran", tanggalPengeluaran, 10);

            if (!rowPengeluaran[0]) {
                sb_send_status(e->stream, 403, "Row Pengeluaran tidak boleh di kosongkan!");
                return SB_RES_OK;
            }

            sqlite3_open("database/pengeluaran.db", &db);
            if (tanggalPengeluaran[0]) sprintf(tempString, "DELETE FROM pengeluaran_%s where rowid=%s", tanggalPengeluaran, rowPengeluaran);
            else sprintf(tempString, "DELETE FROM pengeluaran_%d_%d_%d where rowid=%s", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, rowPengeluaran);

            if (!sqlNormalExec(e, db, tempString)) return SB_RES_OK;

            sb_send_status(e->stream, 200, "OK");
            return SB_RES_OK;
            // hapus pengeluaran dari database
        }
        case 7: {
            cJSON* resultJSON = cJSON_CreateObject();
            cJSON* tempObject = cJSON_CreateObject();
            cJSON* tempArray = cJSON_CreateArray();

            sb_get_body(e->stream, &bodyData);
            size_t splitCount = 0;
            char** valueSplit = strsplit(bodyData.data, "\n", &splitCount);
            sb_send_status(e->stream, 200, "OK");

            for (int a = 0; a < splitCount; a++) {
                sqlite3_open("database/pembukuan.db", &db);
                sprintf(tempString, "SELECT * FROM barangTerjual_%s", valueSplit[a]);

                if (sqlite3_exec(db, tempString, RowBack, &row, &errMsg) != SQLITE_OK) {
                    freeRowBack(&row);
                    if (!isStr(errMsg, "no such table", 0)) {
                        sb_send_status(e->stream, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
                        printf("[ERROR] Something wrong in SQLite at pembukuanLogic.c: %s\n", errMsg);
                        sqlite3_free(errMsg);
                        sqlite3_close(db);
                        free(bodyData.data);
                        return SB_RES_OK;
                    }
                    sqlite3_free(errMsg);
                    sqlite3_close(db);
                } else {
                    if (row.totalChar) {
                        size_t splitSize = 0;
                        char** strSplit = strsplit(row.rows, "\n", &splitSize);
                        for (int a = 0; a < splitSize - 1; a++) cJSON_AddItemToArray(tempArray, cJSON_CreateString(strSplit[a]));
                        cJSON_AddItemToObject(tempObject, valueSplit[a], tempArray);
                        tempArray = cJSON_CreateArray();
                        free(strSplit);
                    }

                    freeRowBack(&row);
                    sqlite3_close(db);
                }
            }

            cJSON_AddItemToObject(resultJSON, "barangTerjual", tempObject);
            tempObject = cJSON_CreateObject();

            for (int a = 0; a < splitCount; a++) {
                sqlite3_open("database/pengeluaran.db", &db);
                sprintf(tempString, "SELECT * FROM pengeluaran_%s", valueSplit[a]);

                if (sqlite3_exec(db, tempString, RowBack, &row, &errMsg) != SQLITE_OK) {
                    freeRowBack(&row);
                    if (!isStr(errMsg, "no such table", 0)) {
                        sb_send_status(e->stream, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
                        printf("[ERROR] Something wrong in SQLite at pembukuanLogic.c: %s\n", errMsg);
                        sqlite3_free(errMsg);
                        sqlite3_close(db);
                        free(bodyData.data);
                        return SB_RES_OK;
                    }
                    sqlite3_free(errMsg);
                    sqlite3_close(db);
                } else {
                    if (row.totalChar) {
                        size_t splitSize = 0;
                        char** strSplit = strsplit(row.rows, "\n", &splitSize);

                        for (int a = 0; a < splitSize - 1; a++) cJSON_AddItemToArray(tempArray, cJSON_CreateString(strSplit[a]));
                        cJSON_AddItemToObject(tempObject, valueSplit[a], tempArray);
                        tempArray = cJSON_CreateArray();
                        free(strSplit);
                    }

                    freeRowBack(&row);
                    sqlite3_close(db);
                }
            }

            cJSON_AddItemToObject(resultJSON, "pengeluaran", tempObject);

            resFree(e, cJSON_PrintUnformatted(resultJSON));
            cJSON_Delete(resultJSON);
            cJSON_Delete(tempArray);
            free(bodyData.data);
            free(valueSplit);
            // total pembukuan dari database (barang terjual, pengeluaran)
            return SB_RES_OK;
        }
        default: return SB_RES_OK;
    }
    return SB_RES_OK;
}
