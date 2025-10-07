#include "CLICommands.h"
#include "Config.h"
#include <WiFi.h>

// WiFi scan results storage
#define MAX_SCAN_RESULTS 20
static String scanResults[MAX_SCAN_RESULTS];
static int32_t scanRSSI[MAX_SCAN_RESULTS];
static int scanCount = 0;

void registerCLICommands() {
    CLI& cli = CLI::getInstance();

    cli.registerCommand("setwifi", "Set WiFi credentials (setwifi \"SSID\" password)", cmdSetWiFi);
    cli.registerCommand("scanwifi", "Scan for WiFi networks", cmdScanWiFi);
    cli.registerCommand("selectwifi", "Select WiFi by index (selectwifi <index> <password>)", cmdSelectWiFi);
    cli.registerCommand("setinterface", "Set communication interface (setinterface wifi|ble)", cmdSetInterface);
    cli.registerCommand("setblename", "Set BLE device name (setblename <name>)", cmdSetBLEName);
    cli.registerCommand("settheme", "Set display theme (settheme 0-3)", cmdSetTheme);
    cli.registerCommand("setbrightness", "Set display brightness (setbrightness 0-255)", cmdSetBrightness);
    cli.registerCommand("setalert", "Set alert threshold (setalert cpu|mem|disk <value>)", cmdSetAlert);
    cli.registerCommand("setport", "Set server port (setport <port>)", cmdSetPort);
}

void cmdSetWiFi(int argc, char* argv[]) {
    CLI& cli = CLI::getInstance();

    if (argc < 3) {
        cli.println("Usage: setwifi \"SSID\" password");
        cli.println("Example: setwifi \"My WiFi Network\" mypassword");
        cli.println("Note: Use quotes if SSID contains spaces");
        return;
    }

    Config& cfg = Config::getInstance();
    cfg.setWiFiSSID(argv[1]);
    cfg.setWiFiPassword(argv[2]);

    cli.printf("WiFi credentials set: SSID=%s\n", argv[1]);
    cli.println("Restart required for changes to take effect");
}

void cmdScanWiFi(int argc, char* argv[]) {
    CLI& cli = CLI::getInstance();

    cli.println("Scanning for WiFi networks...");

    // Stop any ongoing WiFi operations
    WiFi.disconnect(true);
    delay(100);

    // Ensure WiFi is in station mode
    WiFi.mode(WIFI_STA);
    delay(500);

    // Start scan (delete old results first)
    WiFi.scanDelete();

    // Perform the scan
    int n = WiFi.scanNetworks(false, true, false, 300);  // async=false, show_hidden=true, passive=false, max_ms_per_chan=300
    scanCount = 0;

    if (n < 0) {
        cli.printf("Scan failed with error code: %d\n", n);
        cli.println("Possible causes:");
        cli.println("  - WiFi is currently in use by the system");
        cli.println("  - WiFi hardware issue");
        cli.println("  - Try the 'reset' command first, then scan again");
        return;
    }

    if (n == 0) {
        cli.println("No networks found");
        return;
    }

    cli.printf("Found %d networks:\n\n", n);
    cli.println("Index  RSSI  Ch  Encryption  SSID");
    cli.println("-----  ----  --  ----------  ----");

    for (int i = 0; i < n && i < MAX_SCAN_RESULTS; i++) {
        scanResults[i] = WiFi.SSID(i);
        scanRSSI[i] = WiFi.RSSI(i);

        char encType[10];
        switch (WiFi.encryptionType(i)) {
            case WIFI_AUTH_OPEN:
                strcpy(encType, "Open");
                break;
            case WIFI_AUTH_WEP:
                strcpy(encType, "WEP");
                break;
            case WIFI_AUTH_WPA_PSK:
                strcpy(encType, "WPA");
                break;
            case WIFI_AUTH_WPA2_PSK:
                strcpy(encType, "WPA2");
                break;
            case WIFI_AUTH_WPA_WPA2_PSK:
                strcpy(encType, "WPA/WPA2");
                break;
            case WIFI_AUTH_WPA2_ENTERPRISE:
                strcpy(encType, "WPA2-ENT");
                break;
            default:
                strcpy(encType, "Unknown");
                break;
        }

        // Format: [index] RSSI Ch Enc SSID
        char line[128];
        snprintf(line, sizeof(line), "[%2d]  %4d  %2d  %-10s  %s",
                 i, scanRSSI[i], WiFi.channel(i), encType, scanResults[i].c_str());
        cli.println(line);

        scanCount++;
    }

    cli.println("\nUse 'selectwifi <index> <password>' to connect");
    cli.println("Example: selectwifi 0 mypassword");

    // Clean up
    WiFi.scanDelete();
}

void cmdSelectWiFi(int argc, char* argv[]) {
    CLI& cli = CLI::getInstance();

    if (argc < 2) {
        cli.println("Usage: selectwifi <index> <password>");
        cli.println("Run 'scanwifi' first to see available networks");
        return;
    }

    int index = atoi(argv[1]);

    if (index < 0 || index >= scanCount) {
        cli.printf("Invalid index. Valid range: 0-%d\n", scanCount - 1);
        cli.println("Run 'scanwifi' to see available networks");
        return;
    }

    const char* password = "";
    if (argc >= 3) {
        password = argv[2];
    }

    Config& cfg = Config::getInstance();
    cfg.setWiFiSSID(scanResults[index].c_str());
    cfg.setWiFiPassword(password);

    cli.printf("WiFi configured:\n");
    cli.printf("  SSID: %s\n", scanResults[index].c_str());
    cli.printf("  RSSI: %d dBm\n", scanRSSI[index]);
    cli.println("Restart required for changes to take effect");
}

void cmdSetInterface(int argc, char* argv[]) {
    CLI& cli = CLI::getInstance();

    if (argc < 2) {
        cli.println("Usage: setinterface wifi|ble");
        return;
    }

    Config& cfg = Config::getInstance();

    if (strcmp(argv[1], "wifi") == 0) {
        cfg.setCommInterface(COMM_WIFI);
        cli.println("Interface set to WiFi");
    } else if (strcmp(argv[1], "ble") == 0) {
        cfg.setCommInterface(COMM_BLE);
        cli.println("Interface set to BLE");
    } else {
        cli.println("Invalid interface. Use 'wifi' or 'ble'");
        return;
    }

    cli.println("Restart required for changes to take effect");
}

void cmdSetBLEName(int argc, char* argv[]) {
    CLI& cli = CLI::getInstance();

    if (argc < 2) {
        cli.println("Usage: setblename <name>");
        return;
    }

    Config::getInstance().setBLEName(argv[1]);
    cli.printf("BLE name set to: %s\n", argv[1]);
    cli.println("Restart required for changes to take effect");
}

void cmdSetTheme(int argc, char* argv[]) {
    CLI& cli = CLI::getInstance();

    if (argc < 2) {
        cli.println("Usage: settheme <0-3>");
        cli.println("  0 - Default");
        cli.println("  1 - Minimal");
        cli.println("  2 - Graph");
        cli.println("  3 - Compact");
        return;
    }

    int theme = atoi(argv[1]);
    if (theme < 0 || theme > 3) {
        cli.println("Theme must be between 0 and 3");
        return;
    }

    Config::getInstance().setDisplayTheme((DisplayTheme)theme);
    cli.printf("Display theme set to: %d\n", theme);
}

void cmdSetBrightness(int argc, char* argv[]) {
    CLI& cli = CLI::getInstance();

    if (argc < 2) {
        cli.println("Usage: setbrightness <0-255>");
        return;
    }

    int brightness = atoi(argv[1]);
    if (brightness < 0 || brightness > 255) {
        cli.println("Brightness must be between 0 and 255");
        return;
    }

    Config::getInstance().setBrightness((uint8_t)brightness);
    cli.printf("Brightness set to: %d\n", brightness);
}

void cmdSetAlert(int argc, char* argv[]) {
    CLI& cli = CLI::getInstance();

    if (argc < 3) {
        cli.println("Usage: setalert cpu|mem|disk <value>");
        return;
    }

    float value = atof(argv[2]);
    Config& cfg = Config::getInstance();
    AlertThresholds thresh = cfg.getAlertThresholds();

    if (strcmp(argv[1], "cpu") == 0) {
        thresh.cpuTempHigh = value;
        cli.printf("CPU temperature alert threshold set to: %.1f°C\n", value);
    } else if (strcmp(argv[1], "mem") == 0) {
        thresh.memoryLow = value;
        cli.printf("Memory low alert threshold set to: %.1f%%\n", value);
    } else if (strcmp(argv[1], "disk") == 0) {
        thresh.diskLow = value;
        cli.printf("Disk low alert threshold set to: %.1f%%\n", value);
    } else {
        cli.println("Invalid alert type. Use 'cpu', 'mem', or 'disk'");
        return;
    }

    cfg.setAlertThresholds(thresh);
}

void cmdSetPort(int argc, char* argv[]) {
    CLI& cli = CLI::getInstance();

    if (argc < 2) {
        cli.println("Usage: setport <port>");
        return;
    }

    int port = atoi(argv[1]);
    if (port < 1 || port > 65535) {
        cli.println("Port must be between 1 and 65535");
        return;
    }

    Config::getInstance().setServerPort((uint16_t)port);
    cli.printf("Server port set to: %d\n", port);
    cli.println("Restart required for changes to take effect");
}
