#ifndef COMM_INTERFACE_H
#define COMM_INTERFACE_H

#include <Arduino.h>
#include "SystemData.h"

// Abstract communication interface
class CommInterface {
public:
    virtual ~CommInterface() {}

    virtual bool begin() = 0;
    virtual void update() = 0;
    virtual bool isConnected() = 0;
    virtual bool receiveData(SystemData& data) = 0;
    virtual void stop() = 0;

protected:
    bool parseJSON(const char* json, SystemData& data);
};

#endif
