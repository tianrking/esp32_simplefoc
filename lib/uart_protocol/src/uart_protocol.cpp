#include "uart_protocol.h"
#include "simplefoc_control.h"

UARTProtocol uartProtocol;

UARTProtocol::UARTProtocol() : _cmdIndex(0) {}

void UARTProtocol::init(unsigned long baud_rate) {
    Serial.begin(baud_rate);
}

void UARTProtocol::handle() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            if (_cmdIndex > 0) {
                _cmdBuffer[_cmdIndex] = '\0';
                processCommand();
                _cmdIndex = 0;
            }
        } else if (_cmdIndex < BUFFER_SIZE - 1) {
            _cmdBuffer[_cmdIndex++] = c;
        }
    }
}

void UARTProtocol::registerCommand(const String& command, CommandHandler handler) {
    _commandHandlers[command] = handler;
}

void UARTProtocol::processCommand() {
    String cmdString = String(_cmdBuffer);
    if (!cmdString.startsWith("AT+")) {
        sendResponse("ERROR: Invalid command format");
        return;
    }
    
    int equalIndex = cmdString.indexOf('=');
    String command = equalIndex == -1 ? cmdString : cmdString.substring(0, equalIndex);
    String params = equalIndex == -1 ? "" : cmdString.substring(equalIndex + 1);
    
    auto it = _commandHandlers.find(command);
    sendResponse(cmdString);
    if (it != _commandHandlers.end()) {
        String response = it->second(params);
        sendResponse(response);
    } else {
        sendResponse("ERROR: Unknown command");
    }
}

void UARTProtocol::sendResponse(const String& response) {
    Serial.println(response);
}