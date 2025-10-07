#include "CommInterface.h"
#include <ArduinoJson.h>

bool CommInterface::parseJSON(const char* json, SystemData& data) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        Serial.print("JSON parse error: ");
        Serial.println(error.c_str());
        return false;
    }

    // Parse CPU data
    if (!doc["cpu"].isNull()) {
        data.cpuUsage = doc["cpu"]["usage"] | 0.0f;
        data.cpuTemp = doc["cpu"]["temp"] | 0.0f;
        const char* name = doc["cpu"]["name"];
        if (name) {
            strncpy(data.cpuName, name, sizeof(data.cpuName) - 1);
        }
    }

    // Parse Memory data
    if (!doc["memory"].isNull()) {
        data.memoryUsed = doc["memory"]["used"] | 0.0f;
        data.memoryTotal = doc["memory"]["total"] | 0.0f;
        data.memoryPercent = doc["memory"]["percent"] | 0.0f;
    }

    // Parse Disk data
    if (!doc["disk"].isNull()) {
        data.diskUsed = doc["disk"]["used"] | 0.0f;
        data.diskTotal = doc["disk"]["total"] | 0.0f;
        data.diskPercent = doc["disk"]["percent"] | 0.0f;
    }

    // Parse Network data
    if (!doc["network"].isNull()) {
        data.networkUpload = doc["network"]["upload"] | 0.0f;
        data.networkDownload = doc["network"]["download"] | 0.0f;
    }

    // Parse GPU data (optional)
    if (!doc["gpu"].isNull()) {
        data.gpuUsage = doc["gpu"]["usage"] | 0.0f;
        data.gpuTemp = doc["gpu"]["temp"] | 0.0f;
    }

    // Parse additional temperature data
    if (!doc["temperatures"].isNull()) {
        data.motherboardTemp = doc["temperatures"]["motherboard"] | 0.0f;

        // Get first disk temperature if available
        if (!doc["temperatures"]["disks"].isNull() && doc["temperatures"]["disks"].is<JsonArray>()) {
            JsonArray disks = doc["temperatures"]["disks"];
            if (disks.size() > 0) {
                data.diskTemp = disks[0]["temp"] | 0.0f;
                const char* diskName = disks[0]["name"];
                if (diskName) {
                    strncpy(data.diskName, diskName, sizeof(data.diskName) - 1);
                }
            }
        }
    }

    data.timestamp = millis();
    return true;
}
