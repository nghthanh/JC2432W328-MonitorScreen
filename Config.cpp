#include "Config.h"
#include <WiFi.h>
#include <time.h>

Config::Config() {
    // Default values
    commInterface = COMM_WIFI;
    wifiSSID = "";
    wifiPassword = "";
    bleName = "ESP32_Monitor";
    displayTheme = THEME_DEFAULT;
    brightness = 128;
    serverPort = 8080;

    alertThresholds.cpuTempHigh = 80.0;
    alertThresholds.memoryLow = 20.0;
    alertThresholds.diskLow = 10.0;

    idleTimeout = 30;  // Default 30 seconds

    // Default date/time (2025-01-01 00:00:00)
    dateYear = 2025;
    dateMonth = 1;
    dateDay = 1;
    timeHour = 0;
    timeMinute = 0;
    timeSecond = 0;
    lastTimeUpdate = 0;

    // Default NTP settings
    ntpServer = "pool.ntp.org";
    gmtOffset = 0;  // UTC
    daylightOffset = 0;
}

Config& Config::getInstance() {
    static Config instance;
    return instance;
}

void Config::begin() {
    prefs.begin("monitor", false);
    loadSettings();
}

void Config::loadSettings() {
    commInterface = (CommInterfaceType)prefs.getUChar("commIf", COMM_WIFI);
    wifiSSID = prefs.getString("wifiSSID", "");
    wifiPassword = prefs.getString("wifiPass", "");
    bleName = prefs.getString("bleName", "ESP32_Monitor");
    displayTheme = (DisplayTheme)prefs.getUChar("theme", THEME_DEFAULT);
    brightness = prefs.getUChar("brightness", 128);
    serverPort = prefs.getUShort("port", 8080);

    alertThresholds.cpuTempHigh = prefs.getFloat("alertCPU", 80.0);
    alertThresholds.memoryLow = prefs.getFloat("alertMem", 20.0);
    alertThresholds.diskLow = prefs.getFloat("alertDisk", 10.0);

    idleTimeout = prefs.getUShort("idleTimeout", 30);

    // Load date/time settings
    dateYear = prefs.getInt("dtYear", 2025);
    dateMonth = prefs.getInt("dtMonth", 1);
    dateDay = prefs.getInt("dtDay", 1);
    timeHour = prefs.getInt("dtHour", 0);
    timeMinute = prefs.getInt("dtMinute", 0);
    timeSecond = prefs.getInt("dtSecond", 0);
    lastTimeUpdate = millis();

    // Load NTP settings
    ntpServer = prefs.getString("ntpServer", "pool.ntp.org");
    gmtOffset = prefs.getLong("gmtOffset", 0);
    daylightOffset = prefs.getInt("dstOffset", 0);
}

void Config::saveSettings() {
    prefs.putUChar("commIf", (uint8_t)commInterface);
    prefs.putString("wifiSSID", wifiSSID);
    prefs.putString("wifiPass", wifiPassword);
    prefs.putString("bleName", bleName);
    prefs.putUChar("theme", (uint8_t)displayTheme);
    prefs.putUChar("brightness", brightness);
    prefs.putUShort("port", serverPort);

    prefs.putFloat("alertCPU", alertThresholds.cpuTempHigh);
    prefs.putFloat("alertMem", alertThresholds.memoryLow);
    prefs.putFloat("alertDisk", alertThresholds.diskLow);

    prefs.putUShort("idleTimeout", idleTimeout);

    // Save date/time settings
    prefs.putInt("dtYear", dateYear);
    prefs.putInt("dtMonth", dateMonth);
    prefs.putInt("dtDay", dateDay);
    prefs.putInt("dtHour", timeHour);
    prefs.putInt("dtMinute", timeMinute);
    prefs.putInt("dtSecond", timeSecond);

    // Save NTP settings
    prefs.putString("ntpServer", ntpServer);
    prefs.putLong("gmtOffset", gmtOffset);
    prefs.putInt("dstOffset", daylightOffset);
}

void Config::reset() {
    prefs.clear();
    *this = Config();
    saveSettings();
}

void Config::setCommInterface(CommInterfaceType interface) {
    commInterface = interface;
    prefs.putUChar("commIf", (uint8_t)interface);
}

CommInterfaceType Config::getCommInterface() {
    return commInterface;
}

void Config::setWiFiSSID(const char* ssid) {
    wifiSSID = ssid;
    prefs.putString("wifiSSID", wifiSSID);
}

void Config::setWiFiPassword(const char* password) {
    wifiPassword = password;
    prefs.putString("wifiPass", wifiPassword);
}

String Config::getWiFiSSID() {
    return wifiSSID;
}

String Config::getWiFiPassword() {
    return wifiPassword;
}

void Config::setBLEName(const char* name) {
    bleName = name;
    prefs.putString("bleName", bleName);
}

String Config::getBLEName() {
    return bleName;
}

void Config::setDisplayTheme(DisplayTheme theme) {
    displayTheme = theme;
    prefs.putUChar("theme", (uint8_t)theme);
}

DisplayTheme Config::getDisplayTheme() {
    return displayTheme;
}

void Config::setBrightness(uint8_t bright) {
    brightness = bright;
    prefs.putUChar("brightness", bright);
}

uint8_t Config::getBrightness() {
    return brightness;
}

void Config::setAlertThresholds(AlertThresholds thresholds) {
    alertThresholds = thresholds;
    prefs.putFloat("alertCPU", thresholds.cpuTempHigh);
    prefs.putFloat("alertMem", thresholds.memoryLow);
    prefs.putFloat("alertDisk", thresholds.diskLow);
}

AlertThresholds Config::getAlertThresholds() {
    return alertThresholds;
}

void Config::setServerPort(uint16_t port) {
    serverPort = port;
    prefs.putUShort("port", port);
}

uint16_t Config::getServerPort() {
    return serverPort;
}

void Config::setDateTime(int year, int month, int day, int hour, int minute, int second) {
    dateYear = year;
    dateMonth = month;
    dateDay = day;
    timeHour = hour;
    timeMinute = minute;
    timeSecond = second;
    lastTimeUpdate = millis();
    saveSettings();
}

void Config::getDateTime(int& year, int& month, int& day, int& hour, int& minute, int& second) {
    updateTime();
    year = dateYear;
    month = dateMonth;
    day = dateDay;
    hour = timeHour;
    minute = timeMinute;
    second = timeSecond;
}

String Config::getFormattedDate() {
    updateTime();
    char buf[16];
    sprintf(buf, "%04d-%02d-%02d", dateYear, dateMonth, dateDay);
    return String(buf);
}

String Config::getFormattedTime() {
    updateTime();
    char buf[16];
    sprintf(buf, "%02d:%02d", timeHour, timeMinute);
    return String(buf);
}

String Config::getFormattedDateTime() {
    updateTime();
    char buf[32];
    sprintf(buf, "%04d-%02d-%02d %02d:%02d", dateYear, dateMonth, dateDay, timeHour, timeMinute);
    return String(buf);
}

void Config::updateTime() {
    unsigned long currentMillis = millis();
    unsigned long elapsed = currentMillis - lastTimeUpdate;

    if (elapsed >= 1000) {
        int secondsToAdd = elapsed / 1000;
        lastTimeUpdate = currentMillis - (elapsed % 1000);

        timeSecond += secondsToAdd;

        while (timeSecond >= 60) {
            timeSecond -= 60;
            timeMinute++;

            if (timeMinute >= 60) {
                timeMinute = 0;
                timeHour++;

                if (timeHour >= 24) {
                    timeHour = 0;
                    dateDay++;

                    // Simple day overflow handling (doesn't account for months/years perfectly)
                    int daysInMonth = 31;
                    if (dateMonth == 2) {
                        daysInMonth = (dateYear % 4 == 0 && (dateYear % 100 != 0 || dateYear % 400 == 0)) ? 29 : 28;
                    } else if (dateMonth == 4 || dateMonth == 6 || dateMonth == 9 || dateMonth == 11) {
                        daysInMonth = 30;
                    }

                    if (dateDay > daysInMonth) {
                        dateDay = 1;
                        dateMonth++;

                        if (dateMonth > 12) {
                            dateMonth = 1;
                            dateYear++;
                        }
                    }
                }
            }
        }
    }
}

bool Config::syncTimeWithNTP(const char* server, long gmtOff, int dstOff) {
    // Check if WiFi is connected
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected. Cannot sync time.");
        return false;
    }

    // Configure time with NTP
    configTime(gmtOff, dstOff, server);

    Serial.printf("Syncing time with NTP server: %s\n", server);
    Serial.printf("GMT Offset: %ld seconds, DST Offset: %d seconds\n", gmtOff, dstOff);

    // Wait for time to be set (timeout after 10 seconds)
    int timeout = 0;
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo) && timeout < 100) {
        delay(100);
        timeout++;
    }

    if (timeout >= 100) {
        Serial.println("Failed to sync time with NTP server");
        return false;
    }

    // Update internal time from NTP
    dateYear = timeinfo.tm_year + 1900;
    dateMonth = timeinfo.tm_mon + 1;
    dateDay = timeinfo.tm_mday;
    timeHour = timeinfo.tm_hour;
    timeMinute = timeinfo.tm_min;
    timeSecond = timeinfo.tm_sec;
    lastTimeUpdate = millis();

    saveSettings();

    Serial.printf("Time synced successfully: %04d-%02d-%02d %02d:%02d:%02d\n",
                  dateYear, dateMonth, dateDay, timeHour, timeMinute, timeSecond);

    return true;
}

void Config::setNTPServer(const char* server) {
    ntpServer = server;
    prefs.putString("ntpServer", ntpServer);
}

String Config::getNTPServer() {
    return ntpServer;
}

void Config::setGMTOffset(long offset) {
    gmtOffset = offset;
    prefs.putLong("gmtOffset", gmtOffset);
}

long Config::getGMTOffset() {
    return gmtOffset;
}

void Config::setDaylightOffset(int offset) {
    daylightOffset = offset;
    prefs.putInt("dstOffset", daylightOffset);
}

int Config::getDaylightOffset() {
    return daylightOffset;
}

void Config::setIdleTimeout(uint16_t seconds) {
    idleTimeout = seconds;
    prefs.putUShort("idleTimeout", idleTimeout);
}

uint16_t Config::getIdleTimeout() {
    return idleTimeout;
}
