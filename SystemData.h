#ifndef SYSTEM_DATA_H
#define SYSTEM_DATA_H

#include <Arduino.h>

// System data structure received from PC
struct SystemData {
    // CPU info
    float cpuUsage;
    float cpuTemp;
    char cpuName[64];

    // Memory info
    float memoryUsed;
    float memoryTotal;
    float memoryPercent;

    // Disk info
    float diskUsed;
    float diskTotal;
    float diskPercent;

    // Network info
    float networkUpload;
    float networkDownload;

    // GPU info (optional)
    float gpuUsage;
    float gpuTemp;

    // Additional temperature sensors
    float motherboardTemp;
    float diskTemp;  // First disk temperature if available
    char diskName[32];  // Disk name

    // Timestamp
    unsigned long timestamp;

    SystemData() {
        cpuUsage = 0;
        cpuTemp = 0;
        memset(cpuName, 0, sizeof(cpuName));
        memoryUsed = 0;
        memoryTotal = 0;
        memoryPercent = 0;
        diskUsed = 0;
        diskTotal = 0;
        diskPercent = 0;
        networkUpload = 0;
        networkDownload = 0;
        gpuUsage = 0;
        gpuTemp = 0;
        motherboardTemp = 0;
        diskTemp = 0;
        memset(diskName, 0, sizeof(diskName));
        timestamp = 0;
    }
};

#endif
