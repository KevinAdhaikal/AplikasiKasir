// public struct

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

typedef struct {
    char isAlarmPembukuanRun;
} tele_thread_info;

typedef struct {
    char usingTelegramBot;
    char* tokenBot;
    char** userID;
    char notifyBarangKosongTGram;
    char notifyKasirTGram;
    char isNotifyBarangDibawahJumlah;
    unsigned short targetNotifyBarangDibawahJumlah;
    char isNotifyAlarmPembukuan;
    size_t userIDsize;
} telegramBot; // telegram struct

// semua extern
extern telegramBot teleBot;
extern tele_thread_info teleThreadInfo;
#ifdef _WIN32
extern HANDLE tele_thread[1];
#else
extern pthread_t tele_thread[1];
#endif

// public function
void prepare_db(char check_db_ver);
void loadSettings();