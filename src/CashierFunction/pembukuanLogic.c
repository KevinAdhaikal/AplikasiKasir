#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "../sqliteFunction.h"
#include "../../vendor/sqlite3/sqlite3.h"
#include "../../vendor/httplib/httplibrary.h"
#include "../utils/utils.h"
#include "../../vendor/cJSON/cJSON.h"
#include "../funcVarPub.h"
#include "../telegramClient/telegramClient.h"

#include "cashierFunction.h"

void resFree(http_event* ev, char* data) {
    http_write(ev, data, strlen(data));
    cJSON_free(data);
}

void pembukuan(http_event* e) {
    char tempString[1024];
    char tanggalPembukuan[11];
    
    char* body;
    char* errMsg;
    sqlite3* db;
    SQLRow row = {0};
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    switch(http_get_query_to_int(e, "pembukuanArgs")) {
        case 1: {
            size_t newLineTotal;
            if (e->headers.body_pos == -1) return;
            char** splitData = strsplit(e->headers.raw_header + e->headers.body_pos, "\n", &newLineTotal);

            sqlite3_open("database/pembukuan.db", &db);
            sprintf(tempString, "CREATE TABLE IF NOT EXISTS barangTerjual_%d_%d_%d (nama TEXT, jumlah INT, modal INT, jual INT);", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
            if (!sqlNormalExec(e, db, tempString)) {
                free(splitData);
                return;
            }

            sqlite3_close(db);

            for (int a = 0; a < newLineTotal; a++) {
                sqlite3_open("database/pembukuan.db", &db);
                char** valueData = strsplit(splitData[a], "|", 0);
                sprintf(tempString, "INSERT INTO barangTerjual_%d_%d_%d (nama, jumlah, modal, jual) values ('%s', %d,  %d, %d);", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, valueData[0],  atoi(valueData[1]), atoi(valueData[2]), atoi(valueData[3]));
                if (!sqlNormalExec(e, db, tempString)) {
                    free(valueData);
                    free(splitData);
                    return;
                }

                sqlite3_close(db);
                sqlite3_open("database/daftarBarang.db", &db);

                sprintf(tempString, "UPDATE daftarBarang SET jumlah =jumlah - %d WHERE nama = '%s';", atoi(valueData[1]), valueData[0]);
                if (!sqlNormalExec(e, db, tempString)) {
                    free(valueData);
                    free(splitData);
                    return;
                }

                free(valueData);
                sqlite3_close(db);
            }

            free(splitData);

            http_send_status(e, 200, "OK");
            sqlite3_close(db);
            return;

            // barang terjual masukan ke database
        }
        case 2: {
            int isTrue = 0;
            http_get_header(e, "tanggalPembukuan", tanggalPembukuan, 10);
            sqlite3_open("database/pembukuan.db", &db);

            if (!tanggalPembukuan[0]) {
                sprintf(tempString, "SELECT EXISTS (SELECT name FROM sqlite_master WHERE type='table' AND name='barangTerjual_%d_%d_%d');", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
                if (!sqlBackExec(e, db, tempString, sqlReturnInt, &isTrue)) return;

                if (!isTrue) {
                    sprintf(tempString, "%d-%d-%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
                    http_write(e, tempString, strlen(tempString));
                } else {
                    sprintf(tempString, "SELECT rowid,* from barangTerjual_%d_%d_%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);

                    if (!sqlBackExec(e, db, tempString, RowBack, &row)) {
                        freeRowBack(&row);
                        return;
                    }

                    http_write(e, row.rows, row.totalChar);
                    sprintf(tempString, "%d-%d-%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
                    http_write(e, tempString, strlen(tempString));
                    freeRowBack(&row);
                }
            } else {
                sprintf(tempString, "SELECT EXISTS (SELECT name FROM sqlite_master WHERE type='table' AND name='barangTerjual_%s');", tanggalPembukuan);
                if (!sqlBackExec(e, db, tempString, sqlReturnInt, &isTrue)) return;
                
                if (!isTrue) {
                    sprintf(tempString, "%d-%d-%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
                    http_write(e, tempString, strlen(tempString));
                }
                else {
                    sprintf(tempString, "SELECT rowid,* from barangTerjual_%s", tanggalPembukuan);

                    if (!sqlBackExec(e, db, tempString, RowBack, &row)) {
                        freeRowBack(&row);
                        return;
                    }

                    http_send_status(e, 200, "OK");
                    http_write(e, row.rows, row.totalChar);
                    sprintf(tempString, "%d-%d-%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
                    http_write(e, tempString, strlen(tempString));
                    freeRowBack(&row);
                }
            }
            sqlite3_close(db);
            return;
            
            // list barang terjual
        }
        case 3: {
            char idBarang[11];
            char tanggalPembukuan[11];
            char namaBarang[255];
            int totalJumlah = 0;

            http_get_query(e, "idBarang", idBarang, 10);
            http_get_header(e, "tanggalPembukuan", tanggalPembukuan, 10);
            http_get_header(e, "namaBarang", namaBarang, 254);

            if (!idBarang[0] && !namaBarang[0]) {
                http_send_status(e, 403, "ID dan Nama Barang tidak boleh kosong!");
                return;
            }

            if (tanggalPembukuan[0]) {
                if (namaBarang[0]) {
                    sprintf(tempString, "SELECT jumlah FROM barangTerjual_%s WHERE nama='%s'", tanggalPembukuan, namaBarang);
                    sqlite3_open("database/pembukuan.db", &db);
                    if (!sqlBackExec(e, db, tempString, RowBackPlus, &totalJumlah)) return;

                    sprintf(tempString, "DELETE FROM barangTerjual_%s WHERE nama='%s'", tanggalPembukuan, namaBarang);
                    if (!sqlNormalExec(e, db, tempString)) return;

                    sqlite3_close(db);
                    sqlite3_open("database/daftarBarang.db", &db);

                    sprintf(tempString, "UPDATE daftarBarang SET jumlah=jumlah + %d WHERE nama='%s'", totalJumlah, namaBarang);
                    if (!sqlNormalExec(e, db, tempString)) return;
                    sqlite3_close(db);
                }
                else {
                    sprintf(tempString, "SELECT * FROM barangTerjual_%s WHERE rowid=%s", tanggalPembukuan, idBarang);
                    sqlite3_open("database/pembukuan.db", &db);
                    if (!sqlBackExec(e, db, tempString, RowBack, &row)) {
                        freeRowBack(&row);
                        return;
                    }

                    sprintf(tempString, "DELETE FROM barangTerjual_%s WHERE rowid=%s", tanggalPembukuan, idBarang);

                    if (!sqlBackExec(e, db, tempString, RowBack, &row)) {
                        freeRowBack(&row);
                        return;
                    }

                    sqlite3_close(db);
                    sqlite3_open("database/daftarBarang.db", &db);

                    char** splitstring = strsplit(row.rows, "|", 0);
                    sprintf(tempString, "UPDATE daftarBarang SET jumlah=jumlah + %d WHERE nama='%s'", atoi(splitstring[1]), splitstring[0]);
                    freeRowBack(&row);
                    free(splitstring);

                    if (!sqlNormalExec(e, db, tempString)) return;

                    sqlite3_close(db);
                }
            } else {
                if (namaBarang[0]) {
                    sprintf(tempString, "SELECT jumlah FROM barangTerjual_%d_%d_%d WHERE nama='%s'", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, namaBarang);
                    sqlite3_open("database/pembukuan.db", &db);

                    if (!sqlBackExec(e, db, tempString, RowBackPlus, &totalJumlah)) return;

                    sprintf(tempString, "DELETE FROM barangTerjual_%d_%d_%d where nama=%s", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, namaBarang);
                    
                    if (!sqlNormalExec(e, db, tempString)) return;
                    sqlite3_close(db);

                    sqlite3_open("database/daftarBarang.db", &db);
                    sprintf(tempString, "UPDATE daftarBarang SET jumlah=jumlah + %d WHERE nama='%s'", totalJumlah, namaBarang);
                    if (!sqlNormalExec(e, db, tempString)) return;

                    sqlite3_close(db);
                }
                else {
                    sprintf(tempString, "SELECT * FROM barangTerjual_%d_%d_%d WHERE rowid=%s", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, idBarang);
                    sqlite3_open("database/pembukuan.db", &db);

                    if (!sqlBackExec(e, db, tempString, RowBack, &row)) {
                        freeRowBack(&row);
                        return;
                    }

                    sprintf(tempString, "DELETE FROM barangTerjual_%d_%d_%d where rowid=%s", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, idBarang);
                    if (!sqlNormalExec(e, db, tempString)) return;
                    sqlite3_close(db);

                    sqlite3_open("database/daftarBarang.db", &db);
                    char** splitstring = strsplit(row.rows, "|", 0);
                    sprintf(tempString, "UPDATE daftarBarang SET jumlah=jumlah + %d WHERE nama='%s'", atoi(splitstring[1]), splitstring[0]);
                    freeRowBack(&row);
                    free(splitstring);

                    if (!sqlNormalExec(e, db, tempString)) return;
                    sqlite3_close(db);
                }
            }
            http_send_status(e, 200, "OK");
            // hapus barang terjual
            return;
        }
        case 4: {
            char descPengeluaran[255];
            char uangPengeluaran[11];

            http_get_header(e, "descPengeluaran", descPengeluaran, 254);
            http_get_header(e, "uangPengeluaran", uangPengeluaran, 10);

            if (!descPengeluaran[0]) {
                http_send_status(e, 403, "Deskripsi Pengeluaran tidak boleh dikosongkan!");
                return;
            }
            if (!uangPengeluaran[0]) {
                http_send_status(e, 403, "Uang Pengeluaran tidak boleh dikosongkan!");
                return;
            }

            sqlite3_open("database/pengeluaran.db", &db);

            sprintf(tempString, "CREATE TABLE IF NOT EXISTS pengeluaran_%d_%d_%d (desc TEXT, uang INT);", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
            if (!sqlNormalExec(e, db, tempString)) return;
            sqlite3_close(db);

            sqlite3_open("database/pengeluaran.db", &db);

            sprintf(tempString, "INSERT INTO pengeluaran_%d_%d_%d (desc, uang) VALUES ('%s', %s);", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, descPengeluaran, uangPengeluaran);
            if (!sqlNormalExec(e, db, tempString)) return;
            sqlite3_close(db);
            http_send_status(e, 200, "OK");
            return;
            // tambah pengeluaran ke dalam database
        }
        case 5: {
            char tanggalPengeluaran[11];

            sqlite3_open("database/pengeluaran.db", &db);
            http_get_header(e, "tanggalPengeluaran", tanggalPengeluaran, 10);

            if (tanggalPengeluaran[0]) sprintf(tempString, "SELECT rowid,* FROM pengeluaran_%s", tanggalPengeluaran);
            else sprintf(tempString, "SELECT rowid,* FROM pengeluaran_%d_%d_%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);

            if (sqlite3_exec(db, tempString, RowBack, &row, &errMsg) != SQLITE_OK) {
                freeRowBack(&row);
                if (isStr(errMsg, "no such table", 0)) {
                    sprintf(tempString, "%d-%d-%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
                    http_write(e, tempString, strlen(tempString));
                }
                sqlite3_free(errMsg);
                sqlite3_close(db);
                return;
            }

            http_write(e, row.rows, row.totalChar);
            sprintf(tempString, "%d-%d-%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
            http_write(e, tempString, strlen(tempString));

            sqlite3_close(db);
            return;
            // list pengeluaran dalam database
        }
        case 6: {
            char rowPengeluaran[11];
            char tanggalPengeluaran[11];

            http_get_header(e, "rowPengeluaran", rowPengeluaran, 10);
            http_get_header(e, "tanggalPengeluaran", tanggalPengeluaran, 10);

            if (!rowPengeluaran[0]) {
                http_send_status(e, 403, "Row Pengeluaran tidak boleh di kosongkan!");
                return;
            }

            sqlite3_open("database/pengeluaran.db", &db);
            if (tanggalPengeluaran[0]) sprintf(tempString, "DELETE FROM pengeluaran_%s where rowid=%s", tanggalPengeluaran, rowPengeluaran);
            else sprintf(tempString, "DELETE FROM pengeluaran_%d_%d_%d where rowid=%s", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, rowPengeluaran);

            if (!sqlNormalExec(e, db, tempString)) return;

            http_send_status(e, 200, "OK");
            return;
            // hapus pengeluaran dari database
        }
        case 7: {
            cJSON* resultJSON = cJSON_CreateObject();
            cJSON* tempObject = cJSON_CreateObject();
            cJSON* tempArray = cJSON_CreateArray();

            size_t splitCount = 0;
            if (e->headers.body_pos == -1) return;
            char** valueSplit = strsplit(e->headers.raw_header + e->headers.body_pos, "\n", &splitCount);
            http_send_status(e, 200, "OK");

            for (int a = 0; a < splitCount; a++) {
                sqlite3_open("database/pembukuan.db", &db);
                sprintf(tempString, "SELECT * FROM barangTerjual_%s", valueSplit[a]);

                if (sqlite3_exec(db, tempString, RowBack, &row, &errMsg) != SQLITE_OK) {
                    freeRowBack(&row);
                    if (!isStr(errMsg, "no such table", 0)) {
                        http_send_status(e, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
                        printf("[ERROR] Something wrong in SQLite at pembukuanLogic.c: %s\n", errMsg);
                        sqlite3_free(errMsg);
                        sqlite3_close(db);
                        return;
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
                        http_send_status(e, 403, "Ada yang salah pada AplikasiKasir, harap hubungi Pemilik");
                        printf("[ERROR] Something wrong in SQLite at pembukuanLogic.c: %s\n", errMsg);
                        sqlite3_free(errMsg);
                        sqlite3_close(db);
                        return;
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
            free(valueSplit);
            // total pembukuan dari database (barang terjual, pengeluaran)
            return;
        }
        default: return;
    }
    return;
}
