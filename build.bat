@echo off
gcc src/*.c vendor/sqlite3/*.c src/utils/*.c vendor/httplib/*.c src/methodFunction/*.c src/CashierFunction/*.c vendor/cJSON/cJSON.c vendor/tlse/tlse.c src/telegramClient/telegramClient.c vendor/str/str.c src/teleFunction/*.c -lws2_32 -lwinmm -lpthread -o AplikasiKasir
