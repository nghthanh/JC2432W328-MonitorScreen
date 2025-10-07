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

private:
    Config();
    Preferences prefs;

    CommInterfaceType commInterface;
    String wifiSSID;
    String wifiPassword;
    String bleName;
    DisplayTheme displayTheme;
    uint8_t brightness;
    AlertThresholds alertThresholds;
    uint16_t serverPort;

    void loadSettings();
    void saveSettings();
};

#endif
