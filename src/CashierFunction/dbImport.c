#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../vendor/sqlite3/sqlite3.h"
#include "../../vendor/sandbird/sandbird.h"
#include "../../vendor/miniz/miniz.h"
#include "../utils/utils.h"
#include "../funcVarPub.h"

#include "cashierFunction.h"

int dbImport(sb_Event* e) {
    sb_Body body_data;
    sb_get_body(e->stream, &body_data);

    int mem_pos = 5;

    if (!body_data.sizeData || !isStr((char*)body_data.data, "KSRDB", 0)) {
        sb_send_status(e->stream, 403, "Invalid Database");
        return SB_RES_OK;
    }   

    char* file_name[] = {"database/barangMasuk.db",
                         "database/daftarBarang.db",
                         "database/pembukuan.db",
                         "database/pengeluaran.db",
                         "database/settings.db"};
    short file_count;
    int compress_size;
    int real_size;

    memcpy(&file_count, body_data.data + mem_pos, 2);
    mem_pos += 2;

    for (short a = 0; a < file_count; a++) {
        FILE* fp = fopen(file_name[a], "wb");

        memcpy(&compress_size, body_data.data + mem_pos, 4);
        mem_pos += 4;
        
        memcpy(&real_size, body_data.data + mem_pos, 4);
        mem_pos += 4;
        
        unsigned char temp_data[real_size];
        unsigned char compress_data[compress_size];

        memcpy(compress_data, body_data.data + mem_pos, compress_size);
        mem_pos += compress_size;

        mz_uncompress(temp_data, (mz_ulong*)&real_size, compress_data, (mz_ulong)compress_size);

        fwrite(temp_data, real_size, 1, fp);
        fclose(fp);
    }

    sb_send_status(e->stream, 200, "OK");
    free(body_data.data);
    prepare_db(0);

    return SB_RES_OK;
}