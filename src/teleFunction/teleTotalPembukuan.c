#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../utils/utils.h"
#include "../../vendor/sqlite3/sqlite3.h"
#include "../sqliteFunction.h"
#include "../../vendor/string_lib/string_lib.h"
#include "../telegramClient/telegramClient.h"
#include "../funcVarPub.h"

#include "teleFunction.h"

int teleTotalPembukuan(sb_Event* e) {
    int tempInt = 0;
    sqlite3* db[2];
    sqlite3_stmt* statement;
    char tempString[1024];
    SQLRow rows = {0};
    sb_Body body = {0};
    string resultMessage;

    char* formatCurrency1;
    char* formatCurrency2;
    char* formatCurrency3;
    char* formatCurrency4;

    string_init(&resultMessage);
    string_add(&resultMessage, "Total Pembukuan\n\n");
    sb_get_body(e->stream, &body);

    size_t splitSize = 0;
    char** stringSplit = strsplit((char*)body.data, "\x01", &splitSize);
    free(body.data);

    sqlite3_open("database/pembukuan.db", &db[0]);
    sqlite3_open("database/pengeluaran.db", &db[1]);

    for (int a = 0; a < splitSize; a++) {
        sprintf(tempString, "SELECT sum(jumlah),sum(modal),sum(jual),sum(jual - modal) from barangTerjual_%s", stringSplit[a]);
        sqlite3_prepare_v2(db[0], tempString, -1, &statement, NULL);
        
        if (!statement_get_row(statement, &rows, 0)) string_add_format(&resultMessage, "Tanggal %s\nTotal Jumlah Barang: 0\nTotal Harga Modal: Rp0\nTotal Harga Jual: Rp0\nTotal Keuntungan: Rp0\n", stringSplit[a]);
        else {
            char** valueSplit = strsplit(rows.rows, "\x02", 0);

            tempInt = atoi(valueSplit[2]);
            formatCurrency1 = formatCurrency(atoi(valueSplit[0]));
            formatCurrency2 = formatCurrency(atoi(valueSplit[1]));
            formatCurrency3 = formatCurrency(tempInt);
            formatCurrency4 = formatCurrency(atoi(valueSplit[3]));

            string_add_format(&resultMessage,
            "Tanggal %s\nTotal Jumlah Barang: %s\nTotal Harga Modal: Rp%s\nTotal Harga Jual: Rp%s\nTotal Keuntungan: Rp%s\n",
            stringSplit[a],
            formatCurrency1,
            formatCurrency2,
            formatCurrency3,
            formatCurrency4);

            free(formatCurrency1);
            free(formatCurrency2);
            free(formatCurrency4);
            free(valueSplit);
        }

        sqlite3_finalize(statement);
        freeRowBack(&rows);

        sprintf(tempString, "SELECT sum(uang) from pengeluaran_%s", stringSplit[a]);
        sqlite3_prepare_v2(db[1], tempString, -1, &statement, NULL);

        if (!statement_get_row(statement, &rows, 0)) {
            if (!tempInt) string_add_format(&resultMessage, "Total Pengeluaran: Rp0\nTotal Uang Yang Didapat: Rp0\n\n");
            else string_add_format(&resultMessage, "Total Pengeluaran: Rp0\nTotal Uang Yang Didapat: Rp%s\n\n", formatCurrency3);
        } else {
            formatCurrency1 = formatCurrency(atoi(rows.rows));
            formatCurrency2 = formatCurrency(tempInt - atoi(rows.rows));
            string_add_format(&resultMessage, "Total Pengeluaran: Rp%s\nTotal Uang Yang Didapat: Rp%s\n\n", formatCurrency1, formatCurrency2);
            free(formatCurrency1);
            free(formatCurrency2);
            free(formatCurrency3);
        }

        sqlite3_finalize(statement);
        freeRowBack(&rows);
        tempInt = 0;
    }

    sqlite3_close(db[0]);
    sqlite3_close(db[1]);

    int retVal = sendMessage(resultMessage.val, NULL, NULL);
    if (retVal == 0) sb_send_status(e->stream, 403, " bisa terkoneksi ke telegram, apakah internet anda menyala?");
    else if (retVal == -1) sb_send_status(e->stream, 403, "Token Bot dan User ID yang kamu masukan itu salah! mohon masukan Token Bot dan User ID yang benar");
    else sb_send_status(e->stream, 200, "OK");
    sb_send_header(e->stream, "Access-Control-Allow-Origin", "*");
    string_finalize(&resultMessage);
    free(stringSplit);
    return SB_RES_OK;
}
