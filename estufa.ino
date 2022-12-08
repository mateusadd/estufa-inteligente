#include <DHT.h>           // DHT for Library library
#include <WiFi.h>           // WiFi control for ESP32
#include <ThingsBoard.h>    // ThingsBoard SDK

#define DHTPIN 23     // what digital pin we're connected to
#define RELAY_PIN  23  // ESP32 pin GIOP27, which connects to the IN pin of relay

#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

// Helper macro to calculate array size
#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

// WiFi access point
#define WIFI_AP_NAME       "Sim371"// "WIFI_AP"
// WiFi password
#define WIFI_PASSWORD      "91785634"// "WIFI_PASSWORD"

// See https://thingsboard.io/docs/getting-started-guides/helloworld/
// to understand how to obtain an access token
#define TOKEN              "B7z2NsiQPAdup8r6yLWl" // "TOKEN"
// ThingsBoard server instance.
#define THINGSBOARD_SERVER  "demo.thingsboard.io"

// Baud rate for debug serial
#define SERIAL_DEBUG_BAUD    115200

// Initialize ThingsBoard client
WiFiClient espClient;
// Initialize ThingsBoard instance
ThingsBoard tb(espClient);
// the Wifi radio's status
int status = WL_IDLE_STATUS;


// Period of sending a temperature/humidity data.
int send_delay = 2000;
unsigned long millis_counter;
void InitWiFi()
{
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

  WiFi.begin(WIFI_AP_NAME, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

void reconnect() {
  // Loop until we're reconnected
  status = WiFi.status();
  if ( status != WL_CONNECTED) {
    WiFi.begin(WIFI_AP_NAME, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("Connected to AP");
  }
}

// Setup an application
void setup() {
  // Initialize serial for debugging
  Serial.begin(SERIAL_DEBUG_BAUD);
  WiFi.begin(WIFI_AP_NAME, WIFI_PASSWORD);
  InitWiFi();


  // Initialize temperature sensor
  dht.begin();
  pinMode(RELAY_PIN, OUTPUT);
}

// Main application loop
void loop() {

  // Reconnect to WiFi, if needed
  if (WiFi.status() != WL_CONNECTED) {
    reconnect();
    return;
  }


  // Reconnect to ThingsBoard, if needed
  if (!tb.connected()) {
    // Connect to the ThingsBoard
    Serial.print("Connecting to: ");
    Serial.print(THINGSBOARD_SERVER);
    Serial.print(" with token ");
    Serial.println(TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      Serial.println("Failed to connect");
      return;
    }
  }

 

  // Check if it is a time to send DHT11 temperature and humidity
  if(millis()-millis_counter > send_delay) {
    Serial.println("Sending data...");

    // Uploads new telemetry to ThingsBoard using MQTT.
    // See https://thingsboard.io/docs/reference/mqtt-api/#telemetry-upload-api
    // for more details
    
     // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    bool fan = false;

    if (isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
    } else {
      Serial.print("Temperature:");
      Serial.print(t);
      if(t>25){
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("Turn On Cooler...");
      fan = true;
      delay(1000);
    }else{
      digitalWrite(RELAY_PIN, LOW);
      Serial.println("Turn Off Cooler...");
      fan = false;
      delay(1000);
    }
      tb.sendTelemetryFloat("temperature", t);
      tb.sendTelemetryFloat("fan", fan);
    }

    millis_counter = millis(); //reset millis counter
  }

  // Process messages
  tb.loop();
}
