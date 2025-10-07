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

// Display commands
void cmdSetTheme(int argc, char* argv[]);
void cmdSetBrightness(int argc, char* argv[]);

// Alert commands
void cmdSetAlert(int argc, char* argv[]);

// Server commands
void cmdSetPort(int argc, char* argv[]);

#endif
