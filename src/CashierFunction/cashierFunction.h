#include <stdio.h>

#include "../../vendor/sandbird/sandbird.h"

int tambahBarang(sb_Event* e);
int daftarBarang(sb_Event* e);
int hapusDaftarBarang(sb_Event* e);
int infoBarang(sb_Event* e);
int editBarang(sb_Event* e);
int cashierFindBarang(sb_Event* e);
int cashierSettings(sb_Event* e);
int cashierStockChecker(sb_Event* e);
int pembukuan(sb_Event* e);
int dashboardLogic(sb_Event* e);
int barangMasukLogic(sb_Event* e);
int pengaturan(sb_Event* e);
int dbBackup(sb_Event* e);
int dbImport(sb_Event* e);
int userLogic(sb_Event* e);
int roleLogic(sb_Event* e);