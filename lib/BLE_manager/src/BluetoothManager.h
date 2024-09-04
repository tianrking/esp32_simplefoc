// #ifndef BLUETOOTH_MANAGER_H
// #define BLUETOOTH_MANAGER_H

// #include <Arduino.h>
// #include <BLEDevice.h>
// #include <BLEUtils.h>
// #include <BLEServer.h>
// #include <map>
// #include <functional>





// class BLEManager {
// public:
//     using CommandHandler = std::function<String(const String&)>;

//     BLEManager();
//     void init(const char* deviceName);
//     void handle();
//     bool isConnected();
//     void sendMessage(const String& message);
//     String receiveMessage();
//     void registerCommand(const String& command, CommandHandler handler);

// private:
//     BLEServer* pServer;
//     BLEService* pService;
//     BLECharacteristic* pCharacteristic;
//     BLEAdvertising* pAdvertising;
//     bool deviceConnected;
//     std::string receivedMessage;
//     std::map<String, CommandHandler> _commandHandlers;

//     void printDeviceAddress();
//     void processCommand(const String& commandString);
//     void sendResponse(const String& response);
// };

// extern BLEManager bleManager;

// #endif // BLUETOOTH_MANAGER_H