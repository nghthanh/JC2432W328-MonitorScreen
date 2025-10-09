#ifndef WIFI_COMM_H
#define WIFI_COMM_H

#include "CommInterface.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESPmDNS.h>

class WiFiComm : public CommInterface {
public:
    WiFiComm();
    ~WiFiComm();

    bool begin() override;
    void update() override;
    bool isConnected() override;
    bool receiveData(SystemData& data) override;
    void stop() override;

private:
    WiFiUDP udp;
    uint16_t localPort;
    char packetBuffer[1024];
    bool connected;
    unsigned long lastReceiveTime;

    void connectToWiFi();
};

#endif
