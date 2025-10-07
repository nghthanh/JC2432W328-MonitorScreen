#include "CommManager.h"

CommManager::CommManager() : activeInterface(nullptr) {
}

CommManager::~CommManager() {
    stop();
}

CommManager& CommManager::getInstance() {
    static CommManager instance;
    return instance;
}

bool CommManager::begin() {
    Config& cfg = Config::getInstance();
    CommInterfaceType interface = cfg.getCommInterface();

    if (interface == COMM_WIFI) {
        Serial.println("Starting WiFi communication...");
        activeInterface = &wifiComm;
    } else {
        Serial.println("Starting BLE communication...");
        activeInterface = &bleComm;
    }

    return activeInterface->begin();
}

void CommManager::update() {
    if (activeInterface) {
        activeInterface->update();
    }
}

bool CommManager::isConnected() {
    return activeInterface && activeInterface->isConnected();
}

bool CommManager::receiveData(SystemData& data) {
    if (activeInterface) {
        return activeInterface->receiveData(data);
    }
    return false;
}

void CommManager::stop() {
    if (activeInterface) {
        activeInterface->stop();
        activeInterface = nullptr;
    }
}
