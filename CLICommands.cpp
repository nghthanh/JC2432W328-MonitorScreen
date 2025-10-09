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
    cli.registerCommand("setmdnsname", "Set mDNS hostname (setmdnsname <name>)", cmdSetMDNSName);
    cli.registerCommand("settheme", "Set display theme (settheme 0-3)", cmdSetTheme);
    cli.registerCommand("setbrightness", "Set display brightness (setbrightness 0-255)", cmdSetBrightness);
    cli.registerCommand("setalert", "Set alert threshold (setalert cpu|mem|disk <value>)", cmdSetAlert);
    cli.registerCommand("setport", "Set server port (setport <port>)", cmdSetPort);
    cli.registerCommand("setdatetime", "Set date and time (setdatetime YYYY-MM-DD HH:MM:SS)", cmdSetDateTime);
    cli.registerCommand("getdatetime", "Get current date and time", cmdGetDateTime);
    cli.registerCommand("syncntp", "Sync time with NTP server", cmdSyncNTP);
    cli.registerCommand("setntpserver", "Set NTP server (setntpserver <server>)", cmdSetNTPServer);
    cli.registerCommand("settimezone", "Set timezone offset in seconds (settimezone <gmtOffset> [dstOffset])", cmdSetTimezone);
    cli.registerCommand("setidletimeout", "Set idle timeout in seconds (setidletimeout <seconds>)", cmdSetIdleTimeout);
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

void cmdSetMDNSName(int argc, char* argv[]) {
    CLI& cli = CLI::getInstance();

    if (argc < 2) {
        cli.println("Usage: setmdnsname <name>");
        cli.println("Sets the mDNS hostname for device discovery");
        cli.println("Example: setmdnsname mymonitor");
        cli.println("Device will be accessible as <name>.local");
        cli.printf("Current mDNS name: %s\n", Config::getInstance().getMDNSName().c_str());
        return;
    }

    // Validate mDNS name (only alphanumeric and hyphens, lowercase)
    String name = String(argv[1]);
    name.toLowerCase();

    bool valid = true;
    for (size_t i = 0; i < name.length(); i++) {
        char c = name.charAt(i);
        if (!isalnum(c) && c != '-') {
            valid = false;
            break;
        }
    }

    if (!valid) {
        cli.println("Invalid mDNS name. Use only letters, numbers, and hyphens");
        return;
    }

    Config::getInstance().setMDNSName(name.c_str());
    cli.printf("mDNS name set to: %s.local\n", name.c_str());
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

void cmdSetDateTime(int argc, char* argv[]) {
    CLI& cli = CLI::getInstance();

    if (argc < 3) {
        cli.println("Usage: setdatetime YYYY-MM-DD HH:MM:SS");
        cli.println("Example: setdatetime 2025-10-08 14:30:00");
        return;
    }

    // Parse date (YYYY-MM-DD)
    int year, month, day;
    if (sscanf(argv[1], "%d-%d-%d", &year, &month, &day) != 3) {
        cli.println("Invalid date format. Use YYYY-MM-DD");
        return;
    }

    // Parse time (HH:MM:SS)
    int hour, minute, second;
    if (sscanf(argv[2], "%d:%d:%d", &hour, &minute, &second) != 3) {
        cli.println("Invalid time format. Use HH:MM:SS");
        return;
    }

    // Validate ranges
    if (year < 2000 || year > 2099) {
        cli.println("Year must be between 2000 and 2099");
        return;
    }
    if (month < 1 || month > 12) {
        cli.println("Month must be between 1 and 12");
        return;
    }
    if (day < 1 || day > 31) {
        cli.println("Day must be between 1 and 31");
        return;
    }
    if (hour < 0 || hour > 23) {
        cli.println("Hour must be between 0 and 23");
        return;
    }
    if (minute < 0 || minute > 59) {
        cli.println("Minute must be between 0 and 59");
        return;
    }
    if (second < 0 || second > 59) {
        cli.println("Second must be between 0 and 59");
        return;
    }

    Config::getInstance().setDateTime(year, month, day, hour, minute, second);
    cli.printf("Date/Time set to: %04d-%02d-%02d %02d:%02d:%02d\n",
               year, month, day, hour, minute, second);
}

void cmdGetDateTime(int argc, char* argv[]) {
    CLI& cli = CLI::getInstance();

    int year, month, day, hour, minute, second;
    Config::getInstance().getDateTime(year, month, day, hour, minute, second);

    cli.printf("Current Date/Time: %04d-%02d-%02d %02d:%02d:%02d\n",
               year, month, day, hour, minute, second);
}

void cmdSyncNTP(int argc, char* argv[]) {
    CLI& cli = CLI::getInstance();
    Config& cfg = Config::getInstance();

    cli.println("Syncing time with NTP server...");

    bool success = cfg.syncTimeWithNTP(
        cfg.getNTPServer().c_str(),
        cfg.getGMTOffset(),
        cfg.getDaylightOffset()
    );

    if (success) {
        cli.println("Time synchronized successfully!");
        int year, month, day, hour, minute, second;
        cfg.getDateTime(year, month, day, hour, minute, second);
        cli.printf("Current time: %04d-%02d-%02d %02d:%02d:%02d\n",
                   year, month, day, hour, minute, second);
    } else {
        cli.println("Failed to sync time. Make sure WiFi is connected.");
    }
}

void cmdSetNTPServer(int argc, char* argv[]) {
    CLI& cli = CLI::getInstance();

    if (argc < 2) {
        cli.println("Usage: setntpserver <server>");
        cli.println("Example: setntpserver pool.ntp.org");
        cli.printf("Current server: %s\n", Config::getInstance().getNTPServer().c_str());
        return;
    }

    Config::getInstance().setNTPServer(argv[1]);
    cli.printf("NTP server set to: %s\n", argv[1]);
}

void cmdSetTimezone(int argc, char* argv[]) {
    CLI& cli = CLI::getInstance();

    if (argc < 2) {
        cli.println("Usage: settimezone <gmtOffset> [dstOffset]");
        cli.println("GMT offset in seconds (e.g., 3600 for UTC+1, -18000 for UTC-5)");
        cli.println("DST offset in seconds (default: 0)");
        cli.println("Examples:");
        cli.println("  settimezone 0         - UTC");
        cli.println("  settimezone 3600      - UTC+1");
        cli.println("  settimezone -18000    - UTC-5 (EST)");
        cli.println("  settimezone -18000 3600 - EST with DST");
        Config& cfg = Config::getInstance();
        cli.printf("Current GMT offset: %ld seconds\n", cfg.getGMTOffset());
        cli.printf("Current DST offset: %d seconds\n", cfg.getDaylightOffset());
        return;
    }

    long gmtOffset = atol(argv[1]);
    int dstOffset = 0;

    if (argc >= 3) {
        dstOffset = atoi(argv[2]);
    }

    Config& cfg = Config::getInstance();
    cfg.setGMTOffset(gmtOffset);
    cfg.setDaylightOffset(dstOffset);

    cli.printf("Timezone set:\n");
    cli.printf("  GMT Offset: %ld seconds\n", gmtOffset);
    cli.printf("  DST Offset: %d seconds\n", dstOffset);
    cli.println("Use 'syncntp' to sync time with these settings");
}

void cmdSetIdleTimeout(int argc, char* argv[]) {
    CLI& cli = CLI::getInstance();

    if (argc < 2) {
        cli.println("Usage: setidletimeout <seconds>");
        cli.println("Set the idle timeout (time before returning to idle screen)");
        cli.println("Range: 0-65535 seconds (0 = disabled)");
        cli.printf("Current idle timeout: %d seconds\n", Config::getInstance().getIdleTimeout());
        return;
    }

    int timeout = atoi(argv[1]);
    if (timeout < 0 || timeout > 65535) {
        cli.println("Timeout must be between 0 and 65535 seconds");
        return;
    }

    Config::getInstance().setIdleTimeout((uint16_t)timeout);
    if (timeout == 0) {
        cli.println("Idle timeout disabled");
    } else {
        cli.printf("Idle timeout set to: %d seconds\n", timeout);
    }
}
