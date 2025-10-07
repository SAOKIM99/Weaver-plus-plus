/*
 * CAN Communication Test - MAIN BOARD
 * 
 * Board chính gửi "hello world" qua CAN bus
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

// Relay Key Configuration
#define RELAY_KEY_PIN  13     // GPIO13 for relay key
#define KEY_PULSE_TIME 100   // 100ms pulse

// Message ID
#define MSG_ID_HELLO   0x123

// Timing
unsigned long lastSend = 0;
const unsigned long SEND_INTERVAL = 1000;  // 1 second

// Counter
uint32_t messageCount = 0;

// Function prototypes
void sendHelloWorld();
void listenForResponses();

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("========================================");
    Serial.println("    CAN Communication Test - MAIN");
    Serial.println("========================================");
    
    // Initialize Relay Key pin
    pinMode(RELAY_KEY_PIN, OUTPUT);
    digitalWrite(RELAY_KEY_PIN, HIGH);  // Ensure relay is OFF initially
    Serial.printf("Relay Key Pin: %d initialized\n", RELAY_KEY_PIN);
    
    
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
    
    Serial.println("Sending 'hello' every 1 second...");
    Serial.println();
}

void loop() {
    unsigned long currentTime = millis();
    
    // Send "hello world" every second
    if (currentTime - lastSend >= SEND_INTERVAL) {
        sendHelloWorld();
        lastSend = currentTime;
    }
    
    // Listen for responses
    listenForResponses();
    
    delay(10);
}

void sendHelloWorld() {
    String message = "hello";  // Shortened to fit in CAN frame (8 bytes max)
    
    CAN.beginPacket(MSG_ID_HELLO);
    
    // Send each character of "hello"
    for (int i = 0; i < message.length() && i < 8; i++) {
        CAN.write(message[i]);
    }
    
    if (CAN.endPacket()) {
        messageCount++;
        Serial.printf("[0x%03X] TX -> \"%s\" (Count: %ld)\n", 
                      MSG_ID_HELLO, message.c_str(), messageCount);
    } else {
        Serial.println("❌ Failed to send message");
    }
}

void listenForResponses() {
    int packetSize = CAN.parsePacket();
    
    if (packetSize > 0) {
        uint32_t id = CAN.packetId();
        
        Serial.printf("📨 RX [0x%03X] ", id);
        
        // Read message as string
        String receivedMessage = "";
        while (CAN.available()) {
            char c = CAN.read();
            receivedMessage += c;
        }
        
        Serial.printf("\"%s\" (%d bytes)\n", receivedMessage.c_str(), packetSize);
    }
}