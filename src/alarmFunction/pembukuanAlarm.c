#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "../sqliteFunction.h"
#include "../telegramClient/telegramClient.h"
#include "../funcVarPub.h"
#include "../utils/utils.h"

#include "../../vendor/string_lib/string_lib.h"

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
            sqlite3* db[2];
            sqlite3_stmt* statement;
            SQLRow rows = {0};

            string resultMessage;
            string_init(&resultMessage);

            int tempInt = 0;
            char tempString[1024];
            
            char* formatCurrency1;
            char* formatCurrency2;
            char* formatCurrency3;
            char* formatCurrency4;

            string_add(&resultMessage, "Total Pembukuan\n\n");

            sqlite3_open("database/pembukuan.db", &db[0]);
            sqlite3_open("database/pengeluaran.db", &db[1]);

            sprintf(tempString, "SELECT sum(jumlah),sum(modal),sum(jual),sum(jual - modal) from barangTerjual_%d_%d_%d", time_info.tm_mday, time_info.tm_mon + 1, time_info.tm_year + 1900);
            sqlite3_prepare_v2(db[0], tempString, -1, &statement, NULL);
            statement_get_row(statement, &rows, 1);
            sqlite3_finalize(statement);

            if (rows.sql_ret != SQLITE_ROW) string_add_format(&resultMessage, "Tanggal %d_%d_%d\nTotal Jumlah Barang: 0\nTotal Harga Modal: Rp0\nTotal Harga Jual: Rp0\nTotal Keuntungan: Rp0\n", time_info.tm_mday, time_info.tm_mon + 1, time_info.tm_year + 1900);
            else {
                char** valueSplit = strsplit(rows.rows, "\x02", 0);

                tempInt = atoll(valueSplit[2]);
                formatCurrency1 = formatCurrency(atoll(valueSplit[0]));
                formatCurrency2 = formatCurrency(atoll(valueSplit[1]));
                formatCurrency3 = formatCurrency(atoll(valueSplit[2]));
                formatCurrency4 = formatCurrency(atoll(valueSplit[3]));

                string_add_format(&resultMessage,
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
            }

            freeRowBack(&rows);

            sprintf(tempString, "SELECT sum(uang) from pengeluaran_%d_%d_%d", time_info.tm_mday, time_info.tm_mon + 1, time_info.tm_year + 1900);
            sqlite3_prepare_v2(db[1], tempString, -1, &statement, NULL);
            
            if (sqlite3_step(statement) == SQLITE_ROW) {
                formatCurrency1 = formatCurrency(sqlite3_column_int(statement, 0));
                formatCurrency2 = formatCurrency(tempInt - sqlite3_column_int(statement, 0));
                string_add_format(&resultMessage, "Total Pengeluaran: Rp%s\nTotal Uang Yang Didapat: Rp%s\n\n", formatCurrency1, formatCurrency2);
                free(formatCurrency1);
                free(formatCurrency2);
            } else {
                if (!tempInt) string_add_format(&resultMessage, "Total Pengeluaran: Rp0\nTotal Uang Yang Didapat: Rp0\n\n");
                else string_add_format(&resultMessage, "Total Pengeluaran: Rp0\nTotal Uang Yang Didapat: Rp%s\n\n", formatCurrency3);
            }

            free(formatCurrency3);
            sqlite3_finalize(statement);
            tempInt = 0;
            
            sendMessageThread(resultMessage.val);
            string_finalize(&resultMessage);
            sqlite3_close(db[0]);
            sqlite3_close(db[1]);
            sleep(60);
            break;
        }
        delay(0.5);
    }
    return NULL;
}