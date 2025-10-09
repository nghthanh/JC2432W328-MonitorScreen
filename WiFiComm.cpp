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
        Serial.println("WiFi SSID not configured\r\n");
        // Ensure WiFi is fully stopped if not configured
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        return false;
    }

    connectToWiFi();

    if (WiFi.status() == WL_CONNECTED) {
        udp.begin(localPort);
        Serial.printf("WiFi connected. IP: %s, Port: %d\r\n",
                     WiFi.localIP().toString().c_str(), localPort);

        // Initialize mDNS
        String mdnsName = cfg.getMDNSName();
        if (MDNS.begin(mdnsName.c_str())) {
            Serial.printf("mDNS responder started: %s.local\r\n", mdnsName.c_str());

            // Add service to mDNS-SD
            MDNS.addService("esp32monitor", "udp", localPort);
            Serial.printf("mDNS service advertised: _esp32monitor._udp.local port %d\r\n", localPort);
        } else {
            Serial.println("Error starting mDNS responder\r\n");
        }

        connected = true;
        return true;
    }

    // Connection failed - clean up WiFi
    Serial.println("WiFi connection failed\r\n");
    WiFi.disconnect(true);
    return false;
}

void WiFiComm::connectToWiFi() {
    Config& cfg = Config::getInstance();
    String ssid = cfg.getWiFiSSID();
    String password = cfg.getWiFiPassword();

    Serial.printf("Connecting to WiFi: %s\r\n", ssid.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println("\r\n");
}

void WiFiComm::update() {
    if (WiFi.status() != WL_CONNECTED) {
        if (connected) {
            Serial.println("WiFi disconnected\r\n");
            connected = false;
        }
        return;
    }

    if (!connected) {
        connected = true;
        Serial.println("WiFi reconnected\r\n");
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
    MDNS.end();
    udp.stop();
    WiFi.disconnect();
    connected = false;
}
