#include <Arduino.h>
#include <ArduinoJson.h>
#include "LittleFS.h"
#include "FSManager.h"
#include <structures.h>

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

mqttConfig FSManager::configRead() {

  mqttConfig mqttReadedConfig;

  Serial.println("\n Reading config File");
  if (LittleFS.begin()) {
    Serial.println("\n FS Init OK");
    File wifiConfig = LittleFS.open("/wificonfig.json", "r");

    if (wifiConfig) {

      Serial.println("\n config File read ok");
      // Allocate a temporary JsonDocument
      StaticJsonDocument<200> configDoc;

      // Deserialize the JSON document
      DeserializationError error = deserializeJson(configDoc, wifiConfig);
      if (error)
        Serial.println(F("Failed to read file, using default configuration"));

      // Copy values from the JsonDocument to the Config
      strlcpy(mqttReadedConfig.mqtt_server,                  // <- destination
        configDoc["mqtt_broker1"] | "example.com",  // <- source
        sizeof(mqttReadedConfig.mqtt_server));         // <- destination's capacity

      strlcpy(mqttReadedConfig.mqtt_port,                  // <- destination
        configDoc["mqtt_port1"] | "1234",  // <- source
        sizeof(mqttReadedConfig.mqtt_port));         // <- destination's capacity

      strlcpy(mqttReadedConfig.mqtt_id,                  // <- destination
        configDoc["mqtt_id1"] | "thisId",  // <- source
        sizeof(mqttReadedConfig.mqtt_id));         // <- destination's capacity

      strlcpy(mqttReadedConfig.mqtt_user,                  // <- destination
        configDoc["mqtt_user1"] | "thisUser",  // <- source
        sizeof(mqttReadedConfig.mqtt_user));         // <- destination's capacity

      strlcpy(mqttReadedConfig.mqtt_pass,                  // <- destination
        configDoc["mqtt_pass1"] | "thisUser",  // <- source
        sizeof(mqttReadedConfig.mqtt_pass));         // <- destination's capacity
    }
    // Close the file (Curiously, File's destructor doesn't close the file)
    wifiConfig.close();

  }

  return mqttReadedConfig;

}


