#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "../../vendor/miniz/miniz.h"
#include "../../vendor/sandbird/sandbird.h"
#include "../utils/utils.h"

#include "cashierFunction.h"

int dbBackup(sb_Event* e) {
    unsigned char* temp_buffer;
    unsigned char* compress_buffer;
    FILE* fpwb;

    char kasir_db_magic_code[] = {'K', 'S', 'R', 'D', 'B'}; // wm kang

    FILE* fp_hash = fopen("database/dbhash.bin", "rb");

    char* file_name[] = {"database/barangMasuk.db",
                         "database/daftarBarang.db",
                         "database/pembukuan.db",
                         "database/pengeluaran.db",
                         "database/settings.db",
                         "database/user.db",
                         "database/role.db"};

    short total_file = 7;

    if (fp_hash && file_exists("database/backup.ksr")) {
        short current_total_file;
        fread(&current_total_file, 2, 1, fp_hash);

        if (current_total_file == total_file) {
            uint32_t hash_file;
            uint32_t current_hash_file;

            for (short a = 0; a < current_total_file; a++) {
                FILE* fp2 = fopen(file_name[a], "rb");
                fseek(fp2, 0, SEEK_END);
                int file_size = ftell(fp2);
                unsigned char temp_data[file_size];
                fseek(fp2, 0, SEEK_SET);
                fread(temp_data, file_size, 1, fp2);
                fclose(fp2);

                hash_file = SuperFastHash((char*)temp_data, file_size);
                fread(&current_hash_file, 4, 1, fp_hash);

                if (hash_file != current_hash_file) goto COMPRESS_FILE;
            }
            fclose(fp_hash);
            sb_send_file(e->stream, "database/backup.ksr", 0, "application/octet-stream");
            return SB_RES_OK;
        }
    }
    
    if (fp_hash) fclose(fp_hash);
    fp_hash = fopen("database/dbhash.bin", "wb");

    

    COMPRESS_FILE:
    fpwb = fopen("database/backup.ksr", "wb");

    fwrite(kasir_db_magic_code, 5, 1, fpwb);
    fwrite(&total_file, 2, 1, fpwb);

    for (short a = 0; a < total_file; a++) {
        FILE* fp = fopen(file_name[a], "rb");

        fseek(fp, 0, SEEK_END);
        int file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        temp_buffer = malloc(file_size);
        fread(temp_buffer, file_size, 1, fp);
        fclose(fp);

        mz_ulong compressed_size = mz_compressBound((mz_ulong)file_size);
        compress_buffer = (unsigned char*)malloc(compressed_size);

        mz_compress(compress_buffer, &compressed_size, (const unsigned char*)temp_buffer, file_size);

        fwrite(&compressed_size, 4, 1, fpwb);
        fwrite(&file_size, 4, 1, fpwb);
        fwrite(compress_buffer, compressed_size, 1, fpwb);

        uint32_t result_hash = SuperFastHash((char*)temp_buffer, file_size);
        fwrite(&result_hash, 4, 1, fp_hash);

        free(temp_buffer);
        free(compress_buffer);
    }

    fclose(fpwb);
    fclose(fp_hash);

    sb_send_file(e->stream, "database/backup.ksr", 0, "application/octet-stream");

    return 0;
}
