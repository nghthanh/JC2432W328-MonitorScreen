#ifndef COMM_MANAGER_H
#define COMM_MANAGER_H

#include "CommInterface.h"
#include "WiFiComm.h"
#include "BLEComm.h"
#include "Config.h"

class CommManager {
public:
    static CommManager& getInstance();

    bool begin();
    void update();
    bool isConnected();
    bool receiveData(SystemData& data);
    void stop();

private:
    CommManager();
    ~CommManager();

    CommInterface* activeInterface;
    WiFiComm wifiComm;
    BLEComm bleComm;
};

#endif
