#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include "BLEBikeManager.h"
#include "BikeRFIDManager.h"
#include "BikeSensorManager.h"
#include "BikeCANManager.h"
#include "BikeData.h"
#include "BikeMainHardware.h"

// Create instances
BLEBikeManager bleManager;
BikeRFIDManager rfidManager;
BikeSensorManager sensorManager;
BikeCANManager canManager;

// RTOS Task Handles
TaskHandle_t bleTaskHandle = NULL;
TaskHandle_t rfidTaskHandle = NULL;
TaskHandle_t sensorTaskHandle = NULL;
TaskHandle_t systemTaskHandle = NULL;
TaskHandle_t displayTaskHandle = NULL;
TaskHandle_t canTaskHandle = NULL;

// RTOS Synchronization
SemaphoreHandle_t bikeDataMutex;
QueueHandle_t systemEventQueue;

// Shared data instance
SharedBikeData sharedData;

// Master RFID card for bike access
const String MASTER_CARD_UID = "29:0E:72:43"; // Master card always authorized

// =============================================================================
// RTOS TASK FUNCTIONS
// =============================================================================

// Task 1: BLE Communication Task (High Priority - Real-time communication)
void bleTask(void *parameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    Serial.println("[BLE_TASK] Started");
    
    while (true) {
        bleManager.update();
        
        // Update shared BLE connection state
        if (xSemaphoreTake(bikeDataMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            bool wasConnected = sharedData.bleConnected;
            bool currentlyConnected = bleManager.isConnected();
            sharedData.bleConnected = currentlyConnected;
            

            
            // Send event if connection state changed
            if (wasConnected != sharedData.bleConnected) {
                SystemEvent event = sharedData.bleConnected ? EVENT_BLE_CONNECTED : EVENT_BLE_DISCONNECTED;
                xQueueSend(systemEventQueue, &event, 0);
                Serial.printf("[BLE_TASK] Connection change: %s → %s\n", 
                              wasConnected ? "Connected" : "Disconnected",
                              sharedData.bleConnected ? "Connected" : "Disconnected");
            }
            
            xSemaphoreGive(bikeDataMutex);
        }
        
        // High frequency for responsive BLE communication
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(50)); // 20Hz
    }
}

// Task 2: RFID Security Task (Medium Priority - Security critical)
void rfidTask(void *parameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    Serial.println("[RFID_TASK] Started");
    
    while (true) {
        rfidManager.update();
        
        // Update shared RFID state
        if (xSemaphoreTake(bikeDataMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            bool wasUnlocked = sharedData.bikeUnlocked;
            sharedData.bikeUnlocked = rfidManager.isBikeUnlocked();
            
            // Send event if unlock state changed
            if (wasUnlocked != sharedData.bikeUnlocked) {
                SystemEvent event = sharedData.bikeUnlocked ? EVENT_BIKE_UNLOCKED : EVENT_BIKE_LOCKED;
                xQueueSend(systemEventQueue, &event, 0);
            }
            
            xSemaphoreGive(bikeDataMutex);
        }
        
        // Medium frequency for RFID scanning
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100)); // 10Hz
    }
}

// Task 3: Sensor Monitoring Task (Medium Priority - Continuous monitoring)
void sensorTask(void *parameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    Serial.println("[SENSOR_TASK] Started");
    
    while (true) {
        sensorManager.update();
        
        // Update shared sensor data
        if (xSemaphoreTake(bikeDataMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            sharedData.sensorData = sensorManager.getBikeStatus();
            xSemaphoreGive(bikeDataMutex);
        }
        
        // Check for emergency conditions
        BikeStatus currentData = sensorManager.getBikeStatus();
        // if (!currentData.bms1.connected && !currentData.bms2.connected) {
        //     // Both BMS disconnected - emergency!
        //     SystemEvent event = EVENT_EMERGENCY_STOP;
        //     xQueueSend(systemEventQueue, &event, 0);
        // }
        
        // Standard sensor update rate
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(200)); // 5Hz
    }
}

// Task 4: System Control Task (Highest Priority - Main logic controller)
void systemTask(void *parameter) {
    SystemEvent receivedEvent;
    
    Serial.println("[SYSTEM_TASK] Started");
    
    while (true) {
        // Process system events
        if (xQueueReceive(systemEventQueue, &receivedEvent, pdMS_TO_TICKS(100)) == pdTRUE) {
            Serial.printf("[SYSTEM_TASK] Processing event: %d\n", receivedEvent);
            
            switch (receivedEvent) {
                case EVENT_BIKE_UNLOCKED:
                    Serial.println("[SYSTEM] 🔓 Bike UNLOCKED - System ACTIVE");
                    sensorManager.setBikeKeyState(true);
                    break;
                    
                case EVENT_BIKE_LOCKED:
                    Serial.println("[SYSTEM] 🔒 Bike LOCKED - System STANDBY");
                    sensorManager.setBikeKeyState(false);
                    break;
                    
                case EVENT_BLE_CONNECTED:
                    Serial.println("[SYSTEM] 📱 BLE Connected - Remote access enabled");
                    break;
                    
                case EVENT_BLE_DISCONNECTED:
                    Serial.println("[SYSTEM] 📱 BLE Disconnected - Local mode only");
                    break;
                    
                case EVENT_EMERGENCY_STOP:
                    Serial.println("[SYSTEM] 🚨 EMERGENCY STOP - All systems halt");
                    // Emergency procedures - all critical systems disabled
                    break;
                    
                default:
                    Serial.printf("[SYSTEM] Unknown event: %d\n", receivedEvent);
                    break;
            }
        }
        
        // Update system state
        if (xSemaphoreTake(bikeDataMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            // Determine overall system state
            if (sharedData.bikeUnlocked) {
                sharedData.currentState = BIKE_ON;
            } else {
                sharedData.currentState = BIKE_LOCKED;
            }
            
            // Update BLE with current state
            bleManager.setBikeStatus(sharedData.currentState);
            
            xSemaphoreGive(bikeDataMutex);
        }
        
        // System control runs at moderate frequency
        vTaskDelay(pdMS_TO_TICKS(50)); // 20Hz
    }
}

// Task 5: CAN Communication Task (Medium Priority - Display communication)
void canTask(void *parameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    Serial.println("[CAN_TASK] Started");
    
    while (true) {
        // Send data in sequence every 500ms
        if (xSemaphoreTake(bikeDataMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            // Use library's automatic sequence sending
            canManager.sendNextInSequence(sharedData);
            xSemaphoreGive(bikeDataMutex);
        }
        
        // Handle incoming messages
        canManager.update();
        
        // CAN task runs at 2Hz (500ms interval)
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
    }
}

// Task 6: Display/Logging Task (Low Priority - Non-critical output)
void displayTask(void *parameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    Serial.println("[DISPLAY_TASK] Started");
    
    while (true) {
        // Display system status every 5 seconds
        if (xSemaphoreTake(bikeDataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            Serial.println("\n=== 🚲 SMART BIKE SYSTEM STATUS ===");
            
            // BLE Status
            Serial.printf("📡 BLE: %s", sharedData.bleConnected ? "Connected" : "Disconnected");
            if (bleManager.isPairingInProgress()) {
                Serial.print(" (PAIRING - PRESS BOOT!)");
            }
            Serial.printf(" | Bonded: %d\n", bleManager.getBondedDeviceCount());
            
            // RFID & Bike Status  
            Serial.printf("🔐 Bike: %s | Key Output: %s\n", 
                         sharedData.bikeUnlocked ? "UNLOCKED" : "LOCKED",
                         sharedData.sensorData.keyOn ? "HIGH" : "LOW");
            
            // Speed & Hall Status
            Serial.printf("🏁 Speed: %.1f km/h | Hall: %.1f Hz | Pulses: %lu\n",
                         sharedData.sensorData.bikeSpeed,
                         sharedData.sensorData.hallFrequency,
                         sensorManager.getHallPulseCount());
            
            // BMS Status
            Serial.printf("🔋 BMS1: %s", sharedData.sensorData.bms1.connected ? "OK" : "FAIL");
            if (sharedData.sensorData.bms1.connected) {
                Serial.printf(" %.2fV %.1fA %d%% %.1f°C Δ%dmV", 
                             sharedData.sensorData.bms1.voltage, sharedData.sensorData.bms1.current, 
                             sharedData.sensorData.bms1.soc, sharedData.sensorData.bms1.temperature,
                             sharedData.sensorData.bms1.cellVoltageDelta);
            }
            Serial.println();
            
            Serial.printf("🔋 BMS2: %s", sharedData.sensorData.bms2.connected ? "OK" : "FAIL");
            if (sharedData.sensorData.bms2.connected) {
                Serial.printf(" %.2fV %.1fA %d%% %.1f°C Δ%dmV", 
                             sharedData.sensorData.bms2.voltage, sharedData.sensorData.bms2.current, 
                             sharedData.sensorData.bms2.soc, sharedData.sensorData.bms2.temperature,
                             sharedData.sensorData.bms2.cellVoltageDelta);
            }
            Serial.println();
            
            // Task Status
            Serial.printf("⚙️  Tasks: BLE=%d RFID=%d SENSOR=%d SYSTEM=%d CAN=%d DISPLAY=%d\n",
                         uxTaskPriorityGet(bleTaskHandle),
                         uxTaskPriorityGet(rfidTaskHandle), 
                         uxTaskPriorityGet(sensorTaskHandle),
                         uxTaskPriorityGet(systemTaskHandle),
                         uxTaskPriorityGet(canTaskHandle),
                         uxTaskPriorityGet(displayTaskHandle));
                         
            Serial.printf("💾 Free Heap: %d bytes\n", ESP.getFreeHeap());
            Serial.println("=====================================");
            
            xSemaphoreGive(bikeDataMutex);
        }
        
        // Low frequency for display updates
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(5000)); // Every 5 seconds
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== 🚲 SAO KIM SMART BIKE SYSTEM ===");
    Serial.println("🔧 Initializing RTOS Multi-Task System...");
    
    // Initialize RTOS synchronization objects
    bikeDataMutex = xSemaphoreCreateMutex();
    systemEventQueue = xQueueCreate(10, sizeof(SystemEvent));
    
    if (bikeDataMutex == NULL || systemEventQueue == NULL) {
        Serial.println("❌ Failed to create RTOS synchronization objects!");
        ESP.restart();
    }
    
    // Initialize shared data
    memset(&sharedData, 0, sizeof(SharedBikeData));
    sharedData.currentState = BIKE_OFF;
    
    Serial.println("\n🔧 1. Initializing BLE System...");
    bleManager.begin();
    bleManager.setBikeStatus(BIKE_OFF);
    
    Serial.println("\n🔐 2. Initializing RFID System...");
    rfidManager.begin();
    
    // Link RFID Manager to BLE Manager for authentication
    bleManager.setRFIDManager(&rfidManager);
    
    // Set master card first
    Serial.println("Setting master card...");
    rfidManager.setMasterCard(MASTER_CARD_UID);
    
    // Clear all previous cards and add fresh
    Serial.println("Clearing previous cards...");
    rfidManager.clearAllCards();
    delay(100); // Small delay for flash write
    
    Serial.println("Adding authorized card...");
    rfidManager.addAuthorizedCard(MASTER_CARD_UID);
    Serial.printf("Master card added: %s\n", MASTER_CARD_UID.c_str());
    
    Serial.println("\n📊 3. Initializing Sensor System...");
    sensorManager.begin();
    
    Serial.println("\n🔗 4. Initializing CAN Bus...");
    if (!canManager.begin(MAIN_CAN_TX, MAIN_CAN_RX)) {
        Serial.println("⚠️  System will continue without CAN communication");
    }
    
    Serial.println("\n⚙️  5. Creating RTOS Tasks...");
    
    // Create tasks with appropriate priorities
    xTaskCreatePinnedToCore(
        systemTask,         // Task function
        "SystemTask",       // Task name
        4096,              // Stack size
        NULL,              // Parameters
        5,                 // Priority (Highest)
        &systemTaskHandle, // Task handle
        1                  // Core 1
    );
    
    xTaskCreatePinnedToCore(
        bleTask,           // Task function
        "BLETask",         // Task name
        4096,              // Stack size
        NULL,              // Parameters
        4,                 // Priority (High)
        &bleTaskHandle,    // Task handle
        0                  // Core 0 (WiFi/BLE core)
    );
    
    xTaskCreatePinnedToCore(
        rfidTask,          // Task function
        "RFIDTask",        // Task name
        3072,              // Stack size
        NULL,              // Parameters
        3,                 // Priority (Medium-High)
        &rfidTaskHandle,   // Task handle
        1                  // Core 1
    );
    
    xTaskCreatePinnedToCore(
        sensorTask,        // Task function
        "SensorTask",      // Task name
        4096,              // Stack size
        NULL,              // Parameters
        3,                 // Priority (Medium)
        &sensorTaskHandle, // Task handle
        1                  // Core 1
    );
    
    xTaskCreatePinnedToCore(
        canTask,           // Task function
        "CANTask",         // Task name
        3072,              // Stack size
        NULL,              // Parameters
        2,                 // Priority (Medium-Low)
        &canTaskHandle,    // Task handle
        0                  // Core 0 (same as Display)
    );
    
    xTaskCreatePinnedToCore(
        displayTask,       // Task function
        "DisplayTask",     // Task name
        3072,              // Stack size
        NULL,              // Parameters
        1,                 // Priority (Low)
        &displayTaskHandle,// Task handle
        0                  // Core 0
    );
    
    Serial.println("\n✅ === RTOS SYSTEM READY ===");
    Serial.println("📋 Task Distribution:");
    Serial.println("   🎯 Core 0: BLE + CAN + Display");
    Serial.println("   🎯 Core 1: System + RFID + Sensors");
    Serial.println("🔧 Instructions:");
    Serial.println("   - Use RFID card to lock/unlock bike");
    Serial.println("   - BLE: Press BOOT for new devices");
    Serial.println("   - CAN: Sends bike status to display board");
    Serial.println("   - All sensors monitoring active");
    Serial.println("   - Real-time RTOS task management");
    Serial.println("=================================");
    
    // Delete Arduino loop task - we use our own RTOS tasks
    vTaskDelete(NULL);
}

void loop() {
    // This loop is deleted by vTaskDelete(NULL) in setup()
    // All functionality is handled by RTOS tasks
}
