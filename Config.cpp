#include "Config.h"

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
