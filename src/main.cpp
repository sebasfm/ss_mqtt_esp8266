#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <FSManager.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "structures.h"

//#define mqtt_server "your.broker.url"
//#define mqtt_port 1883
//#define client_id "clientid"
//#define mqtt_user "user"
//#define mqtt_password "pass"

//#define mqtt_server "node02.myqtthub.com"
//#define mqtt_port 1883
//#define client_id "node0001"
//#define mqtt_user "node0001"
//#define mqtt_password "thisNodePass22"

#define in_topic "/light/in"
#define out_topic "/light/out"
// Replace by 2 if you aren't enable to use Serial Monitor... Don't forget to Rewire R1 to GPIO2!
#define in_led 2
#define TRIGGER_PIN 0

unsigned long myTime;

bool wm_nonblocking = false; // change to true to use non blocking
bool portalRunning = false;

mqttConfig mqttConf;

FSManager FSM;

WiFiManager wm; // global wm instance

// WiFiManagerParameter custom_field; // global param ( for non blocking w params )
WiFiManagerParameter custom_mqtt_server("server", "mqtt server", "", 40);
WiFiManagerParameter custom_mqtt_port("port", "mqtt port", "", 6);
WiFiManagerParameter custom_mqtt_user("mqttUser", "mqtt user", "", 32);
WiFiManagerParameter custom_mqtt_pass("mqttPass", "mqtt pass ", "", 32);

WiFiClient espClient;
PubSubClient client;

String getParam(String name)
{
  // read parameter from server, for customhmtl input
  String value;
  if (wm.server->hasArg(name))
  {
    value = wm.server->arg(name);
  }
  return value;
}

void saveParamCallback()
{
  Serial.println("[CALLBACK] saveParamCallback fired");
  Serial.println("PARAM mqtt server : " + getParam("server"));
  Serial.println("PARAM mqtt server : " + getParam("port"));
  Serial.println("PARAM mqtt server : " + getParam("mqttUser"));
  Serial.println("PARAM mqtt server : " + getParam("mqttPass"));
}

void checkButton()
{
  // is auto timeout portal running
  if (portalRunning)
  {
    wm.process();
  }

  // is configuration portal requested?
  if (digitalRead(TRIGGER_PIN) == LOW)
  {
    delay(50);
    if (digitalRead(TRIGGER_PIN) == LOW)
    {
      if (!portalRunning)
      {
        Serial.println("Button Pressed, Starting Portal");
        wm.startWebPortal();
        portalRunning = true;
      }
      else
      {
        Serial.println("Button Pressed, Stopping Portal");
        wm.stopWebPortal();
        portalRunning = false;
      }
    }
  }
}

void setup_wifi()
{
  delay(10);

  // We start by connecting to a WiFi network
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  delay(3000);
  Serial.println("\n Starting");
  pinMode(TRIGGER_PIN, INPUT_PULLUP);

  // wm.resetSettings(); // wipe settings

  if (wm_nonblocking)
    wm.setConfigPortalBlocking(false);

  // parameters

  // add a custom input field
  // int customFieldLength = 40;

  // test custom html(radio)
  // const char *custom_radio_str = "<br/><label for='customfieldid'>Custom Field Label</label><input type='radio' name='customfieldid' value='1' checked> One<br><input type='radio' name='customfieldid' value='2'> Two<br><input type='radio' name='customfieldid' value='3'> Three";
  // new (&custom_field) WiFiManagerParameter(custom_radio_str); // custom html input

  // wm.addParameter(&custom_field);


  //set the values to show in portal
  custom_mqtt_server.setValue(mqttConf.mqtt_server, 40);
  custom_mqtt_port.setValue(mqttConf.mqtt_port, 6);
  custom_mqtt_user.setValue(mqttConf.mqtt_user, 40);
  custom_mqtt_pass.setValue(mqttConf.mqtt_pass, 40);

  wm.addParameter(&custom_mqtt_server);
  wm.addParameter(&custom_mqtt_port);
  wm.addParameter(&custom_mqtt_user);
  wm.addParameter(&custom_mqtt_pass);

  wm.setSaveParamsCallback(saveParamCallback);

  std::vector<const char*> menu = { "wifi", "param", "sep", "restart", "exit" };
  wm.setMenu(menu);

  // set dark theme
  wm.setClass("invert");

  wm.setConfigPortalTimeout(180); // auto close configportal after n seconds

  bool res;
  res = wm.autoConnect("AutoConnectAP", "password"); // password protected ap

  if (!res)
  {
    Serial.println("Failed to connect or hit timeout");
    // ESP.restart();
  }
  else
  {
    // if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
  }
}

void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++)
  {
    char receivedChar = (char)payload[i];
    Serial.print(receivedChar);
    if (receivedChar == '0')
      digitalWrite(in_led, LOW);
    if (receivedChar == '1')
      digitalWrite(in_led, HIGH);
  }
  Serial.println();
}

void setup()
{
  Serial.begin(115200);
  delay(3000);
  // Start FS Manager
  if (FSM.init()) {
    Serial.println("\n Filesystem Started Ok.");
    mqttConf = FSM.configRead();
  }

/*   strcpy(mqttConf.mqtt_server, "A_verGaston");
  strcpy(mqttConf.mqtt_port, "1234");
  strcpy(mqttConf.mqtt_id, "1234");
  strcpy(mqttConf.mqtt_user, "user1234");
  strcpy(mqttConf.mqtt_pass, "pass1234"); */

  setup_wifi();
  client.setClient(espClient);
  client.setServer(mqttConf.mqtt_server, atoi(mqttConf.mqtt_port));
  client.setCallback(callback);

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(in_led, OUTPUT);
  digitalWrite(in_led, HIGH);
}

void reconnect()
{
  // Loop until we're reconnected
  if (!client.connected() && (millis() - myTime) > 20000)
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect(mqttConf.mqtt_id, mqttConf.mqtt_user, mqttConf.mqtt_pass))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 20 seconds");
      // Wait 5 seconds before retrying
      myTime = millis();
    }
  }
}

void loop()
{

  checkButton();

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  // Publishes a random 0 and 1 like someone switching off and on randomly (random(2))
  // client.publish(out_topic, String(random(2)).c_str(), true);
  // delay(1000);
  client.subscribe(in_topic);
  delay(1000);
}