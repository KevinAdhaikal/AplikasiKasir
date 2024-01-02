#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "../sqliteFunction.h"
#include "../telegramClient/telegramClient.h"
#include "../funcVarPub.h"
#include "../utils/utils.h"

#include "../../vendor/str/str.h"

#include "alarmFunction.h"

void* pembukuanAlarm(void* args) {
    teleThreadInfo.isAlarmPembukuanRun = 1;
    int second = *(int*)args;

    time_t raw_time;
    struct tm time_info;

    while(1) {
        time(&raw_time);
        time_info = *localtime(&raw_time);
        
        if ((time_info.tm_hour * 3600 + time_info.tm_min * 60) == second) {
            int tempInt = 0;
            sqlite3* db[2];
            char* errMsg;
            char tempString[1024];
            SQLRow rows = {0};
            Str resultMessage;

            char* formatCurrency1;
            char* formatCurrency2;
            char* formatCurrency3;
            char* formatCurrency4;

            str_init(&resultMessage);

            str_append_format(&resultMessage, "Total Pembukuan\n\n");

            sqlite3_open("database/pembukuan.db", &db[0]);
            sqlite3_open("database/pengeluaran.db", &db[1]);

            sprintf(tempString, "SELECT sum(jumlah),sum(modal),sum(jual),sum(jual - modal) from barangTerjual_%d_%d_%d", time_info.tm_mday, time_info.tm_mon + 1, time_info.tm_year + 1900);
            if (sqlite3_exec(db[0], tempString, RowBack, &rows, &errMsg) != SQLITE_OK) {
                if (!isStr(errMsg, "no such table", 0)) printf("%s\n", errMsg);
                else str_append_format(&resultMessage, "Tanggal %d_%d_%d\nTotal Jumlah Barang: 0\nTotal Harga Modal: Rp0\nTotal Harga Jual: Rp0\nTotal Keuntungan: Rp0\n", time_info.tm_mday, time_info.tm_mon + 1, time_info.tm_year + 1900);
                sqlite3_free(errMsg);
                freeRowBack(&rows);
            } else {
                char** valueSplit = strsplit(rows.rows, "|", 0);

                tempInt = atoi(valueSplit[2]);
                formatCurrency1 = formatCurrency(atoi(valueSplit[0]));
                formatCurrency2 = formatCurrency(atoi(valueSplit[1]));
                formatCurrency3 = formatCurrency(tempInt);
                formatCurrency4 = formatCurrency(atoi(valueSplit[3]));

                str_append_format(&resultMessage,
                "Tanggal %d_%d_%d\nTotal Jumlah Barang: %s\nTotal Harga Modal: Rp%s\nTotal Harga Jual: Rp%s\nTotal Keuntungan: Rp%s\n",
                time_info.tm_mday, time_info.tm_mon + 1, time_info.tm_year + 1900,
                formatCurrency1,
                formatCurrency2,
                formatCurrency3,
                formatCurrency4);

                free(formatCurrency1);
                free(formatCurrency2);
                free(formatCurrency4);
                free(valueSplit);

                freeRowBack(&rows);
            }

            sprintf(tempString, "SELECT sum(uang) from pengeluaran_%d_%d_%d", time_info.tm_mday, time_info.tm_mon + 1, time_info.tm_year + 1900);
            if (sqlite3_exec(db[1], tempString, RowBack, &rows, &errMsg) != SQLITE_OK) {
                if (!isStr(errMsg, "no such table", 0)) printf("%s\n", errMsg);
                else {
                    if (!tempInt) str_append_format(&resultMessage, "Total Pengeluaran: Rp0\nTotal Uang Yang Didapat: Rp0\n\n");
                    else str_append_format(&resultMessage, "Total Pengeluaran: Rp0\nTotal Uang Yang Didapat: Rp%s\n\n", formatCurrency3);
                }
                freeRowBack(&rows);
                sqlite3_free(errMsg);
            } else {
                formatCurrency1 = formatCurrency(atoi(rows.rows));
                formatCurrency2 = formatCurrency(tempInt - atoi(rows.rows));
                str_append_format(&resultMessage, "Total Pengeluaran: Rp%s\nTotal Uang Yang Didapat: Rp%s\n\n", formatCurrency1, formatCurrency2);
                free(formatCurrency1);
                free(formatCurrency2);
                free(formatCurrency3);
                freeRowBack(&rows);
            }
            tempInt = 0;
            
            sendMessageThread(resultMessage.value);
            str_finalize(&resultMessage);
            sleep(60);
            break;
        }

        sleep(1);
    }

    return NULL;
}