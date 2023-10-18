// public struct

typedef struct {
    char usingTelegramBot;
    char* tokenBot;
    char** userID;
    char notifyBarangKosongTGram;
    char notifyKasirTGram;
    char isNotifyBarangDibawahJumlah;
    unsigned short targetNotifyBarangDibawahJumlah;
    size_t userIDsize;
} telegramBot; // telegram struct

// semua extern
extern telegramBot teleBot;

// public function
void loadSettings();