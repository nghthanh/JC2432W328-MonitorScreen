#include "BLEComm.h"
#include "Config.h"
#include <ArduinoJson.h>

BLEComm::BLEComm() : pServer(nullptr), pCharacteristic(nullptr),
                     deviceConnected(false), dataAvailable(false), lastReceiveTime(0) {
}

BLEComm::~BLEComm() {
    stop();
}

bool BLEComm::begin() {
    Config& cfg = Config::getInstance();
    String bleName = cfg.getBLEName();

    Serial.printf("Initializing BLE: %s\n", bleName.c_str());

    BLEDevice::init(bleName.c_str());
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(this);

    BLEService* pService = pServer->createService(SERVICE_UUID);

    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );

    pCharacteristic->setCallbacks(this);
    pCharacteristic->addDescriptor(new BLE2902());

    pService->start();

    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();

    Serial.println("BLE advertising started");
    return true;
}

void BLEComm::update() {
    // Handle reconnection if needed
    if (!deviceConnected && pServer->getConnectedCount() == 0) {
        delay(500);
        pServer->startAdvertising();
    }
}

bool BLEComm::isConnected() {
    return deviceConnected;
}

bool BLEComm::receiveData(SystemData& data) {
    if (dataAvailable) {
        dataAvailable = false;
        bool result = parseJSON(receivedData.c_str(), data);
        receivedData = "";
        return result;
    }
    return false;
}

void BLEComm::stop() {
    if (pServer) {
        BLEDevice::deinit(true);
        pServer = nullptr;
        pCharacteristic = nullptr;
    }
    deviceConnected = false;
}

// BLE callbacks
void BLEComm::onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("BLE client connected");
}

void BLEComm::onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("BLE client disconnected");
}

void BLEComm::onWrite(BLECharacteristic* pCharacteristic) {
    String value = pCharacteristic->getValue().c_str();
    if (value.length() > 0) {
        receivedData = value;
        dataAvailable = true;
        lastReceiveTime = millis();
    }
}
