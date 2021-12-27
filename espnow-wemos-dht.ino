
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

uint8_t broadcastAddress[] = {0xc8, 0xc9, 0xa3, 0xd2, 0x5a, 0xc8};
#define BOARD_ID 2

#define DHTPIN 13    
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
DHT dht(DHTPIN, DHTTYPE);

// Define variables to store DHT readings to be sent
//float temperature;
float humidity;

// Updates DHT readings every 10 seconds
const long interval = 10000; 
unsigned long previousMillis = 0;    // will store last time DHT was updated 

// Variable to store if sending data was successful
String success;

typedef struct struct_message {
    int id;
    //float temp;
    float hum;
} struct_message;
struct_message DHTReadings;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}

void getReadings(){
  // Read Temperature
  /*temperature = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  //float t = dht.readTemperature(true);
  if (isnan(temperature)){
    Serial.println("Failed to read from DHT");
    temperature = 0.0;
  } */
  humidity = dht.readHumidity();
  if (isnan(humidity)){
    Serial.println("Failed to read from DHT");
    humidity = 0.0;
  }
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  dht.begin();
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Set ESP-NOW Role
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
}
 
void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you updated the DHT values
    previousMillis = currentMillis;

    //Get DHT readings
    getReadings();

    //Set values to send
    DHTReadings.id = BOARD_ID;
    //DHTReadings.temp = temperature;
    DHTReadings.hum = humidity;

    // Send message via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *) &DHTReadings, sizeof(DHTReadings));
  }
}