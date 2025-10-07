/*
 * ESP32 System Monitor
 *
 * A comprehensive system monitor that displays PC system information
 * on a TFT display (ST7789) via WiFi or BLE interface.
 *
 * Features:
 * - WiFi and BLE communication support
 * - Multiple display themes
 * - CLI configuration via Serial
 * - Web configuration interface
 * - Alert system for critical values
 * - Graph history display
 */

#include <WiFi.h>
#include "Config.h"
#include "CLI.h"
#include "CLICommands.h"
#include "CommManager.h"
#include "Display.h"
#include "MonitorWebServer.h"

// Module instances
Config& config = Config::getInstance();
CLI& cli = CLI::getInstance();
CommManager& comm = CommManager::getInstance();
Display& display = Display::getInstance();
MonitorWebServer& webServer = MonitorWebServer::getInstance();

// System data
SystemData systemData;
unsigned long lastDataTime = 0;
unsigned long lastDisplayUpdate = 0;

#define DATA_TIMEOUT 5000      // No data timeout (ms)
#define DISPLAY_UPDATE_RATE 500 // Display update rate (ms)

void setup() {
    // Initialize configuration
    config.begin();

    // Initialize CLI
    cli.begin(115200);
    registerCLICommands();

    // Initialize display
    display.begin();
    display.showStatus("Initializing...");

    // Initialize communication
    bool commStarted = comm.begin();
    if (commStarted) {
        display.showStatus("Communication started");

        // Show connection info on startup
        if (config.getCommInterface() == COMM_WIFI && WiFi.status() == WL_CONNECTED) {
            char info[64];
            snprintf(info, sizeof(info), "WiFi: %s:%d", WiFi.localIP().toString().c_str(), config.getServerPort());
            display.showConnectionInfo(info);
        } else if (config.getCommInterface() == COMM_BLE) {
            char info[64];
            snprintf(info, sizeof(info), "BLE: %s", config.getBLEName().c_str());
            display.showConnectionInfo(info);
        }
    } else {
        display.showStatus("Communication failed!");
        if (config.getCommInterface() == COMM_WIFI && config.getWiFiSSID().length() == 0) {
            Serial.println("\n*** WiFi not configured! ***");
            Serial.println("Use these commands to configure WiFi:");
            Serial.println("  setwifi <SSID> <password>");
            Serial.println("  reset");
            Serial.println("Then restart the device.\n");
        }
    }

    // Initialize web server (only if using WiFi and connected)
    if (config.getCommInterface() == COMM_WIFI && commStarted && WiFi.status() == WL_CONNECTED) {
        webServer.begin();
    }

    Serial.println("\n=== System Monitor Ready ===");
    Serial.println("Waiting for data from PC...\n");
}

void loop() {
    // Update CLI
    cli.update();

    // Update communication
    comm.update();

    // Check for incoming data
    if (comm.receiveData(systemData)) {
        lastDataTime = millis();

        // Update web server data
        webServer.setSystemData(systemData);

        // Update display at controlled rate
        if (millis() - lastDisplayUpdate >= DISPLAY_UPDATE_RATE) {
            display.update(systemData);
            lastDisplayUpdate = millis();
        }
    }

    // Update web server
    webServer.update();

    // Check for data timeout
    if (lastDataTime > 0 && (millis() - lastDataTime > DATA_TIMEOUT)) {
        static unsigned long lastTimeoutMsg = 0;
        if (millis() - lastTimeoutMsg > 10000) {
            display.showStatus("No data received");
            Serial.println("Warning: No data received from PC");
            lastTimeoutMsg = millis();
        }
    }

    // Small delay to prevent watchdog issues
    delay(10);
}
