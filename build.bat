@echo off
gcc src/*.c vendor/sqlite3/sqlite3.c src/utils/*.c vendor/sandbird/*.c src/methodFunction/*.c src/CashierFunction/*.c vendor/cJSON/cJSON.c vendor/tlse/tlse.c src/telegramClient/telegramClient.c vendor/str/str.c src/teleFunction/*.c src/alarmFunction/*.c -lws2_32 -lwinmm -lpthread -o AplikasiKasir
cls
AplikasiKasir