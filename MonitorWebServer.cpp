#include "MonitorWebServer.h"

MonitorWebServer::MonitorWebServer() : server(nullptr) {
}

MonitorWebServer& MonitorWebServer::getInstance() {
    static MonitorWebServer instance;
    return instance;
}

void MonitorWebServer::begin() {
    Config& cfg = Config::getInstance();

    // Only start web server if using WiFi
    if (cfg.getCommInterface() != COMM_WIFI) {
        Serial.println("Web server disabled (not using WiFi)");
        return;
    }

    server = new WebServer(80);

    server->on("/", HTTP_GET, handleRoot);
    server->on("/config", HTTP_GET, handleConfig);
    server->on("/config", HTTP_POST, handleConfigSave);
    server->on("/status", HTTP_GET, handleStatus);
    server->on("/restart", HTTP_GET, handleRestart);
    server->onNotFound(handleNotFound);

    server->begin();
    Serial.println("Web server started on port 80");
}

void MonitorWebServer::update() {
    if (server) {
        server->handleClient();
    }
}

void MonitorWebServer::setSystemData(const SystemData& data) {
    currentData = data;
}

void MonitorWebServer::handleRoot() {
    MonitorWebServer::getInstance().server->send(200, "text/html", generateHomePage());
}

void MonitorWebServer::handleConfig() {
    MonitorWebServer::getInstance().server->send(200, "text/html", generateConfigPage());
}

void MonitorWebServer::handleConfigSave() {
    WebServer* srv = MonitorWebServer::getInstance().server;
    Config& cfg = Config::getInstance();

    if (srv->hasArg("ssid")) {
        cfg.setWiFiSSID(srv->arg("ssid").c_str());
    }
    if (srv->hasArg("password")) {
        cfg.setWiFiPassword(srv->arg("password").c_str());
    }
    if (srv->hasArg("theme")) {
        cfg.setDisplayTheme((DisplayTheme)srv->arg("theme").toInt());
    }
    if (srv->hasArg("brightness")) {
        cfg.setBrightness((uint8_t)srv->arg("brightness").toInt());
    }
    if (srv->hasArg("datetime")) {
        String datetime = srv->arg("datetime");
        int year, month, day, hour, minute, second;
        if (sscanf(datetime.c_str(), "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second) == 6) {
            cfg.setDateTime(year, month, day, hour, minute, second);
        }
    }
    if (srv->hasArg("idletimeout")) {
        cfg.setIdleTimeout((uint16_t)srv->arg("idletimeout").toInt());
    }

    srv->send(200, "text/html",
        "<html><body><h1>Configuration Saved</h1>"
        "<p>Settings have been saved.</p>"
        "<a href='/'>Back to Home</a></body></html>");
}

void MonitorWebServer::handleStatus() {
    WebServer* srv = MonitorWebServer::getInstance().server;
    SystemData& data = MonitorWebServer::getInstance().currentData;

    String json = "{";
    json += "\"cpu\":{\"usage\":" + String(data.cpuUsage) + ",\"temp\":" + String(data.cpuTemp) + "},";
    json += "\"memory\":{\"used\":" + String(data.memoryUsed) + ",\"total\":" + String(data.memoryTotal) + ",\"percent\":" + String(data.memoryPercent) + "},";
    json += "\"disk\":{\"used\":" + String(data.diskUsed) + ",\"total\":" + String(data.diskTotal) + ",\"percent\":" + String(data.diskPercent) + "},";
    json += "\"network\":{\"upload\":" + String(data.networkUpload) + ",\"download\":" + String(data.networkDownload) + "}";
    json += "}";

    srv->send(200, "application/json", json);
}

void MonitorWebServer::handleRestart() {
    MonitorWebServer::getInstance().server->send(200, "text/html",
        "<html><body><h1>Restarting...</h1>"
        "<p>Device will restart in 3 seconds.</p></body></html>");
    delay(3000);
    ESP.restart();
}

void MonitorWebServer::handleNotFound() {
    MonitorWebServer::getInstance().server->send(404, "text/plain", "Not Found");
}

String MonitorWebServer::generateHomePage() {
    SystemData& data = MonitorWebServer::getInstance().currentData;

    String html = "<!DOCTYPE html><html><head>";
    html += "<title>ESP32 System Monitor</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += "body { font-family: Arial; margin: 20px; background: #1a1a1a; color: #fff; }";
    html += "h1 { color: #4CAF50; }";
    html += ".info { background: #2a2a2a; padding: 15px; margin: 10px 0; border-radius: 5px; }";
    html += ".label { color: #4CAF50; font-weight: bold; }";
    html += ".value { color: #fff; }";
    html += "a { color: #4CAF50; text-decoration: none; padding: 10px; display: inline-block; }";
    html += "</style>";
    html += "<script>";
    html += "setInterval(function() { location.reload(); }, 2000);";
    html += "</script>";
    html += "</head><body>";
    html += "<h1>ESP32 System Monitor</h1>";

    html += "<div class='info'>";
    html += "<span class='label'>CPU Usage:</span> <span class='value'>" + String(data.cpuUsage, 1) + "%</span><br>";
    html += "<span class='label'>CPU Temp:</span> <span class='value'>" + String(data.cpuTemp, 1) + "°C</span>";
    html += "</div>";

    html += "<div class='info'>";
    html += "<span class='label'>Memory:</span> <span class='value'>" + String(data.memoryUsed, 1) + " / " + String(data.memoryTotal, 1) + " GB (" + String(data.memoryPercent, 1) + "%)</span>";
    html += "</div>";

    html += "<div class='info'>";
    html += "<span class='label'>Disk:</span> <span class='value'>" + String(data.diskUsed, 1) + " / " + String(data.diskTotal, 1) + " GB (" + String(data.diskPercent, 1) + "%)</span>";
    html += "</div>";

    html += "<div class='info'>";
    html += "<span class='label'>Network Upload:</span> <span class='value'>" + String(data.networkUpload, 2) + " KB/s</span><br>";
    html += "<span class='label'>Network Download:</span> <span class='value'>" + String(data.networkDownload, 2) + " KB/s</span>";
    html += "</div>";

    html += "<div class='info'>";
    html += "<span class='label'>Date/Time:</span> <span class='value'>" + Config::getInstance().getFormattedDateTime() + "</span>";
    html += "</div>";

    html += "<br><a href='/config'>Configuration</a> | ";
    html += "<a href='/status'>JSON Status</a> | ";
    html += "<a href='/restart'>Restart Device</a>";

    html += "</body></html>";
    return html;
}

String MonitorWebServer::generateConfigPage() {
    Config& cfg = Config::getInstance();

    String html = "<!DOCTYPE html><html><head>";
    html += "<title>Configuration</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += "body { font-family: Arial; margin: 20px; background: #1a1a1a; color: #fff; }";
    html += "h1 { color: #4CAF50; }";
    html += "form { background: #2a2a2a; padding: 20px; border-radius: 5px; }";
    html += "label { display: block; margin: 10px 0 5px; color: #4CAF50; }";
    html += "input, select { width: 100%; padding: 8px; margin-bottom: 15px; background: #3a3a3a; border: 1px solid #4a4a4a; color: #fff; border-radius: 3px; }";
    html += "button { background: #4CAF50; color: white; padding: 10px 20px; border: none; border-radius: 5px; cursor: pointer; }";
    html += "button:hover { background: #45a049; }";
    html += "a { color: #4CAF50; text-decoration: none; }";
    html += "</style>";
    html += "</head><body>";
    html += "<h1>Configuration</h1>";
    html += "<form method='POST' action='/config'>";

    html += "<label>WiFi SSID:</label>";
    html += "<input type='text' name='ssid' value='" + cfg.getWiFiSSID() + "'>";

    html += "<label>WiFi Password:</label>";
    html += "<input type='password' name='password' value='" + cfg.getWiFiPassword() + "'>";

    html += "<label>Display Theme:</label>";
    html += "<select name='theme'>";
    html += "<option value='0'" + String(cfg.getDisplayTheme() == 0 ? " selected" : "") + ">Default</option>";
    html += "<option value='1'" + String(cfg.getDisplayTheme() == 1 ? " selected" : "") + ">Minimal</option>";
    html += "<option value='2'" + String(cfg.getDisplayTheme() == 2 ? " selected" : "") + ">Graph</option>";
    html += "<option value='3'" + String(cfg.getDisplayTheme() == 3 ? " selected" : "") + ">Compact</option>";
    html += "</select>";

    html += "<label>Brightness (0-255):</label>";
    html += "<input type='number' name='brightness' min='0' max='255' value='" + String(cfg.getBrightness()) + "'>";

    html += "<label>Date/Time:</label>";
    int year, month, day, hour, minute, second;
    cfg.getDateTime(year, month, day, hour, minute, second);
    char datetimeBuf[32];
    sprintf(datetimeBuf, "%04d-%02d-%02dT%02d:%02d:%02d", year, month, day, hour, minute, second);
    html += "<input type='datetime-local' name='datetime' value='" + String(datetimeBuf) + "'>";

    html += "<label>Idle Timeout (seconds, 0=disabled):</label>";
    html += "<input type='number' name='idletimeout' min='0' max='65535' value='" + String(cfg.getIdleTimeout()) + "'>";

    html += "<br><button type='submit'>Save Configuration</button>";
    html += "</form>";
    html += "<br><a href='/'>Back to Home</a>";
    html += "</body></html>";
    return html;
}

String MonitorWebServer::generateStatusPage() {
    return "See /status endpoint";
}
