#include "Display.h"
#include <SPI.h>

Display::Display() : historyIndex(0), alertActive(false), lastAlertTime(0), hasData(false) {
    currentTheme = THEME_DEFAULT;
    memset(cpuHistory, 0, sizeof(cpuHistory));
    memset(memHistory, 0, sizeof(memHistory));
    memset(diskHistory, 0, sizeof(diskHistory));
    lastTimeDisplayed = "";
}

Display& Display::getInstance() {
    static Display instance;
    return instance;
}

void Display::begin() {
    // Initialize SPI
    SPI.begin();

    // Initialize TFT
    tft.init();

    // Set backlight on (if defined in User_Setup.h)
    #ifdef TFT_BL
        pinMode(TFT_BL, OUTPUT);
        digitalWrite(TFT_BL, HIGH);  // Turn backlight on
    #endif

    tft.setRotation(0);  // Portrait mode
    tft.fillScreen(COLOR_BG);

    currentTheme = Config::getInstance().getDisplayTheme();

    tft.setTextColor(COLOR_TEXT, COLOR_BG);
    tft.setTextSize(2);
    tft.setCursor(20, 140);
    tft.println("System Monitor");
    tft.setTextSize(1);
    tft.setCursor(40, 170);
    tft.println("Waiting for data...");

    // Display date/time
    tft.setTextColor(COLOR_LABEL, COLOR_BG);
    tft.setTextSize(1);
    String dateStr = Config::getInstance().getFormattedDate();
    String timeStr = Config::getInstance().getFormattedTime();
    int16_t w = tft.textWidth(dateStr.c_str());
    int x = (SCREEN_WIDTH - w) / 2;
    tft.setCursor(x, 200);
    tft.println(dateStr);
    w = tft.textWidth(timeStr.c_str());
    x = (SCREEN_WIDTH - w) / 2;
    tft.setCursor(x, 215);
    tft.println(timeStr);

    Serial.println("Display initialized");
}

void Display::update(const SystemData& data) {
    hasData = true;

    // Check if theme changed
    DisplayTheme theme = Config::getInstance().getDisplayTheme();
    if (theme != currentTheme || needsFullRedraw()) {
        currentTheme = theme;
        tft.fillScreen(COLOR_BG);
    }

    // Update history
    updateHistory(data);

    // Check for alerts
    checkAlerts(data);

    // Render based on current theme
    switch (currentTheme) {
        case THEME_MINIMAL:
            renderThemeMinimal(data);
            break;
        case THEME_GRAPH:
            renderThemeGraph(data);
            break;
        case THEME_COMPACT:
            renderThemeCompact(data);
            break;
        default:
            renderThemeDefault(data);
            break;
    }

    lastData = data;
}

void Display::updateTimeDisplay() {
    String currentTime = Config::getInstance().getFormattedTime();

    // Only update if time has changed
    if (currentTime == lastTimeDisplayed) {
        return;
    }

    lastTimeDisplayed = currentTime;

    tft.setTextSize(1);
    tft.setTextColor(COLOR_LABEL, COLOR_BG);

    if (!hasData) {
        // Start screen - show date and time centered below "Waiting for data..."
        String dateStr = Config::getInstance().getFormattedDate();
        int16_t w = tft.textWidth(dateStr.c_str());
        int x = (SCREEN_WIDTH - w) / 2;
        tft.fillRect(0, 200, SCREEN_WIDTH, 30, COLOR_BG);
        tft.setCursor(x, 200);
        tft.println(dateStr);
        w = tft.textWidth(currentTime.c_str());
        x = (SCREEN_WIDTH - w) / 2;
        tft.setCursor(x, 215);
        tft.println(currentTime);
    } else {
        // Monitor screen - update time in top corner based on theme
        DisplayTheme theme = Config::getInstance().getDisplayTheme();

        switch (theme) {
            case THEME_MINIMAL:
            case THEME_COMPACT: {
                // For minimal and compact, time is shown with date in center
                String dateTimeStr = Config::getInstance().getFormattedDateTime();
                int16_t w = tft.textWidth(dateTimeStr.c_str());
                int x = (SCREEN_WIDTH - w) / 2;
                int y = (theme == THEME_MINIMAL) ? 10 : 5;
                tft.fillRect(0, y, SCREEN_WIDTH, 12, COLOR_BG);
                tft.setCursor(x, y);
                tft.print(dateTimeStr);
                break;
            }
            case THEME_DEFAULT:
            case THEME_GRAPH:
            default: {
                // For default and graph, time is shown in top right
                int16_t w = tft.textWidth(currentTime.c_str());
                tft.fillRect(SCREEN_WIDTH - w - 5, 10, w + 5, 10, COLOR_BG);
                tft.setCursor(SCREEN_WIDTH - w - 5, 10);
                tft.print(currentTime);
                break;
            }
        }
    }
}

void Display::showStatus(const char* message) {
    tft.fillRect(0, 300, SCREEN_WIDTH, 20, COLOR_BG);
    tft.setTextSize(1);
    tft.setTextColor(COLOR_LABEL, COLOR_BG);
    tft.setCursor(5, 305);
    tft.print(message);
}

void Display::showAlert(const char* message) {
    tft.fillRect(0, 0, SCREEN_WIDTH, 30, COLOR_ALERT);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, COLOR_ALERT);
    tft.setCursor(5, 10);
    tft.print("ALERT: ");
    tft.print(message);
}

void Display::showConnectionInfo(const char* info) {
    // Display connection info below the title, but above date/time
    int y = 185;
    tft.fillRect(0, y, SCREEN_WIDTH, 12, COLOR_BG);
    tft.setTextSize(1);
    tft.setTextColor(COLOR_LABEL, COLOR_BG);

    // Center align and display the info
    int16_t w = tft.textWidth(info);
    int x = (SCREEN_WIDTH - w) / 2;

    tft.setCursor(x, y);
    tft.print(info);

    // Redraw date/time below connection info
    updateTimeDisplay();
}

void Display::clear() {
    tft.fillScreen(COLOR_BG);
}

void Display::showIdleScreen() {
    hasData = false;
    tft.fillScreen(COLOR_BG);

    // Show title
    tft.setTextColor(COLOR_TEXT, COLOR_BG);
    tft.setTextSize(2);
    tft.setCursor(20, 140);
    tft.println("System Monitor");
    tft.setTextSize(1);
    tft.setCursor(40, 170);
    tft.println("Waiting for data...");

    // Display date/time
    tft.setTextColor(COLOR_LABEL, COLOR_BG);
    tft.setTextSize(1);
    String dateStr = Config::getInstance().getFormattedDate();
    String timeStr = Config::getInstance().getFormattedTime();
    int16_t w = tft.textWidth(dateStr.c_str());
    int x = (SCREEN_WIDTH - w) / 2;
    tft.setCursor(x, 200);
    tft.println(dateStr);
    w = tft.textWidth(timeStr.c_str());
    x = (SCREEN_WIDTH - w) / 2;
    tft.setCursor(x, 215);
    tft.println(timeStr);
    lastTimeDisplayed = timeStr;

    Serial.println("Display returned to idle screen");
}

bool Display::needsFullRedraw() {
    static unsigned long lastRedraw = 0;
    unsigned long now = millis();
    if (now - lastRedraw > 10000) {  // Full redraw every 10 seconds
        lastRedraw = now;
        return true;
    }
    return false;
}

void Display::renderThemeDefault(const SystemData& data) {
    int y = 10;

    // Date/Time at top
    tft.setTextSize(1);
    tft.setTextColor(COLOR_LABEL, COLOR_BG);
    String timeStr = Config::getInstance().getFormattedTime();
    int16_t w = tft.textWidth(timeStr.c_str());
    tft.fillRect(SCREEN_WIDTH - w - 5, y, w + 5, 10, COLOR_BG);
    tft.setCursor(SCREEN_WIDTH - w - 5, y);
    tft.print(timeStr);
    y += 15;

    // CPU
    tft.setTextSize(1);
    tft.setTextColor(COLOR_LABEL, COLOR_BG);
    tft.setCursor(5, y);
    tft.print("CPU:");
    y += 15;
    drawProgressBar(5, y, SCREEN_WIDTH - 10, 20, data.cpuUsage, COLOR_CPU);
    tft.setTextColor(COLOR_TEXT, COLOR_BG);
    tft.setCursor(10, y + 5);
    char buf[32];
    sprintf(buf, "%.1f%% | %.1fC", data.cpuUsage, data.cpuTemp);
    tft.print(buf);
    y += 30;

    // Memory
    tft.setTextColor(COLOR_LABEL, COLOR_BG);
    tft.setCursor(5, y);
    tft.print("Memory:");
    y += 15;
    drawProgressBar(5, y, SCREEN_WIDTH - 10, 20, data.memoryPercent, COLOR_MEMORY);
    tft.setTextColor(COLOR_TEXT, COLOR_BG);
    tft.setCursor(10, y + 5);
    sprintf(buf, "%.1f/%.1f GB (%.1f%%)", data.memoryUsed, data.memoryTotal, data.memoryPercent);
    tft.print(buf);
    y += 30;

    // Disk
    tft.setTextColor(COLOR_LABEL, COLOR_BG);
    tft.setCursor(5, y);
    tft.print("Disk:");
    y += 15;
    drawProgressBar(5, y, SCREEN_WIDTH - 10, 20, data.diskPercent, COLOR_DISK);
    tft.setTextColor(COLOR_TEXT, COLOR_BG);
    tft.setCursor(10, y + 5);
    sprintf(buf, "%.1f/%.1f GB (%.1f%%)", data.diskUsed, data.diskTotal, data.diskPercent);
    tft.print(buf);
    y += 30;

    // Network
    tft.setTextColor(COLOR_LABEL, COLOR_BG);
    tft.setCursor(5, y);
    tft.print("Network:");
    y += 15;
    tft.setTextColor(COLOR_TEXT, COLOR_BG);
    tft.setCursor(10, y);
    sprintf(buf, "UP: %.2f KB/s", data.networkUpload);
    tft.print(buf);
    y += 15;
    tft.setCursor(10, y);
    sprintf(buf, "DN: %.2f KB/s", data.networkDownload);
    tft.print(buf);
    y += 25;

    // GPU (if available)
    if (data.gpuUsage > 0 || data.gpuTemp > 0) {
        tft.setTextColor(COLOR_LABEL, COLOR_BG);
        tft.setCursor(5, y);
        tft.print("GPU:");
        y += 15;
        tft.setTextColor(COLOR_TEXT, COLOR_BG);
        tft.setCursor(10, y);
        sprintf(buf, "%.1f%% | %.1fC", data.gpuUsage, data.gpuTemp);
        tft.print(buf);
        y += 20;
    }

    // Additional Temperatures (if available)
    if (data.motherboardTemp > 0 || data.diskTemp > 0) {
        tft.setTextColor(COLOR_LABEL, COLOR_BG);
        tft.setCursor(5, y);
        tft.print("Temps:");
        y += 15;
        tft.setTextColor(COLOR_TEXT, COLOR_BG);
        tft.setCursor(10, y);

        if (data.motherboardTemp > 0) {
            sprintf(buf, "MB: %.1fC", data.motherboardTemp);
            tft.print(buf);

            if (data.diskTemp > 0) {
                tft.print(" | ");
            }
        }

        if (data.diskTemp > 0) {
            sprintf(buf, "%s: %.1fC", data.diskName[0] ? data.diskName : "Disk", data.diskTemp);
            tft.print(buf);
        }
    }
}

void Display::renderThemeMinimal(const SystemData& data) {
    int y = 10;

    // Date/Time at top
    tft.setTextSize(1);
    tft.setTextColor(COLOR_LABEL, COLOR_BG);
    String dateTimeStr = Config::getInstance().getFormattedDateTime();
    int16_t w = tft.textWidth(dateTimeStr.c_str());
    int x = (SCREEN_WIDTH - w) / 2;
    tft.fillRect(0, y, SCREEN_WIDTH, 12, COLOR_BG);
    tft.setCursor(x, y);
    tft.print(dateTimeStr);
    y += 30;

    tft.setTextSize(2);

    char buf[32];
    sprintf(buf, "CPU: %.0f%%", data.cpuUsage);
    tft.setTextColor(COLOR_CPU, COLOR_BG);
    tft.fillRect(0, y, SCREEN_WIDTH, 25, COLOR_BG);
    tft.setCursor(20, y);
    tft.print(buf);
    y += 40;

    sprintf(buf, "MEM: %.0f%%", data.memoryPercent);
    tft.setTextColor(COLOR_MEMORY, COLOR_BG);
    tft.fillRect(0, y, SCREEN_WIDTH, 25, COLOR_BG);
    tft.setCursor(20, y);
    tft.print(buf);
    y += 40;

    sprintf(buf, "DISK: %.0f%%", data.diskPercent);
    tft.setTextColor(COLOR_DISK, COLOR_BG);
    tft.fillRect(0, y, SCREEN_WIDTH, 25, COLOR_BG);
    tft.setCursor(20, y);
    tft.print(buf);
    y += 40;

    sprintf(buf, "TEMP: %.0fC", data.cpuTemp);
    tft.setTextColor(COLOR_ALERT, COLOR_BG);
    tft.fillRect(0, y, SCREEN_WIDTH, 25, COLOR_BG);
    tft.setCursor(20, y);
    tft.print(buf);
}

void Display::renderThemeGraph(const SystemData& data) {
    int y = 10;

    // Date/Time at top right
    tft.setTextSize(1);
    tft.setTextColor(COLOR_LABEL, COLOR_BG);
    String timeStr = Config::getInstance().getFormattedTime();
    int16_t w = tft.textWidth(timeStr.c_str());
    tft.fillRect(SCREEN_WIDTH - w - 5, y, w + 5, 10, COLOR_BG);
    tft.setCursor(SCREEN_WIDTH - w - 5, y);
    tft.print(timeStr);

    // CPU Graph
    tft.setTextSize(1);
    tft.setTextColor(COLOR_CPU, COLOR_BG);
    tft.setCursor(5, y);
    char buf[32];
    sprintf(buf, "CPU: %.1f%%", data.cpuUsage);
    tft.print(buf);
    y += 15;
    drawGraph(5, y, SCREEN_WIDTH - 10, 60, cpuHistory, HISTORY_SIZE, COLOR_CPU);
    y += 70;

    // Memory Graph
    tft.setTextColor(COLOR_MEMORY, COLOR_BG);
    tft.setCursor(5, y);
    sprintf(buf, "MEM: %.1f%%", data.memoryPercent);
    tft.print(buf);
    y += 15;
    drawGraph(5, y, SCREEN_WIDTH - 10, 60, memHistory, HISTORY_SIZE, COLOR_MEMORY);
    y += 70;

    // Disk info
    tft.setTextColor(COLOR_DISK, COLOR_BG);
    tft.setCursor(5, y);
    sprintf(buf, "DISK: %.1f%% | NET: U%.1f D%.1f KB/s",
            data.diskPercent, data.networkUpload, data.networkDownload);
    tft.print(buf);
    y += 15;

    // Temperature info
    if (data.gpuTemp > 0 || data.motherboardTemp > 0 || data.diskTemp > 0) {
        tft.setTextColor(COLOR_LABEL, COLOR_BG);
        tft.fillRect(0, y, SCREEN_WIDTH, 12, COLOR_BG);
        tft.setCursor(5, y);

        char tempBuf[64] = "TEMP:";
        if (data.gpuTemp > 0) {
            sprintf(buf, " GPU:%.0fC", data.gpuTemp);
            strcat(tempBuf, buf);
        }
        if (data.motherboardTemp > 0) {
            sprintf(buf, " MB:%.0fC", data.motherboardTemp);
            strcat(tempBuf, buf);
        }
        if (data.diskTemp > 0) {
            sprintf(buf, " %s:%.0fC", data.diskName[0] ? data.diskName : "DSK", data.diskTemp);
            strcat(tempBuf, buf);
        }
        tft.print(tempBuf);
    }
}

void Display::renderThemeCompact(const SystemData& data) {
    tft.setTextSize(1);
    int y = 5;
    char buf[64];

    // Date/Time at top center
    tft.setTextColor(COLOR_LABEL, COLOR_BG);
    String dateTimeStr = Config::getInstance().getFormattedDateTime();
    int16_t w = tft.textWidth(dateTimeStr.c_str());
    int x = (SCREEN_WIDTH - w) / 2;
    tft.fillRect(0, y, SCREEN_WIDTH, 12, COLOR_BG);
    tft.setCursor(x, y);
    tft.print(dateTimeStr);
    y += 15;

    // Line 1: CPU
    sprintf(buf, "CPU:%3.0f%% %4.1fC", data.cpuUsage, data.cpuTemp);
    drawLabel(5, y, "CPU", buf, COLOR_CPU);
    drawProgressBar(110, y, 125, 12, data.cpuUsage, COLOR_CPU);
    y += 20;

    // Line 2: Memory
    sprintf(buf, "MEM:%3.0f%% %.1f/%.1fGB", data.memoryPercent, data.memoryUsed, data.memoryTotal);
    drawLabel(5, y, "MEM", buf, COLOR_MEMORY);
    drawProgressBar(110, y, 125, 12, data.memoryPercent, COLOR_MEMORY);
    y += 20;

    // Line 3: Disk
    sprintf(buf, "DSK:%3.0f%% %.0f/%.0fGB", data.diskPercent, data.diskUsed, data.diskTotal);
    drawLabel(5, y, "DSK", buf, COLOR_DISK);
    drawProgressBar(110, y, 125, 12, data.diskPercent, COLOR_DISK);
    y += 20;

    // Line 4: Network
    sprintf(buf, "NET: U%.1f D%.1f KB/s", data.networkUpload, data.networkDownload);
    tft.setTextColor(COLOR_NETWORK, COLOR_BG);
    tft.fillRect(0, y, SCREEN_WIDTH, 12, COLOR_BG);
    tft.setCursor(5, y);
    tft.print(buf);
    y += 20;

    // Line 5: Additional Temperatures
    if (data.motherboardTemp > 0 || data.diskTemp > 0) {
        tft.fillRect(0, y, SCREEN_WIDTH, 12, COLOR_BG);
        tft.setTextColor(COLOR_LABEL, COLOR_BG);
        tft.setCursor(5, y);

        char tempStr[64] = "";
        bool first = true;

        if (data.motherboardTemp > 0) {
            sprintf(buf, "MB:%.0fC", data.motherboardTemp);
            strcat(tempStr, buf);
            first = false;
        }

        if (data.diskTemp > 0) {
            if (!first) strcat(tempStr, " ");
            sprintf(buf, "%s:%.0fC", data.diskName[0] ? data.diskName : "DSK", data.diskTemp);
            strcat(tempStr, buf);
        }

        tft.print(tempStr);
        y += 20;
    }

    // Small graphs
    y += 10;
    drawGraph(5, y, SCREEN_WIDTH - 10, 80, cpuHistory, HISTORY_SIZE, COLOR_CPU);
    y += 90;
    drawGraph(5, y, SCREEN_WIDTH - 10, 80, memHistory, HISTORY_SIZE, COLOR_MEMORY);
}

void Display::drawProgressBar(int x, int y, int w, int h, float percent, uint16_t color) {
    percent = constrain(percent, 0, 100);
    int fillWidth = (w * percent) / 100;

    tft.drawRect(x, y, w, h, COLOR_TEXT);
    tft.fillRect(x + 1, y + 1, w - 2, h - 2, COLOR_BG);
    tft.fillRect(x + 1, y + 1, fillWidth - 2, h - 2, color);
}

void Display::drawGraph(int x, int y, int w, int h, float* history, int size, uint16_t color, float maxVal) {
    tft.drawRect(x, y, w, h, COLOR_TEXT);
    tft.fillRect(x + 1, y + 1, w - 2, h - 2, COLOR_BG);

    if (size <= 1 || w < 10 || h < 10) return;  // Safety check

    float step = (float)(w - 2) / (float)size;
    for (int i = 1; i < size; i++) {
        // Constrain history values to prevent overflow
        float val1 = constrain(history[i - 1], 0, maxVal);
        float val2 = constrain(history[i], 0, maxVal);

        // Calculate coordinates with float precision first, then convert
        int x1 = x + 1 + (int)((i - 1) * step);
        int x2 = x + 1 + (int)(i * step);

        // Calculate y coordinates safely
        int y1 = y + h - 2 - (int)((val1 * (h - 4)) / maxVal);
        int y2 = y + h - 2 - (int)((val2 * (h - 4)) / maxVal);

        // Final bounds check
        y1 = constrain(y1, y + 1, y + h - 2);
        y2 = constrain(y2, y + 1, y + h - 2);

        tft.drawLine(x1, y1, x2, y2, color);

        // Yield every 10 iterations to prevent watchdog timeout
        if (i % 10 == 0) yield();
    }
}

void Display::drawLabel(int x, int y, const char* label, const char* value, uint16_t color) {
    tft.setTextColor(color, COLOR_BG);
    tft.fillRect(x, y, SCREEN_WIDTH - x - 5, 12, COLOR_BG);
    tft.setCursor(x, y);
    tft.print(value);
}

void Display::updateHistory(const SystemData& data) {
    cpuHistory[historyIndex] = data.cpuUsage;
    memHistory[historyIndex] = data.memoryPercent;
    diskHistory[historyIndex] = data.diskPercent;

    historyIndex = (historyIndex + 1) % HISTORY_SIZE;
}

void Display::checkAlerts(const SystemData& data) {
    AlertThresholds thresh = Config::getInstance().getAlertThresholds();
    bool alert = false;
    char alertMsg[64] = "";

    if (data.cpuTemp >= thresh.cpuTempHigh) {
        sprintf(alertMsg, "High CPU Temp: %.1fC", data.cpuTemp);
        alert = true;
    } else if (data.memoryPercent <= thresh.memoryLow) {
        sprintf(alertMsg, "Low Memory: %.1f%%", data.memoryPercent);
        alert = true;
    } else if (data.diskPercent <= thresh.diskLow) {
        sprintf(alertMsg, "Low Disk: %.1f%%", data.diskPercent);
        alert = true;
    }

    if (alert && (millis() - lastAlertTime > 5000)) {
        showAlert(alertMsg);
        lastAlertTime = millis();
        alertActive = true;
    } else if (!alert && alertActive) {
        // Clear alert
        tft.fillRect(0, 0, SCREEN_WIDTH, 30, COLOR_BG);
        alertActive = false;
    }
}
