#ifndef UART_PROTOCOL_H
#define UART_PROTOCOL_H

#include <Arduino.h>
#include <map>
#include <functional>

class UARTProtocol {
public:
    using CommandHandler = std::function<String(const String&)>;

    UARTProtocol();
    void init(unsigned long baud_rate = 115200);
    void handle();
    void registerCommand(const String& command, CommandHandler handler);

private:
    static const int BUFFER_SIZE = 64;
    char _cmdBuffer[BUFFER_SIZE];
    int _cmdIndex;
    std::map<String, CommandHandler> _commandHandlers;

    void processCommand();
    void sendResponse(const String& response);
};

extern UARTProtocol uartProtocol;

#endif // UART_PROTOCOL_H