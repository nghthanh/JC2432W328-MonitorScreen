#include "WiFiComm.h"
#include "Config.h"
#include <ArduinoJson.h>

WiFiComm::WiFiComm() : connected(false), lastReceiveTime(0) {
    localPort = Config::getInstance().getServerPort();
}

WiFiComm::~WiFiComm() {
    stop();
}

bool WiFiComm::begin() {
    Config& cfg = Config::getInstance();
    String ssid = cfg.getWiFiSSID();
    String password = cfg.getWiFiPassword();

    if (ssid.length() == 0) {
        Serial.println("WiFi SSID not configured");
        // Ensure WiFi is fully stopped if not configured
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        return false;
    }

    connectToWiFi();

    if (WiFi.status() == WL_CONNECTED) {
        udp.begin(localPort);
        Serial.printf("WiFi connected. IP: %s, Port: %d\n",
                     WiFi.localIP().toString().c_str(), localPort);
        connected = true;
        return true;
    }

    // Connection failed - clean up WiFi
    Serial.println("WiFi connection failed");
    WiFi.disconnect(true);
    return false;
}

void WiFiComm::connectToWiFi() {
    Config& cfg = Config::getInstance();
    String ssid = cfg.getWiFiSSID();
    String password = cfg.getWiFiPassword();

    Serial.printf("Connecting to WiFi: %s\n", ssid.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println();
}

void WiFiComm::update() {
    if (WiFi.status() != WL_CONNECTED) {
        if (connected) {
            Serial.println("WiFi disconnected");
            connected = false;
        }
        return;
    }

    if (!connected) {
        connected = true;
        Serial.println("WiFi reconnected");
    }
}

bool WiFiComm::isConnected() {
    return connected && (WiFi.status() == WL_CONNECTED);
}

bool WiFiComm::receiveData(SystemData& data) {
    int packetSize = udp.parsePacket();
    if (packetSize > 0) {
        int len = udp.read(packetBuffer, sizeof(packetBuffer) - 1);
        if (len > 0) {
            packetBuffer[len] = 0;
            lastReceiveTime = millis();
            return parseJSON(packetBuffer, data);
        }
    }
    return false;
}

void WiFiComm::stop() {
    udp.stop();
    WiFi.disconnect();
    connected = false;
}
