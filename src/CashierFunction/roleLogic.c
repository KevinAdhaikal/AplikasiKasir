#define _GNU_SOURCE // Disable implicit declaration warning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "../../vendor/sandbird/sandbird.h"
#include "../../vendor/sqlite3/sqlite3.h"
#include "../utils/utils.h"
#include "../sqliteFunction.h"

#include "cashierFunction.h"

static char isStr_str_small(const char* str, const char* toFind, char isEndLine) {
    int string_len = strlen(toFind);
    if (strlen(str) > string_len) return 0;

    for (int a = 0; a < string_len; a++) if (tolower(str[a]) != toFind[a]) return 0;
    if (str[string_len] != '\0' && isEndLine) return 0;
    return 1;
}

int roleLogic(sb_Event* e) {
    sqlite3* db;
    sqlite3_stmt* statement;
    switch(sb_convert_var_to_int(e->stream, "role_opt")) {
        case 1: {
            SQLRow rows = {0};
            switch(sb_convert_var_to_int(e->stream, "role_view_opt")) {
                case 1: {
                    sqlite3_open("database/role.db", &db);
                    char role_id[13];
                    sb_get_var(e->stream, "role_id", role_id, 12);
                    sqlite3_prepare_v2(db, "SELECT * from role_table WHERE rowid = ?", -1, &statement, NULL);
                    sqlite3_bind_int(statement, 1, atoi(role_id));
                    break;
                }
                case 2: {
                    // siapa yang memakai role saya?
                    sqlite3_open("database/user.db", &db);
                    char role_id[13];
                    sb_get_var(e->stream, "role_id", role_id, 12);
                    sqlite3_prepare_v2(db, "SELECT username FROM user_table WHERE role_id = ?", -1, &statement, NULL);
                    sqlite3_bind_int(statement, 1, atoi(role_id));
                    break;
                }
                default: {
                    sqlite3_open("database/role.db", &db);
                    sqlite3_prepare_v2(db, "SELECT rowid, name_role, dibuat_waktu, dimodif_waktu, is_super from role_table", -1, &statement, NULL);
                    break;
                }
            }
            statement_get_row(statement, &rows, 1);
            sqlite3_finalize(statement);
            sqlite3_close(db);

            sb_write(e->stream, rows.rows, rows.totalChar);
            freeRowBack(&rows);

            break;
            // list all role / view Role
        }
        case 2: {
            char body_res[256];
            sb_get_body_static(e->stream, body_res, 256);
            char** str_split = strsplit(body_res, "\x01", 0);

            sqlite3_open("database/role.db", &db);
            sqlite3_prepare_v2(db, "SELECT 1 FROM role_table WHERE name_role = lower(?)", -1, &statement, NULL);
            sqlite3_bind_text(statement, 1, str_split[0], -1, SQLITE_STATIC);
            if (sqlite3_step(statement) != SQLITE_ROW) {
                // jika nama role tersebut belum ada
                sqlite3_reset(statement);
                sqlite3_clear_bindings(statement);

                sqlite3_prepare_v2(db, "INSERT INTO role_table (name_role, role_perm) VALUES (?, ?)", -1, &statement, NULL);
                sqlite3_bind_text(statement, 1, str_split[0], -1, SQLITE_STATIC);
                sqlite3_bind_text(statement, 2, str_split[1], -1, SQLITE_STATIC);
                sqlite3_step(statement);

                sb_send_status(e->stream, 200, "OK");
            } else sb_send_status(e->stream, 403, "Nama Role Permission tersebut sudah ada!"); // jika nama role tersebut sudah ada
            
            sqlite3_finalize(statement);
            sqlite3_close(db);
            free(str_split);

            break;
            // Add role
        }
        case 3: {
            char id_role[13];
            sb_get_var(e->stream, "role_id", id_role, 12);
            sqlite3_open("database/role.db", &db);

            // kita cek dulu, apakah ini super role atau tidak
            sqlite3_prepare_v2(db, "SELECT 1 FROM role_table WHERE rowid = ? AND is_super = 1", -1, &statement, NULL);
            sqlite3_bind_int(statement, 1, atoi(id_role));
            
            if (sqlite3_step(statement) != SQLITE_ROW) {
                // jika bukan super role
                sqlite3_reset(statement);
                sqlite3_clear_bindings(statement);
                sqlite3_prepare_v2(db, "DELETE FROM role_table WHERE rowid = ?", -1, &statement, NULL);
                sqlite3_bind_int(statement, 1, atoi(id_role));
                sqlite3_step(statement);
            } else sb_send_status(e->stream, 403, "Tidak bisa dihapus karena ini adalah Super Role"); // jika super role

            sqlite3_finalize(statement);
            sqlite3_close(db);

            sb_send_status(e->stream, 200, "OK");
            break;
            // delete role
        }
        case 4: {
            char body_res[512];
            sb_get_body_static(e->stream, body_res, 512);
            char** str_split = strsplit(body_res, "\x01", 0);

            sqlite3_open("database/role.db", &db);

            sqlite3_prepare_v2(db, "SELECT 1 FROM role_table WHERE rowid = ?", -1, &statement, NULL);
            sqlite3_bind_int(statement, 1, atoi(str_split[0]));

            if (sqlite3_step(statement) != SQLITE_ROW) {
                sb_send_status(e->stream, 403, "Role ID tersebut tidak ada");
                goto END_4;
            }

            sqlite3_reset(statement);
            sqlite3_clear_bindings(statement);
            
            sqlite3_prepare_v2(db, "SELECT 1 FROM role_table WHERE name_role = ?", -1, &statement, NULL);
            sqlite3_bind_int(statement, 1, atoi(str_split[1]));

            if (sqlite3_step(statement) == SQLITE_ROW && strcmp((char*)sqlite3_column_text(statement, 0), str_split[1])) {
                sb_send_status(e->stream, 403, "Nama Role tersebut sudah ada!"); // jika nama role tersebut sudah ada
                goto END_4;
            }
            else {
                sqlite3_reset(statement);
                sqlite3_clear_bindings(statement);

                sqlite3_prepare_v2(db, "SELECT 1 FROM role_table WHERE rowid = ? AND is_super = 1", -1, &statement, NULL);
                sqlite3_bind_int(statement, 1, atoi(str_split[0]));
                if (sqlite3_step(statement) != SQLITE_ROW) {
                    // jika bukan super role
                    sqlite3_reset(statement);
                    sqlite3_clear_bindings(statement);
                    sqlite3_prepare_v2(db, "UPDATE role_table SET name_role = ?, role_perm = ?, dimodif_waktu = (strftime('%d-%m-%Y %H:%M:%S', 'now', 'localtime')) WHERE rowid = ?", -1, &statement, NULL);

                    sqlite3_bind_text(statement, 1, str_split[1], -1, SQLITE_STATIC);
                    sqlite3_bind_text(statement, 2, str_split[2], -1, SQLITE_STATIC);
                    sqlite3_bind_int(statement, 3, atoi(str_split[0]));

                    sqlite3_step(statement);
                    sb_send_status(e->stream, 200, "OK");
                } else {
                    sqlite3_reset(statement);
                    sqlite3_clear_bindings(statement);
                    sqlite3_prepare_v2(db, "UPDATE role_table SET name_role = ?, dimodif_waktu = (strftime('%d-%m-%Y %H:%M:%S', 'now', 'localtime')) WHERE rowid = ?", -1, &statement, NULL);

                    sqlite3_bind_text(statement, 1, str_split[1], -1, SQLITE_STATIC);
                    sqlite3_bind_int(statement, 2, atoi(str_split[0]));

                    sqlite3_step(statement);
                    sb_send_status(e->stream, 200, "OK");
                }
            }

            END_4:
            sqlite3_finalize(statement);
            sqlite3_close(db);
            free(str_split);

            break;
            // edit role
        }
        case 5: {
            SQLRow rows = {0};

            sqlite3_open("database/role.db", &db);
            sqlite3_prepare_v2(db , "SELECT rowid,name_role FROM role_table", -1, &statement, NULL);
            statement_get_row(statement, &rows, 1);

            sb_write(e->stream, rows.rows, rows.totalChar);

            freeRowBack(&rows);
            sqlite3_finalize(statement);
            sqlite3_close(db);
            break;
            // list role
        }
    }
    
    return SB_RES_OK;
}