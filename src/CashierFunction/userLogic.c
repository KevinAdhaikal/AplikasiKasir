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

int userLogic(sb_Event* e) {
    sqlite3* db;
    sqlite3_stmt* statement;
    switch(sb_convert_var_to_int(e->stream, "user_opt")) {
        case 1: {
            SQLRow rows = {0};
            sqlite3_open("database/user.db", &db);
            if (sb_convert_var_to_int(e->stream, "is_view_user") == 1) {
                char user_id[13];
                sb_get_var(e->stream, "user_id", user_id, 12);
                sqlite3_prepare_v2(db, "SELECT username, password, role_id, is_super from user_table WHERE rowid = ?", -1, &statement, NULL);
                sqlite3_bind_int(statement, 1, atoi(user_id));
            } else sqlite3_prepare_v2(db, "SELECT rowid, username, dibuat_waktu, dimodif_waktu, is_super from user_table", -1, &statement, NULL);
            
            statement_get_row(statement, &rows, 1);
            sqlite3_finalize(statement);
            sqlite3_close(db);

            sb_write(e->stream, rows.rows, rows.totalChar);
            freeRowBack(&rows);
            
            break;
            // list all user / view user
        }
        case 2: {
            char username[65], password[65], role_id[13];
            sb_get_var(e->stream, "username", username, 64);
            sb_get_var(e->stream, "password", password, 64);
            sb_get_var(e->stream, "role_id", role_id, 12);

            if (!username[0]) {
                sb_send_status(e->stream, 403, "Username tidak boleh kosong!");
                return SB_RES_OK;
            }
            if (!password[0]) {
                sb_send_status(e->stream, 403, "Password tidak boleh kosong!");
                return SB_RES_OK;
            }
            if (!role_id[0]) {
                sb_send_status(e->stream, 403, "Role ID tidak boleh kosong!");
                return SB_RES_OK;
            }

            sqlite3_open("database/user.db", &db);

            sqlite3_prepare_v2(db, "SELECT 1 FROM user_table WHERE username = lower(?)", -1, &statement, NULL);
            sqlite3_bind_text(statement, 1, username, -1, SQLITE_STATIC);

            if (sqlite3_step(statement) == SQLITE_ROW) {
                sb_send_status(e->stream, 403, "Username tersebut sudah ada");
                goto EXIT_2;
            }

            sqlite3_clear_bindings(statement);
            sqlite3_reset(statement);
            sqlite3_prepare_v2(db, "INSERT INTO user_table (username, password, role_id) VALUES (?, ?, ?)", -1, &statement, NULL);
            
            sqlite3_bind_text(statement, 1, username, -1, SQLITE_STATIC);
            sqlite3_bind_text(statement, 2, password, -1, SQLITE_STATIC);
            sqlite3_bind_text(statement, 3, role_id, -1, SQLITE_STATIC);

            if (sqlite3_step(statement) != SQLITE_DONE) sb_send_status(e->stream, 403, "Terjadi kesalahan membuat User: User tersebut sudah ada!");
            else sb_send_status(e->stream, 200, "OK");

            EXIT_2:
            sqlite3_finalize(statement);
            sqlite3_close(db);
            
            break;
            // create user
        }
        case 3: {
            char user_id[13], username[65], password[65], role_id[13];
            // TODO: using body

            sb_get_var(e->stream, "user_id", user_id, 12);
            sb_get_var(e->stream, "username", username, 64);
            sb_get_var(e->stream, "password", password, 64);
            sb_get_var(e->stream, "role_id", role_id, 12);

            if (!user_id[0]) {
                sb_send_status(e->stream, 403, "User ID Tidak boleh kosong!");
                return SB_RES_OK;
            }
            if (!username[0]) {
                sb_send_status(e->stream, 403, "Username tidak boleh kosong!");
                return SB_RES_OK;
            }
            if (!password[0]) {
                sb_send_status(e->stream, 403, "Password tidak boleh kosong!");
                return SB_RES_OK;
            }
            if (!role_id[0]) {
                sb_send_status(e->stream, 403, "Role ID tidak boleh kosong!");
                return SB_RES_OK;
            }

            sqlite3_open("database/user.db", &db);
            sqlite3_prepare_v2(db, "SELECT 1 FROM user_table WHERE username = lower(username)", -1, &statement, NULL);
            sqlite3_bind_text(statement, 1, username, -1, SQLITE_STATIC);
            if (sqlite3_step(statement) == SQLITE_ROW) {
                sqlite3_clear_bindings(statement);
                sqlite3_reset(statement);

                sqlite3_prepare_v2(db, "SELECT lower(username) FROM user_table WHERE rowid = ?", -1, &statement, NULL);
                sqlite3_bind_int(statement, 1, atoi(user_id));
                if (sqlite3_step(statement) == SQLITE_ROW && !isStr_str_small(username, (char*)sqlite3_column_text(statement, 0), 1)) {
                    sb_send_status(e->stream, 403, "Username tersebut sudah ada!");
                    goto EXIT_3;
                }
            }

            sqlite3_clear_bindings(statement);
            sqlite3_reset(statement);
            
            sqlite3_prepare_v2(db, "UPDATE user_table SET username = ?, password = ?, role_id = ? WHERE rowid = ?", -1, &statement, NULL);
            
            sqlite3_bind_text(statement, 1, username, -1, SQLITE_STATIC);
            sqlite3_bind_text(statement, 2, password, -1, SQLITE_STATIC);
            sqlite3_bind_int(statement, 3, atoi(role_id));
            sqlite3_bind_int(statement, 4, atoi(user_id));
            sqlite3_step(statement);

            sb_send_status(e->stream, 200, "OK");

            EXIT_3:
            sqlite3_finalize(statement);
            sqlite3_close(db);

            break;
            // edit user
        }
        case 4: {
            char user_id[13];
            sb_get_var(e->stream, "user_id", user_id, 12);
            
            sqlite3_open("database/user.db", &db);
            sqlite3_prepare_v2(db, "DELETE FROM user_table WHERE rowid = ?", -1, &statement, NULL);
            sqlite3_bind_int(statement, 1, atoi(user_id));
            sqlite3_step(statement);

            sqlite3_finalize(statement);
            sqlite3_close(db);

            sb_send_status(e->stream, 200, "OK");
            break;
            // remove user
        }
        case 5: {
            char username[65], current_password[65], new_password[65];

            sb_get_var(e->stream, "username", username, 64);
            sb_get_var(e->stream, "current_password", current_password, 64);
            sb_get_var(e->stream, "new_password", new_password, 64);

            sqlite3_open("database/user.db", &db);
            sqlite3_prepare_v2(db, "SELECT 1 FROM user_table WHERE username = ? AND password = ?", -1, &statement, NULL);
            sqlite3_bind_text(statement, 1, username, -1, SQLITE_STATIC);
            sqlite3_bind_text(statement, 2, current_password, -1, SQLITE_STATIC);

            if (sqlite3_step(statement) == SQLITE_ROW) {
                sqlite3_reset(statement);
                sqlite3_clear_bindings(statement);

                sqlite3_prepare_v2(db, "UPDATE user_table SET password = ? WHERE username = ?", -1, &statement, NULL);
                sqlite3_bind_text(statement, 1, new_password, -1, SQLITE_STATIC);
                sqlite3_bind_text(statement, 2, username, -1, SQLITE_STATIC);
                sqlite3_step(statement);
            } else sb_send_status(e->stream, 403, "Password sebelumnya tidak benar!");

            sqlite3_finalize(statement);
            sqlite3_close(db);

            sb_send_status(e->stream, 200, "OK");
            break;
            // change password
        }
    }
    return SB_RES_OK;
}