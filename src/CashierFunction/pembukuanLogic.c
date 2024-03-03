#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include "../sqliteFunction.h"
#include "../../vendor/sqlite3/sqlite3.h"
#include "../../vendor/sandbird/sandbird.h"
#include "../utils/utils.h"
#include "../../vendor/cJSON/cJSON.h"
#include "../funcVarPub.h"
#include "../telegramClient/telegramClient.h"

#include "cashierFunction.h"

pthread_t telegramThread;

void resFree(sb_Event* ev, char* data) {
    sb_writef(ev->stream, "%s", data);
    cJSON_free(data);
}

int pembukuan(sb_Event* e) {
    char tempString[1024];
    char tanggalPembukuan[32];
    sqlite3* db, *db2;
    sqlite3_stmt* statement;
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
            char** splitData = strsplit((char*)bodyData.data, "\x01", &newLineTotal);

            sqlite3_open("database/pembukuan.db", &db);
            sqlite3_open("database/daftarBarang.db", &db2);

            sprintf(tempString, "CREATE TABLE IF NOT EXISTS barangTerjual_%d_%d_%d (nama TEXT, jumlah INT, modal INT, jual INT, waktu TEXT DEFAULT (strftime('%%H:%%M:%%S', 'now', 'localtime')));", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
            sqlite3_exec(db, tempString, 0, 0, NULL);

            for (int a = 0; a < newLineTotal; a++) {
                char** valueData = strsplit(splitData[a], "\x02", 0);
                sprintf(tempString, "INSERT INTO barangTerjual_%d_%d_%d (nama, jumlah, modal, jual) values (?, ?, ?, ?)", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
                sqlite3_prepare_v2(db, tempString, -1, &statement, NULL);
                sqlite3_bind_text(statement, 1, valueData[0], -1, SQLITE_STATIC);
                sqlite3_bind_int64(statement, 2, atoll(valueData[1]));
                sqlite3_bind_int64(statement, 3, atoll(valueData[2]));
                sqlite3_bind_int64(statement, 4, atoll(valueData[3]));
                sqlite3_step(statement);
                sqlite3_finalize(statement);

                sqlite3_prepare_v2(db2, "UPDATE daftarBarang SET jumlah = jumlah - ? WHERE nama = ?", -1, &statement, NULL);
                sqlite3_bind_int64(statement, 1, atoll(valueData[1]));
                sqlite3_bind_text(statement, 2, valueData[0], -1, SQLITE_STATIC);
                sqlite3_step(statement);
                sqlite3_finalize(statement);

                if (teleBot.usingTelegramBot && (teleBot.notifyBarangKosongTGram || teleBot.isNotifyBarangDibawahJumlah)) {
                    long long int jumlahBarang = 0;
                    sqlite3_prepare_v2(db2, "SELECT jumlah FROM daftarBarang where nama = ?", -1, &statement, NULL);
                    sqlite3_bind_text(statement, 1, valueData[0], -1, SQLITE_STATIC);
                    sqlite3_step(statement);
                    jumlahBarang = sqlite3_column_int64(statement, 0);
                    sqlite3_finalize(statement);

                    if (teleBot.notifyBarangKosongTGram && jumlahBarang <= 0) {
                        pthread_create(&telegramThread, NULL, sendMessageThread, dynamic_string_format("[PERINGATAN] Barang bernama '%s' Sudah Kosong!", valueData[0]));
                        pthread_detach(telegramThread);
                    }
                    if (teleBot.isNotifyBarangDibawahJumlah && jumlahBarang <= teleBot.targetNotifyBarangDibawahJumlah) {
                        pthread_create(&telegramThread, NULL, sendMessageThread, dynamic_string_format("[PERINGATAN] Barang bernama '%s' Stock barang sudah dibawah %d!", valueData[0], teleBot.targetNotifyBarangDibawahJumlah));
                        pthread_detach(telegramThread);
                    }
                }

                free(valueData);
            }

            free(splitData);
            free(bodyData.data);

            sb_send_status(e->stream, 200, "OK");
            sqlite3_close(db);
            sqlite3_close(db2);
            return SB_RES_OK;
        }
        case 2: {
            int isTrue = 0;
            sb_get_header(e->stream, "tanggalPembukuan", tanggalPembukuan, 31);
            sqlite3_open("database/pembukuan.db", &db);

            if (!tanggalPembukuan[0]) {
                sprintf(tempString, "SELECT EXISTS (SELECT name FROM sqlite_master WHERE type='table' AND name='barangTerjual_%d_%d_%d');", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
                if (!sqlBackExec(e, db, tempString, sqlReturnInt, &isTrue)) return SB_RES_OK;

                if (!isTrue) {
                    sb_send_status(e->stream, 200, "OK");
                    sb_writef(e->stream, "%d-%d-%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
                } else {
                    sprintf(tempString, "SELECT rowid,waktu,nama,jumlah,modal,jual from barangTerjual_%d_%d_%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);

                    sqlite3_prepare_v2(db, tempString, -1, &statement, NULL);
                    statement_get_row(statement, &row, 0);
                    sqlite3_finalize(statement);

                    sb_write(e->stream, row.rows, row.totalChar);
                    sb_writef(e->stream, "%d-%d-%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
                    freeRowBack(&row);
                }
            } else {
                if (!is_valid_date(tanggalPembukuan)) {
                    sb_send_status(e->stream, 403, "Tanggal tidak valid!");
                    return SB_RES_OK;
                }
                sprintf(tempString, "SELECT EXISTS (SELECT name FROM sqlite_master WHERE type='table' AND name='barangTerjual_%s');", tanggalPembukuan);
                if (!sqlBackExec(e, db, tempString, sqlReturnInt, &isTrue)) return SB_RES_OK;

                if (!isTrue) {
                    sb_send_status(e->stream, 200, "OK");
                    sb_writef(e->stream, "%d-%d-%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
                }
                else {
                    sprintf(tempString, "SELECT rowid,waktu,nama,jumlah,modal,jual from barangTerjual_%s", tanggalPembukuan);
                    
                    sqlite3_prepare_v2(db, tempString, -1, &statement, NULL);
                    statement_get_row(statement, &row, 0);
                    sqlite3_finalize(statement);

                    sb_write(e->stream, row.rows, row.totalChar);
                    sb_writef(e->stream, "%d-%d-%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
                    freeRowBack(&row);
                }
            }
            sqlite3_close(db);
            return SB_RES_OK;
        }
        case 3: {
            char idBarang[32];
            char namaBarang[255];

            sb_get_var(e->stream, "idBarang", idBarang, 31);
            sb_get_header(e->stream, "tanggalPembukuan", tanggalPembukuan, 31);
            sb_get_header(e->stream, "namaBarang", namaBarang, 254);

            if (!idBarang[0] && !namaBarang[0]) {
                sb_send_status(e->stream, 403, "ID dan Nama Barang tidak boleh kosong!");
                return SB_RES_OK;
            }

            if (tanggalPembukuan[0]) {
                if (!is_valid_date(tanggalPembukuan)) {
                    sb_send_status(e->stream, 403, "Tanggal tidak valid!");
                    return SB_RES_OK;
                }
                if (namaBarang[0]) {
                    sqlite3_open("database/pembukuan.db", &db);

                    sprintf(tempString, "SELECT jumlah FROM barangTerjual_%s WHERE nama = ?", tanggalPembukuan);
                    sqlite3_prepare_v2(db, tempString, -1, &statement, NULL);
                    sqlite3_bind_text(statement, 1, namaBarang, -1, SQLITE_STATIC);
                    sqlite3_finalize(statement);

                    sprintf(tempString, "DELETE FROM barangTerjual_%s WHERE nama = ?", tanggalPembukuan);
                    sqlite3_prepare_v2(db, tempString, -1, &statement, NULL);
                    sqlite3_bind_text(statement, 1, namaBarang, -1, SQLITE_STATIC);
                    sqlite3_step(statement);
                    sqlite3_finalize(statement);

                    sqlite3_close(db);
                    sqlite3_open("database/daftarBarang.db", &db);

                    sprintf(tempString, "UPDATE daftarBarang SET jumlah=jumlah + ? WHERE nama = ?");
                    sqlite3_prepare_v2(db, tempString, -1, &statement, NULL);
                    sqlite3_bind_int64(statement, 1, statement_row_plus(statement));
                    sqlite3_bind_text(statement, 2, namaBarang, -1, SQLITE_STATIC);
                    sqlite3_step(statement);
                    sqlite3_finalize(statement);
                    sqlite3_close(db);
                }
                else {
                    sprintf(tempString, "SELECT * FROM barangTerjual_%s WHERE rowid = ?", tanggalPembukuan);
                    sqlite3_open("database/pembukuan.db", &db);
                    sqlite3_prepare_v2(db, tempString, -1, &statement, NULL);
                    sqlite3_bind_int64(statement, 1, atoll(idBarang));
                    statement_get_row(statement, &row, 0);
                    sqlite3_finalize(statement);

                    sprintf(tempString, "DELETE FROM barangTerjual_%s WHERE rowid = ?", tanggalPembukuan);
                    sqlite3_prepare_v2(db, tempString, -1, &statement, NULL);
                    sqlite3_bind_int64(statement, 1, atoll(idBarang));
                    sqlite3_step(statement);
                    sqlite3_finalize(statement);

                    sqlite3_close(db);
                    sqlite3_open("database/daftarBarang.db", &db);

                    char** splitstring = strsplit(row.rows, "\x02", 0);
                    sqlite3_prepare_v2(db, "UPDATE daftarBarang SET jumlah=jumlah + ? WHERE nama = ?", -1, &statement, NULL);
                    sqlite3_bind_int64(statement, 1, atoll(splitstring[1]));
                    sqlite3_bind_text(statement, 2, splitstring[0], -1, SQLITE_STATIC);
                    sqlite3_step(statement);
                    sqlite3_finalize(statement);
                    sqlite3_close(db);

                    freeRowBack(&row);
                    free(splitstring);
                }
            } else {
                if (namaBarang[0]) {
                    sqlite3_open("database/pembukuan.db", &db);

                    sprintf(tempString, "SELECT jumlah FROM barangTerjual_%d_%d_%d WHERE nama = ?", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
                    sqlite3_prepare_v2(db, tempString, -1, &statement, NULL);
                    sqlite3_bind_text(statement, 1, namaBarang, -1, SQLITE_STATIC);
                    sqlite3_finalize(statement);

                    sprintf(tempString, "DELETE FROM barangTerjual_%d_%d_%d where nama = ?", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);

                    sqlite3_prepare_v2(db, tempString, -1, &statement, NULL);
                    sqlite3_bind_text(statement, 1, namaBarang, -1, SQLITE_STATIC);
                    sqlite3_step(statement);
                    sqlite3_finalize(statement);
                    sqlite3_close(db);

                    sqlite3_open("database/daftarBarang.db", &db);
                    sqlite3_prepare_v2(db, "UPDATE daftarBarang SET jumlah=jumlah + ? WHERE nama = ?", -1, &statement, NULL);
                    sqlite3_bind_int64(statement, 1, statement_row_plus(statement));
                    sqlite3_bind_text(statement, 2, namaBarang, -1, SQLITE_STATIC);
                    sqlite3_step(statement);
                    sqlite3_finalize(statement);
                    sqlite3_close(db);
                }
                else {
                    sprintf(tempString, "SELECT * FROM barangTerjual_%d_%d_%d WHERE rowid = ?", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
                    sqlite3_open("database/pembukuan.db", &db);
                    sqlite3_prepare_v2(db, tempString, -1, &statement, NULL);
                    sqlite3_bind_int64(statement, 1, atoll(idBarang));
                    statement_get_row(statement, &row, 0);
                    sqlite3_finalize(statement);

                    sprintf(tempString, "DELETE FROM barangTerjual_%d_%d_%d where rowid = ?", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
                    sqlite3_prepare_v2(db, tempString, -1, &statement, NULL);
                    sqlite3_bind_int64(statement, 1, atoll(idBarang));
                    sqlite3_step(statement);
                    sqlite3_finalize(statement);

                    sqlite3_close(db);

                    sqlite3_open("database/daftarBarang.db", &db);
                    char** splitstring = strsplit(row.rows, "\x02", 0);
                    sqlite3_prepare_v2(db, "UPDATE daftarBarang SET jumlah=jumlah + ? WHERE nama = ?", -1, &statement, NULL);
                    sqlite3_bind_int64(statement, 1, atoll(splitstring[1]));
                    sqlite3_bind_text(statement, 2, splitstring[0], -1, SQLITE_STATIC);
                    sqlite3_step(statement);
                    sqlite3_finalize(statement);

                    freeRowBack(&row);
                    free(splitstring);

                    sqlite3_close(db);
                }
            }
            sb_send_status(e->stream, 200, "OK");
            // hapus barang terjual
            return SB_RES_OK;
        }
        case 4: {
            char descPengeluaran[255];
            char uangPengeluaran[32];

            sb_get_header(e->stream, "descPengeluaran", descPengeluaran, 254);
            sb_get_header(e->stream, "uangPengeluaran", uangPengeluaran, 31);

            if (!descPengeluaran[0]) {
                sb_send_status(e->stream, 403, "Deskripsi Pengeluaran tidak boleh dikosongkan!");
                return SB_RES_OK;
            }
            if (!uangPengeluaran[0]) {
                sb_send_status(e->stream, 403, "Uang Pengeluaran tidak boleh dikosongkan!");
                return SB_RES_OK;
            }

            sqlite3_open("database/pengeluaran.db", &db);

            sprintf(tempString, "CREATE TABLE IF NOT EXISTS pengeluaran_%d_%d_%d (desc TEXT, uang INT, waktu TEXT DEFAULT (strftime('%%H:%%M:%%S', 'now', 'localtime')));", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
            if (!sqlNormalExec(e, db, tempString)) return SB_RES_OK;

            sprintf(tempString, "INSERT INTO pengeluaran_%d_%d_%d (desc, uang) VALUES (?, ?);", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
            sqlite3_prepare_v2(db, tempString, -1, &statement, NULL);
            sqlite3_bind_text(statement, 1, descPengeluaran, -1, SQLITE_STATIC);
            sqlite3_bind_int64(statement, 2, atoll(uangPengeluaran));
            sqlite3_step(statement);
            sqlite3_finalize(statement);

            sqlite3_close(db);
            sb_send_status(e->stream, 200, "OK");
            return SB_RES_OK;
            // tambah pengeluaran ke dalam database
        }
        case 5: {
            char tanggalPengeluaran[32];

            sqlite3_open("database/pengeluaran.db", &db);
            sb_get_header(e->stream, "tanggalPengeluaran", tanggalPengeluaran, 31);

            if (tanggalPengeluaran[0]) {
                if (!is_valid_date(tanggalPengeluaran)) {
                    sb_send_status(e->stream, 403, "Tanggal tidak valid!");
                    return SB_RES_OK;
                }
                sprintf(tempString, "SELECT rowid,waktu,desc,uang FROM pengeluaran_%s", tanggalPengeluaran);
            }
            else sprintf(tempString, "SELECT rowid,waktu,desc,uang FROM pengeluaran_%d_%d_%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);

            sqlite3_prepare_v2(db, tempString, -1, &statement, NULL);
            statement_get_row(statement, &row, 0);
            sqlite3_finalize(statement);
            sqlite3_close(db);

            sb_write(e->stream, row.rows, row.totalChar);
            sb_writef(e->stream, "%d-%d-%d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
            return SB_RES_OK;
            // list pengeluaran dalam database
        }
        case 6: {
            char rowPengeluaran[32];
            char tanggalPengeluaran[32];

            sb_get_header(e->stream, "rowPengeluaran", rowPengeluaran, 31);
            sb_get_header(e->stream, "tanggalPengeluaran", tanggalPengeluaran, 31);

            if (!rowPengeluaran[0]) {
                sb_send_status(e->stream, 403, "Row Pengeluaran tidak boleh di kosongkan!");
                return SB_RES_OK;
            }

            sqlite3_open("database/pengeluaran.db", &db);
            if (tanggalPengeluaran[0]) {
                if (!is_valid_date(tanggalPengeluaran)) {
                    sb_send_status(e->stream, 403, "Tanggal tidak valid!");
                    return SB_RES_OK;
                }
                
                sprintf(tempString, "DELETE FROM pengeluaran_%s where rowid = ?", tanggalPengeluaran);
            }
            else sprintf(tempString, "DELETE FROM pengeluaran_%d_%d_%d where rowid = ?", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);

            sqlite3_prepare_v2(db, tempString, -1, &statement, NULL);
            sqlite3_bind_int64(statement, 1, atoll(rowPengeluaran));
            sqlite3_step(statement);
            sqlite3_finalize(statement);
            sqlite3_close(db);

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
            char** valueSplit = strsplit((char*)bodyData.data, "\x01", &splitCount);
            sqlite3_open("database/pembukuan.db", &db);

            for (int a = 0; a < splitCount; a++) {
                if (!is_valid_date(valueSplit[a])) {
                    sb_send_status(e->stream, 403, "Tanggal tidak valid!");
                    cJSON_Delete(resultJSON);
                    cJSON_Delete(tempArray);
                    cJSON_Delete(tempObject);
                    return SB_RES_OK;
                }
                
                sprintf(tempString, "SELECT * FROM barangTerjual_%s", valueSplit[a]);
                sqlite3_prepare_v2(db, tempString, -1, &statement, NULL);
                statement_get_row(statement, &row, 0);
                sqlite3_finalize(statement);

                if (row.totalChar) {
                    size_t splitSize = 0;
                    char** strSplit = strsplit(row.rows, "\x01", &splitSize);
                    for (int a = 0; a < splitSize - 1; a++) cJSON_AddItemToArray(tempArray, cJSON_CreateString(strSplit[a]));
                    cJSON_AddItemToObject(tempObject, valueSplit[a], tempArray);
                    tempArray = cJSON_CreateArray();
                    free(strSplit);
                }

                freeRowBack(&row);
            }
            sqlite3_close(db);

            cJSON_AddItemToObject(resultJSON, "barangTerjual", tempObject);
            tempObject = cJSON_CreateObject();
            sqlite3_open("database/pengeluaran.db", &db);

            for (int a = 0; a < splitCount; a++) {
                sprintf(tempString, "SELECT * FROM pengeluaran_%s", valueSplit[a]);
                sqlite3_prepare_v2(db, tempString, -1, &statement, NULL);
                statement_get_row(statement, &row, 0);
                sqlite3_finalize(statement);

                if (row.totalChar) {
                    size_t splitSize = 0;
                    char** strSplit = strsplit(row.rows, "\x01", &splitSize);

                    for (int a = 0; a < splitSize - 1; a++) cJSON_AddItemToArray(tempArray, cJSON_CreateString(strSplit[a]));
                    cJSON_AddItemToObject(tempObject, valueSplit[a], tempArray);
                    tempArray = cJSON_CreateArray();
                    free(strSplit);
                }

                freeRowBack(&row);
            }
            sqlite3_close(db);

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
