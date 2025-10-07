#ifndef BLE_COMM_H
#define BLE_COMM_H

#include "CommInterface.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// UUIDs for BLE service and characteristic
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class BLEComm : public CommInterface, public BLEServerCallbacks, public BLECharacteristicCallbacks {
public:
    BLEComm();
    ~BLEComm();

    bool begin() override;
    void update() override;
    bool isConnected() override;
    bool receiveData(SystemData& data) override;
    void stop() override;

    // BLE callbacks
    void onConnect(BLEServer* pServer) override;
    void onDisconnect(BLEServer* pServer) override;
    void onWrite(BLECharacteristic* pCharacteristic) override;

private:
    BLEServer* pServer;
    BLECharacteristic* pCharacteristic;
    bool deviceConnected;
    bool dataAvailable;
    String receivedData;
    unsigned long lastReceiveTime;
};

#endif
