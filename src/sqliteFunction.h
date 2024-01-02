#include <stdio.h>

#include "../vendor/sqlite3/sqlite3.h"
#include "../vendor/sandbird/sandbird.h"

typedef struct {
    char* rows;
    int totalChar;
} SQLRow;

int tableExists(void *data, int argc, char **argv, char **azColName);
void freeRowBack(SQLRow* data);
int RowBack(void* data, int argc, char** argv, char** colName);
int RowBackPlus(void* data, int argc, char** argv, char** colName);
int sqlReturnInt(void* data, int argc, char** argv, char** colName);
int sqlNormalExec(sb_Event* e, sqlite3* db, char* tempString);
int sqlBackExec(sb_Event* e, sqlite3* db, char* tempString, int (*callback)(void *, int, char **, char **), void* var);
int sqlTOF(void* data, int argc, char** argv, char** colName);
void statement_get_row(sqlite3_stmt* statement, SQLRow* row, char no_lf); // thanks ChatGPT
int statement_row_plus(sqlite3_stmt* statement);