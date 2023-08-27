#include <stdio.h>
#include <stdlib.h>

#include "../../vendor/httplib/httplibrary.h"
#include "../utils/utils.h"
#include "../teleFunction/teleFunction.h"
#include "../telegramClient/telegramClient.h"
#include "../../vendor/str/str.h"

#include "teleFunction.h"

void teleKasir(http_event* e) {
    char* formatCurrency0;
    char* formatCurrency1;
    char* formatCurrency2;
    char* formatCurrency3;
    int tempInt[3] = {0, 0, 0};
    Str totalString;
    
    str_init(&totalString);
    str_append_format(&totalString, "Kasir (Barang Keluar)\n\n");

    size_t sizeSplit = 0;
    char** stringSplit = strsplit(e->headers.raw_header + e->headers.body_pos, "\n", &sizeSplit);

    for (int a = 0; a < sizeSplit; a++) {
        char** valueSplit = strsplit(stringSplit[a], "|", 0);
        
        tempInt[0] += atoi(valueSplit[1]);
        tempInt[1] += atoi(valueSplit[2]);
        tempInt[2] += atoi(valueSplit[3]);

        formatCurrency1 = formatCurrency(atoi(valueSplit[1]));
        formatCurrency2 = formatCurrency(atoi(valueSplit[2]));
        formatCurrency3 = formatCurrency(atoi(valueSplit[3]));

        str_append_format(&totalString, "Nama Barang: %s\nJumlah Barang: %s\nHarga Modal: %s\nHarga Jual: %s\n\n", valueSplit[0], formatCurrency1, formatCurrency2, formatCurrency3);
        
        free(formatCurrency1);
        free(formatCurrency2);
        free(formatCurrency3);

        free(valueSplit);
    }

    formatCurrency0 = formatCurrency(sizeSplit);
    formatCurrency1 = formatCurrency(tempInt[0]);
    formatCurrency2 = formatCurrency(tempInt[1]);
    formatCurrency3 = formatCurrency(tempInt[2]);

    str_append_format(&totalString, "Total Barang Dibeli: %s\nTotal Jumlah Barang: %s\nTotal Harga Modal: %s\nTotal Harga Jual: %s\n\n", formatCurrency0, formatCurrency1, formatCurrency2, formatCurrency3);
    
    free(stringSplit);
    free(formatCurrency0);
    free(formatCurrency1);
    free(formatCurrency2);
    free(formatCurrency3);

    sendMessage(totalString.value);
    str_finalize(&totalString);

    http_send_header(e, "Access-Control-Allow-Origin", "*");
    http_send_status(e, 200, "OK");
    
    return;
}