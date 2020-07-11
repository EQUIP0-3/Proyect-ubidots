
//Este código implementadi fue basado en el código que se encuentra en https://github.com/techiesms/ESP32-MQTT y 
//https://github.com/mjScientech/Esp32-Ubidots-Wireless-long-range-Temperature-And-Humidity
/****************************************
   Include Libraries
 ****************************************/
#include <WiFi.h>
#include <PubSubClient.h>

#define WIFISSID "INFINITUMt3ye" // Put your WifiSSID here
#define PASSWORD "GH6pke018v" // Put your wifi password here
#define TOKEN "BBFF-ynvW5cCnI7oNXZSuqiBXYhVNtVBIDz" // Put your Ubidots' TOKEN
#define MQTT_CLIENT_NAME "RandomnP" // MQTT client Name, please enter your own 8-12 alphanumeric character ASCII string; 
//it should be a random and unique ascii string and different from all other devices

/****************************************
   Define Constants
 ****************************************/
#define VARIABLE_LABEL "potenciometro" // Assing the variable label
#define VARIABLE_LED "led"
#define DEVICE_LABEL "esp32" // Assig the device label

#define led 2 // se utilizo el led que viene integrado en el ESP32
#define pot 39

char mqttBroker[]  = "industrial.api.ubidots.com"; 
char payload[100];
char topic[150];
char topicSubscribe[100];//HERE
// Space to store values to send
char str_sensor[10];

/****************************************
   Auxiliar Functions
 ****************************************/
WiFiClient ubidots;
PubSubClient client(ubidots);

void callback(char* topic, byte* payload, unsigned int length) {
  //controlar led
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  String message(p);
  Serial.println("message = " + message);
  if (message == "0") { 
    digitalWrite(led, LOW);
  } else {
    digitalWrite(led, HIGH);
  }

  Serial.write(payload, length);
  Serial.println(topic);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");

    // Attemp to connect
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
      Serial.println("Connected");
      client.subscribe(topicSubscribe); //HERE
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}

/****************************************
   Main Functions
 ****************************************/
void setup() {
  Serial.begin(115200);
  delay(3000);
  
  pinMode(led, OUTPUT);

 
  Serial.println();
  WiFi.begin(WIFISSID, PASSWORD);
  Serial.print("Warten WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("!");
  Serial.println("WiFi Verbuden");
  Serial.print("IP Adresse: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqttBroker, 1883);
  client.setCallback(callback);
  sprintf(topicSubscribe, "/v1.6/devices/%s/%s/lv", DEVICE_LABEL, VARIABLE_LED);
  client.subscribe(topicSubscribe);
}


void loop() {
  
  
  if (!client.connected()) {
    client.subscribe(topicSubscribe);
    reconnect();
  }

  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload, "{\"%s\":", VARIABLE_LABEL); // Adds the variable label

  float sensor = analogRead(pot);
  Serial.print("Valor del potenciometro:"); Serial.println(sensor);
  
  //Se convierten los valores obtenidos del potenciometro a voltaje 0 - 5 V
  
  float voltaje= (sensor * 5)/4095;
  Serial.println(voltaje);
  delay(1000);

  dtostrf(voltaje, 4, 2, str_sensor);

  sprintf(payload, "%s {\"value\": %s}}", payload, str_sensor); // Adds the value
  Serial.println("Publishing data to Ubidots Cloud");
  client.publish(topic, payload);
  client.loop();
  delay(1000);
}
