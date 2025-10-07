#ifndef MONITOR_WEB_SERVER_H
#define MONITOR_WEB_SERVER_H

#ifdef ESP32
  #include <WiFi.h>
  #include <FS.h>
  // Create global FS alias for WebServer compatibility with ESP32 core 3.x
  using FS = fs::FS;
  #include <WebServer.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  #define WebServer ESP8266WebServer
#endif

#include "Config.h"
#include "SystemData.h"

class MonitorWebServer {
public:
    static MonitorWebServer& getInstance();

    void begin();
    void update();
    void setSystemData(const SystemData& data);

private:
    MonitorWebServer();

    WebServer* server;
    SystemData currentData;

    // Web handlers
    static void handleRoot();
    static void handleConfig();
    static void handleConfigSave();
    static void handleStatus();
    static void handleRestart();
    static void handleNotFound();

    // HTML page generators
    static String generateHomePage();
    static String generateConfigPage();
    static String generateStatusPage();
};

#endif
