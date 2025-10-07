#include "CLI.h"
#include "Config.h"
#include <WiFi.h>
#include <stdarg.h>

CLI::CLI() : cmdIndex(0) {
    memset(cmdBuffer, 0, CMD_BUFFER_SIZE);
}

CLI& CLI::getInstance() {
    static CLI instance;
    return instance;
}

void CLI::begin(unsigned long baudRate) {
    Serial.begin(baudRate);
    while (!Serial && millis() < 5000); // Wait for serial with timeout

    // Register built-in commands
    registerCommand("help", "Show available commands", cmdHelp);
    registerCommand("status", "Show system status", cmdStatus);
    registerCommand("reset", "Reset configuration to defaults", cmdReset);

    println("\n=== ESP32 System Monitor ===");
    println("Type 'help' for available commands\n");
}

void CLI::update() {
    static bool lastWasReturn = false;

    while (Serial.available()) {
        char c = Serial.read();

        if (c == '\n' || c == '\r') {
            // Skip if this is the second part of CRLF
            if (lastWasReturn && c == '\n') {
                lastWasReturn = false;
                continue;
            }

            lastWasReturn = (c == '\r');
            Serial.println();  // Send newline

            if (cmdIndex > 0) {
                cmdBuffer[cmdIndex] = '\0';
                processCommand(cmdBuffer);
                cmdIndex = 0;
                memset(cmdBuffer, 0, CMD_BUFFER_SIZE);
            }
            Serial.print("> ");
        } else {
            lastWasReturn = false;

            if (c == 8 || c == 127) { // Backspace
                if (cmdIndex > 0) {
                    cmdIndex--;
                    Serial.print("\b \b");
                }
            } else if (cmdIndex < CMD_BUFFER_SIZE - 1) {
                cmdBuffer[cmdIndex++] = c;
                Serial.write(c);
            }
        }
    }
}

void CLI::registerCommand(const char* name, const char* description, CommandHandler handler) {
    Command cmd;
    cmd.name = name;
    cmd.description = description;
    cmd.handler = handler;
    commands.push_back(cmd);
}

void CLI::processCommand(char* cmdLine) {
    char* argv[MAX_CMD_ARGS];
    int argc = 0;

    // Tokenize command line, handling quoted strings
    char* p = cmdLine;
    bool inQuote = false;
    char* start = p;

    while (*p && argc < MAX_CMD_ARGS) {
        if (*p == '"') {
            if (inQuote) {
                // End quote
                *p = '\0';
                argv[argc++] = start;
                inQuote = false;
                start = p + 1;
            } else {
                // Start quote
                inQuote = true;
                start = p + 1;
            }
        } else if (*p == ' ' && !inQuote) {
            if (p > start) {
                *p = '\0';
                argv[argc++] = start;
            }
            start = p + 1;
        }
        p++;
    }

    // Handle last token
    if (p > start && argc < MAX_CMD_ARGS) {
        argv[argc++] = start;
    }

    if (argc > 0) {
        executeCommand(argc, argv);
    }
}

void CLI::executeCommand(int argc, char* argv[]) {
    const char* cmdName = argv[0];

    for (const auto& cmd : commands) {
        if (strcmp(cmd.name, cmdName) == 0) {
            cmd.handler(argc, argv);
            return;
        }
    }

    printf("Unknown command: %s\n", cmdName);
    println("Type 'help' for available commands");
}

void CLI::showHelp() {
    println("Available commands:");
    for (const auto& cmd : commands) {
        Serial.print("  ");
        Serial.print(cmd.name);
        for (int i = strlen(cmd.name); i < 15; i++) {
            Serial.print(" ");
        }
        Serial.print(" - ");
        Serial.println(cmd.description);
    }
}

void CLI::print(const char* str) {
    Serial.print(str);
}

void CLI::println(const char* str) {
    Serial.println(str);
}

void CLI::printf(const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    Serial.print(buffer);
}

// Built-in command handlers
void CLI::cmdHelp(int argc, char* argv[]) {
    CLI::getInstance().showHelp();
}

void CLI::cmdStatus(int argc, char* argv[]) {
    CLI& cli = CLI::getInstance();
    Config& cfg = Config::getInstance();

    cli.println("\n=== System Status ===");
    cli.printf("Communication: %s\n", cfg.getCommInterface() == COMM_WIFI ? "WiFi" : "BLE");

    if (cfg.getCommInterface() == COMM_WIFI) {
        cli.printf("WiFi SSID: %s\n", cfg.getWiFiSSID().c_str());
        cli.printf("WiFi Status: ");
        switch(WiFi.status()) {
            case WL_CONNECTED:
                cli.println("Connected");
                cli.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
                cli.printf("Signal Strength: %d dBm\n", WiFi.RSSI());
                cli.printf("Web Interface: http://%s/\n", WiFi.localIP().toString().c_str());
                break;
            case WL_NO_SSID_AVAIL:
                cli.println("SSID not found");
                break;
            case WL_CONNECT_FAILED:
                cli.println("Connection failed (wrong password?)");
                break;
            case WL_IDLE_STATUS:
                cli.println("Idle");
                break;
            case WL_DISCONNECTED:
                cli.println("Disconnected");
                break;
            default:
                cli.printf("Unknown (%d)\n", WiFi.status());
                break;
        }
    } else {
        cli.printf("BLE Name: %s\n", cfg.getBLEName().c_str());
    }

    cli.printf("Display Theme: %d\n", cfg.getDisplayTheme());
    cli.printf("Brightness: %d\n", cfg.getBrightness());
    cli.printf("Server Port: %d\n", cfg.getServerPort());

    AlertThresholds thresh = cfg.getAlertThresholds();
    cli.printf("\nAlert Thresholds:\n");
    cli.printf("  CPU Temp High: %.1f°C\n", thresh.cpuTempHigh);
    cli.printf("  Memory Low: %.1f%%\n", thresh.memoryLow);
    cli.printf("  Disk Low: %.1f%%\n", thresh.diskLow);

    cli.printf("\nFree Heap: %d bytes\n", ESP.getFreeHeap());
}

void CLI::cmdReset(int argc, char* argv[]) {
    CLI& cli = CLI::getInstance();
    cli.println("Resetting configuration to defaults...");
    Config::getInstance().reset();
    cli.println("Configuration reset complete. Restarting...");
    delay(1000);
    ESP.restart();
}
