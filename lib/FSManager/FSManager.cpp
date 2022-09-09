#include <Arduino.h>
#include "LittleFS.h"
#include "FSManager.h"

bool FSManager::init() {

  if (!LittleFS.begin()) {
    Serial.println("An Error has occurred while mounting LittleFS");
    return false;
  }

  File wifiConfig = LittleFS.open("/wificonfig.json", "r");
  if (!wifiConfig) {
    Serial.println("Failed to open file for reading");
    return false;
  }

  Serial.println("File Content:");
  while (wifiConfig.available()) {
    Serial.write(wifiConfig.read());
  }
  wifiConfig.close();
  return true;
}


