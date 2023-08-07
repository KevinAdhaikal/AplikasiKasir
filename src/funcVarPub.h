// public struct

typedef struct {
    char usingTelegramBot;
    char* tokenBot;
    char** userID;
    size_t userIDsize;
} telegramBot;

extern telegramBot teleBot;

// public function
void loadSettings();