#ifndef DISPLAY_H
#define DISPLAY_H

#include <TFT_eSPI.h>
#include "SystemData.h"
#include "Config.h"

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

// History buffer size for graphs
#define HISTORY_SIZE 60

// Colors
#define COLOR_BG        TFT_BLACK
#define COLOR_TEXT      TFT_WHITE
#define COLOR_LABEL     TFT_CYAN
#define COLOR_CPU       TFT_GREEN
#define COLOR_MEMORY    TFT_YELLOW
#define COLOR_DISK      TFT_ORANGE
#define COLOR_NETWORK   TFT_MAGENTA
#define COLOR_ALERT     TFT_RED

class Display {
public:
    static Display& getInstance();

    void begin();
    void update(const SystemData& data);
    void updateTimeDisplay();  // Update just the time display
    void showIdleScreen();     // Show idle/waiting screen
    void showStatus(const char* message);
    void showAlert(const char* message);
    void showConnectionInfo(const char* info);
    void clear();

private:
    Display();

    TFT_eSPI tft;
    DisplayTheme currentTheme;
    SystemData lastData;

    // History buffers for graphs
    float cpuHistory[HISTORY_SIZE];
    float memHistory[HISTORY_SIZE];
    float diskHistory[HISTORY_SIZE];
    int historyIndex;

    // Alert state
    bool alertActive;
    unsigned long lastAlertTime;

    // Time display state
    String lastTimeDisplayed;
    bool hasData;

    void renderThemeDefault(const SystemData& data);
    void renderThemeMinimal(const SystemData& data);
    void renderThemeGraph(const SystemData& data);
    void renderThemeCompact(const SystemData& data);

    void drawProgressBar(int x, int y, int w, int h, float percent, uint16_t color);
    void drawGraph(int x, int y, int w, int h, float* history, int size, uint16_t color, float maxVal = 100.0);
    void drawLabel(int x, int y, const char* label, const char* value, uint16_t color);
    void updateHistory(const SystemData& data);
    void checkAlerts(const SystemData& data);

    bool needsFullRedraw();
};

#endif
