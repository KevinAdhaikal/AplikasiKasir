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
        case 6: {
            struct tm startDate = {0};
            struct tm endDate = {0};

            char dateTarget[2][20]; // [0] = Start Date, [1] = End Date
            sb_get_header(e->stream, "startDate", dateTarget[0], 19);
            sb_get_header(e->stream, "endDate", dateTarget[1], 19);

            if (!is_valid_date(dateTarget[0]) || !is_valid_date(dateTarget[1])) {
                printf("apakah bagian sini?\n");
                sb_send_status(e->stream, 403, "Mohon input tanggal yang valid!");
                return SB_RES_OK;
            }

            char** to_split[2];
            to_split[0] = strsplit(dateTarget[0], "_", 0);
            to_split[1] = strsplit(dateTarget[1], "_", 0);

            startDate.tm_mday = atoi(to_split[0][0]);
            startDate.tm_mon = atoi(to_split[0][1]) - 1;
            startDate.tm_year = atoi(to_split[0][2]) - 1900;
            startDate.tm_isdst = -1;

            endDate.tm_mday = atoi(to_split[1][0]);
            endDate.tm_mon = atoi(to_split[1][1]) - 1;
            endDate.tm_year = atoi(to_split[1][2]) - 1900;
            endDate.tm_isdst = -1;

            free(to_split[0]);
            free(to_split[1]);

            if (mktime(&endDate) < mktime(&startDate)) {
                printf("mungkin bagian sini?\n");
                sb_send_status(e->stream, 403, "Mohon input tanggal yang valid!");
                return SB_RES_OK;
            }
            char* resultDate = dateRange(&startDate, &endDate);

            size_t dateSplitTotal;
            char** dateSplit = strsplit(resultDate, "\n", &dateSplitTotal);
            free(resultDate);

            char** keyData = NULL;
            int* valueData = NULL;
            int keyTotal = 0;

            sqlite3_open("database/pembukuan.db", &db);

            for (int a = 0; a < dateSplitTotal - 1; a++) {
                sprintf(tempString, "SELECT nama, jumlah FROM barangTerjual_%s", dateSplit[a]);
                if (sqlite3_exec(db, tempString, RowBack, &rowBack, NULL) == SQLITE_OK) {
                    size_t splitTotal;
                    char** stringSplit = strsplit(rowBack.rows, "\n", &splitTotal);

                    for (int b = 0; b < splitTotal - 1; b++) {
                        char** valueSplit = strsplit(stringSplit[b], "|", 0);

                        if (keyTotal != 0) {
                            for (int c = 0; c < keyTotal; c++) {
                                if (isStr(keyData[c], valueSplit[0], 1)) {
                                    valueData[c] += atoi(valueSplit[1]);
                                    goto GO_HERE;
                                }
                            }
                            
                            keyTotal++;
                            keyData = realloc(keyData, keyTotal * sizeof(char*));
                            valueData = realloc(valueData, keyTotal * sizeof(int));

                            keyData[keyTotal - 1] = strdup(valueSplit[0]);
                            valueData[keyTotal - 1] = atoi(valueSplit[1]);
                        } else {
                            keyData = malloc(1 * sizeof(char*));
                            valueData = malloc(1 * sizeof(int));

                            keyData[0] = strdup(valueSplit[0]);
                            valueData[0] = atoi(valueSplit[1]);
                            keyTotal++;
                        }
                        GO_HERE:
                        free(valueSplit);
                    }
                    
                    free(stringSplit);
                    freeRowBack(&rowBack);
                }
            }

            for (int a = 0; a < keyTotal; a++) {
                sb_writef(e->stream, "%s|%d\n", keyData[a], valueData[a]);
                free(keyData[a]);
            }

            free(keyData);
            free(valueData);

            sqlite3_close(db);
            free(dateSplit);
            freeRowBack(&rowBack);
            return SB_RES_OK;
            // barang yang sering dibeli sama orang
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
