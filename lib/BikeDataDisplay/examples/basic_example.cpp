/*
 * BikeDataDisplay Library - Basic Example
 * This example demonstrates basic usage of the BikeDataDisplay library
 */

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <lvgl.h>
#include <BikeDataDisplay.h>

BikeDataDisplay dashboard;
BikeData bike;

void setup() {
  Serial.begin(115200);
  
  // Initialize display and LVGL (implementation depends on your setup)
  // ...
  
  // Initialize dashboard
  dashboard.initialize();
  
  Serial.println("BikeDataDisplay Library Example Ready!");
}

void loop() {
  // Update bike data
  bike.speed = 45.5;
  bike.current = 3.2;
  bike.batteryPercent = 75;
  
  // Update dashboard
  dashboard.updateAll(bike);
  
  delay(100);
}