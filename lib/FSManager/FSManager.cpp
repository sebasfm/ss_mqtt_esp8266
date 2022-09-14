#include <Arduino.h>
#include "LittleFS.h"
#include "FSManager.h"

bool FSManager::init() {

  if (!LittleFS.begin()) {
    Serial.println("\n An Error has occurred while mounting LittleFS");
    return false;
  }

  File wifiConfig = LittleFS.open("/wificonfig.json", "r");
  if (!wifiConfig) {
    Serial.println("\n ailed to open file for reading");
    return false;
  }

  Serial.println("\n File Content:");
  while (wifiConfig.available()) {
    Serial.write(wifiConfig.read());
  }
  wifiConfig.close();
  return true;
}


