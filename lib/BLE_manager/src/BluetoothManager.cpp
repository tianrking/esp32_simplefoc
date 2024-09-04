// #include "BluetoothManager.h"

// #define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
// #define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// BLEManager bleManager;

// class MyServerCallbacks: public BLEServerCallbacks {
//     void onConnect(BLEServer* pServer) {
//       Serial.println("Device connected");
//       bleManager.deviceConnected = true;
//     }

//     void onDisconnect(BLEServer* pServer) {
//       Serial.println("Device disconnected");
//       bleManager.deviceConnected = false;
//       pServer->startAdvertising();
//     }
// };

// class MyCharacteristicCallbacks: public BLECharacteristicCallbacks {
//     void onWrite(BLECharacteristic *pCharacteristic) {
//         std::string value = pCharacteristic->getValue();
//         if (value.length() > 0) {
//             Serial.println("Received Value: ");
//             for (int i = 0; i < value.length(); i++) {
//                 Serial.print(value[i]);
//             }
//             Serial.println();
//             bleManager.receivedMessage = value;
//         }
//     }
// };

// BLEManager::BLEManager() : deviceConnected(false) {}

// void BLEManager::init(const char* deviceName) {
//     BLEDevice::init(deviceName);
//     pServer = BLEDevice::createServer();
//     pServer->setCallbacks(new MyServerCallbacks());
//     pService = pServer->createService(SERVICE_UUID);
//     pCharacteristic = pService->createCharacteristic(
//                         CHARACTERISTIC_UUID,
//                         BLECharacteristic::PROPERTY_READ |
//                         BLECharacteristic::PROPERTY_WRITE |
//                         BLECharacteristic::PROPERTY_NOTIFY
//                       );

//     pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

//     pCharacteristic->setValue("Hello World");
//     pService->start();
//     pAdvertising = BLEDevice::getAdvertising();
//     pAdvertising->addServiceUUID(SERVICE_UUID);
//     pAdvertising->setScanResponse(true);
//     pAdvertising->setMinPreferred(0x06);  
//     pAdvertising->setMinPreferred(0x12);
//     BLEDevice::startAdvertising();
//     Serial.println("BLE device initialized and advertising!");
    
//     printDeviceAddress();
// }

// void BLEManager::handle() {
//     if (!receivedMessage.empty()) {
//         processCommand(String(receivedMessage.c_str()));
//         receivedMessage.clear();
//     }
// }

// bool BLEManager::isConnected() {
//     return deviceConnected;
// }

// void BLEManager::sendMessage(const String& message) {
//     if (deviceConnected) {
//         pCharacteristic->setValue(message.c_str());
//         pCharacteristic->notify();
//     }
// }

// String BLEManager::receiveMessage() {
//     String message = receivedMessage.c_str();
//     receivedMessage.clear();
//     return message;
// }

// void BLEManager::printDeviceAddress() {
//     const uint8_t* point = esp_bt_dev_get_address();
//     if (point != nullptr) {
//         Serial.print("ESP32 BLE MAC: ");
//         for (int i = 0; i < 6; i++) {
//             char str[3];
//             sprintf(str, "%02X", (int)point[i]);
//             Serial.print(str);
//             if (i < 5) {
//                 Serial.print(":");
//             }
//         }
//         Serial.println();
//     } else {
//         Serial.println("Failed to get BLE MAC address");
//     }
// }

// void BLEManager::registerCommand(const String& command, CommandHandler handler) {
//     _commandHandlers[command] = handler;
// }

// void BLEManager::processCommand(const String& commandString) {
//     int equalIndex = commandString.indexOf('=');
//     String command = equalIndex == -1 ? commandString : commandString.substring(0, equalIndex);
//     String params = equalIndex == -1 ? "" : commandString.substring(equalIndex + 1);

//     auto it = _commandHandlers.find(command);
//     if (it != _commandHandlers.end()) {
//         String response = it->second(params);
//         sendResponse(response);
//     } else {
//         sendResponse("ERROR: Unknown command");
//     }
// }

// void BLEManager::sendResponse(const String& response) {
//     pCharacteristic->setValue(response.c_str());
//     pCharacteristic->notify();
// }