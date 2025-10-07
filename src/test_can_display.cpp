/*
 * CAN Communication Test - DISPLAY BOARD
 * 
 * Board display nhận "hello world" từ CAN bus và hiển thị
 * Sử dụng IC VP230 CAN transceiver
 * 
 * Kết nối:
 * ESP32 GPIO25 (CAN_TX) -> VP230 TXD
 * ESP32 GPIO26 (CAN_RX) -> VP230 RXD
 * VP230 CANH/CANL -> CAN Bus
 * 
 * Author: SAOKIM99
 * Date: 2025
 */

#include <Arduino.h>
#include <CAN.h>

// CAN Configuration
#define CAN_TX_PIN    25
#define CAN_RX_PIN    26
#define CAN_SPEED     500E3  // 500 kbps

// Message IDs
#define MSG_ID_HELLO   0x123
#define MSG_ID_ACK     0x456

// Statistics
uint32_t messagesReceived = 0;
unsigned long lastAckSent = 0;
const unsigned long ACK_INTERVAL = 2000;  // Send ACK every 2 seconds

// Function prototypes
void listenForMessages();
void sendAcknowledgment();

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("========================================");
    Serial.println("   CAN Communication Test - DISPLAY");
    Serial.println("========================================");
    
    // Initialize CAN bus
    CAN.setPins(CAN_RX_PIN, CAN_TX_PIN);
    
    if (!CAN.begin(CAN_SPEED)) {
        Serial.println("❌ CAN initialization failed!");
        while (1) {
            delay(1000);
            Serial.println("Retrying CAN initialization...");
            if (CAN.begin(CAN_SPEED)) break;
        }
    }
    
    Serial.println("✅ CAN initialized successfully");
    Serial.printf("CAN Speed: %.0f kbps\n", CAN_SPEED / 1000.0f);
    Serial.printf("TX Pin: %d, RX Pin: %d\n", CAN_TX_PIN, CAN_RX_PIN);
    Serial.println();
    
    Serial.println("Listening for 'hello' messages...");
    Serial.println();
}

void loop() {
    unsigned long currentTime = millis();
    
    // Listen for CAN messages
    listenForMessages();
    
    // Send acknowledgment periodically
    if (currentTime - lastAckSent >= ACK_INTERVAL) {
        sendAcknowledgment();
        lastAckSent = currentTime;
    }
    
    delay(10);
}

void listenForMessages() {
    int packetSize = CAN.parsePacket();
    
    if (packetSize > 0) {
        uint32_t id = CAN.packetId();
        
        if (id == MSG_ID_HELLO) {
            // Read message as string
            String receivedMessage = "";
            while (CAN.available()) {
                char c = CAN.read();
                receivedMessage += c;
            }
            
            messagesReceived++;
            Serial.printf("📨 [0x%03X] RX -> \"%s\" (Count: %ld)\n", 
                          id, receivedMessage.c_str(), messagesReceived);
            
            // Check if it's "hello"
            if (receivedMessage == "hello") {
                Serial.println("   ✅ Correct message received!");
            } else {
                Serial.println("   ❌ Unexpected message!");
            }
        } else {
            Serial.printf("📨 [0x%03X] UNKNOWN <- %d bytes\n", id, packetSize);
        }
    }
}

void sendAcknowledgment() {
    String ackMessage = "ACK";
    
    CAN.beginPacket(MSG_ID_ACK);
    
    // Send "ACK" message
    for (int i = 0; i < ackMessage.length(); i++) {
        CAN.write(ackMessage[i]);
    }
    
    if (CAN.endPacket()) {
        Serial.printf("📤 [0x%03X] TX -> \"%s\" (Total received: %ld)\n", 
                      MSG_ID_ACK, ackMessage.c_str(), messagesReceived);
    } else {
        Serial.println("❌ Failed to send ACK");
    }
}