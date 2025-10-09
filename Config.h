#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Preferences.h>

// Communication interface types
enum CommInterfaceType {
    COMM_WIFI = 0,
    COMM_BLE = 1
};

// Display themes
enum DisplayTheme {
    THEME_DEFAULT = 0,
    THEME_MINIMAL = 1,
    THEME_GRAPH = 2,
    THEME_COMPACT = 3
};

// Alert thresholds
struct AlertThresholds {
    float cpuTempHigh;
    float memoryLow;
    float diskLow;
};

class Config {
public:
    static Config& getInstance();

    void begin();
    void reset();

    // Communication settings
    void setCommInterface(CommInterfaceType interface);
    CommInterfaceType getCommInterface();

    // WiFi settings
    void setWiFiSSID(const char* ssid);
    void setWiFiPassword(const char* password);
    String getWiFiSSID();
    String getWiFiPassword();

    // BLE settings
    void setBLEName(const char* name);
    String getBLEName();

    // mDNS settings
    void setMDNSName(const char* name);
    String getMDNSName();

    // Display settings
    void setDisplayTheme(DisplayTheme theme);
    DisplayTheme getDisplayTheme();
    void setBrightness(uint8_t brightness);
    uint8_t getBrightness();

    // Alert settings
    void setAlertThresholds(AlertThresholds thresholds);
    AlertThresholds getAlertThresholds();

    // Server settings
    void setServerPort(uint16_t port);
    uint16_t getServerPort();

    // Idle timeout settings
    void setIdleTimeout(uint16_t seconds);
    uint16_t getIdleTimeout();

    // Date/Time settings
    void setDateTime(int year, int month, int day, int hour, int minute, int second);
    void getDateTime(int& year, int& month, int& day, int& hour, int& minute, int& second);
    String getFormattedDate();
    String getFormattedTime();
    String getFormattedDateTime();
    void updateTime();  // Updates internal time counter

    // NTP settings
    bool syncTimeWithNTP(const char* ntpServer = "pool.ntp.org", long gmtOffset = 0, int daylightOffset = 0);
    void setNTPServer(const char* server);
    String getNTPServer();
    void setGMTOffset(long offset);
    long getGMTOffset();
    void setDaylightOffset(int offset);
    int getDaylightOffset();

private:
    Config();
    Preferences prefs;

    CommInterfaceType commInterface;
    String wifiSSID;
    String wifiPassword;
    String bleName;
    String mdnsName;
    DisplayTheme displayTheme;
    uint8_t brightness;
    AlertThresholds alertThresholds;
    uint16_t serverPort;
    uint16_t idleTimeout;  // Seconds before returning to idle screen

    // Date/Time storage
    int dateYear;
    int dateMonth;
    int dateDay;
    int timeHour;
    int timeMinute;
    int timeSecond;
    unsigned long lastTimeUpdate;

    // NTP storage
    String ntpServer;
    long gmtOffset;
    int daylightOffset;

    void loadSettings();
    void saveSettings();
};

#endif
