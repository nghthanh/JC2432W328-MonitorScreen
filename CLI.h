#ifndef CLI_H
#define CLI_H

#include <Arduino.h>
#include <vector>

#define MAX_CMD_ARGS 10
#define CMD_BUFFER_SIZE 128

// Command handler function pointer type
typedef void (*CommandHandler)(int argc, char* argv[]);

// Command structure
struct Command {
    const char* name;
    const char* description;
    CommandHandler handler;
};

class CLI {
public:
    static CLI& getInstance();

    void begin(unsigned long baudRate = 115200);
    void update();

    // Register a new command
    void registerCommand(const char* name, const char* description, CommandHandler handler);

    // Utility functions for commands
    void print(const char* str);
    void println(const char* str);
    void printf(const char* format, ...);

private:
    CLI();

    std::vector<Command> commands;
    char cmdBuffer[CMD_BUFFER_SIZE];
    uint8_t cmdIndex;

    void processCommand(char* cmdLine);
    void executeCommand(int argc, char* argv[]);
    void showHelp();

    // Built-in commands
    static void cmdHelp(int argc, char* argv[]);
    static void cmdStatus(int argc, char* argv[]);
    static void cmdReset(int argc, char* argv[]);
};

#endif
