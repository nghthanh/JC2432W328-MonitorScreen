#ifndef CLI_COMMANDS_H
#define CLI_COMMANDS_H

#include "CLI.h"

// Register all custom commands
void registerCLICommands();

// WiFi commands
void cmdSetWiFi(int argc, char* argv[]);
void cmdScanWiFi(int argc, char* argv[]);
void cmdSelectWiFi(int argc, char* argv[]);
void cmdSetInterface(int argc, char* argv[]);

// BLE commands
void cmdSetBLEName(int argc, char* argv[]);

// mDNS commands
void cmdSetMDNSName(int argc, char* argv[]);

// Display commands
void cmdSetTheme(int argc, char* argv[]);
void cmdSetBrightness(int argc, char* argv[]);

// Alert commands
void cmdSetAlert(int argc, char* argv[]);

// Server commands
void cmdSetPort(int argc, char* argv[]);

// Idle timeout commands
void cmdSetIdleTimeout(int argc, char* argv[]);

// Date/Time commands
void cmdSetDateTime(int argc, char* argv[]);
void cmdGetDateTime(int argc, char* argv[]);

// NTP commands
void cmdSyncNTP(int argc, char* argv[]);
void cmdSetNTPServer(int argc, char* argv[]);
void cmdSetTimezone(int argc, char* argv[]);

#endif
