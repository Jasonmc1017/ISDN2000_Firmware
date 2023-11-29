#include <Wire.h>
#include <Firebase_ESP_Client.h>
#include <WiFi.h>
#include <stdio.h>

//Self Written .hpp"
#include "init.hpp"
const char* ssid = "JASON";
const char* password = "12345678";

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert Firebase project API Key
#define API_KEY  "AIzaSyBAC4PhpS_hT3ykloYtZ-VM3b2p3CtYNdM"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://isdn2000-final-project-default-rtdb.firebaseio.com/"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;
bool lost = false;

//Device ID Address 
uint8_t ID = 2; 

void setup()
{

  Serial.begin(115200);
  // Begin serial communication Arduino IDE (Serial Monitor)
  //Init Serial. 
  Serial.println("-------Initialing...--------");
 
  // Connect Wifi

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
    
  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", ""))
  {
    Serial.println("ok");
    signupOK = true;
  }
  else
  {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  delay(500);


  //Setup the Servo PWM Channel
ledcSetup(ServoPWM, Servo_Freq, SigResolution);
ledcAttachPin(ServoPin_1, ServoPWM);

// //Setting up MQTT Connection
//   setup_wifi();
//   client.setServer(mqttServer, 1883);
//   client.setCallback(callback);

//Init Pin
pinMode(Motor_IN1, OUTPUT);
pinMode(Motor_IN2, OUTPUT);
pinMode(LED_1, OUTPUT);
pinMode(LED_2, OUTPUT);
pinMode(Thermal_Sensor, INPUT);
pinMode(trigPin, OUTPUT); 
pinMode(echoPin, INPUT); 
pinMode(ServoPin_1, OUTPUT); 

}
/// @user functions---
uint8_t distance_sensing()
{
  long duration;
  uint8_t distance;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2;
  // Debug
  // Serial.print("Distance: ");
  // Serial.println(distance);
  return distance;
}



bool HumanDetector(float distance)
{
  if(digitalRead(Thermal_Sensor) == 1 && distance <= 60){
    digitalWrite(LED_2,HIGH);
    return true;
  }
  else{
    digitalWrite(LED_2,LOW);
    return false;
  }
}

//Servo Motor Driver 
void ServoMotorDeg(uint8_t degree)
{
  //deg = 0 -->  1ms 
  //deg = 90 --> 1.5ms 
  //deg = 180 --> 2ms 
  //should be 1-2 ms (5-10% Duty Cycle)
  int dutycycle = 12.8 + degree*(38.4)/180;
ledcWrite(ServoPWM,dutycycle);
Serial.println(dutycycle);
}

// void SearchingForHuman(){


bool current_status = false;
bool prev_status = false; 


float avg_distance = 0.0f; 
// }
void loop()
{
  boolean newData = false;
  bool distacne_data_ready = false;
  for (unsigned long start = millis(); millis() - start < 1000;)
  {;
  }
  Serial.println("---------------------------------------------");
  //Getting the status of the sit
  uint8_t distance_sum = 0;
  if(distacne_data_ready == false){
  for(int i = 0; i<10 ;){
    distance_sum += distance_sensing();
    i++;
    if(i == 10){
    distacne_data_ready = true;
    avg_distance = distance_sum/10;
    //For Debug
    Serial.print("Distance: ");
    Serial.println(avg_distance);
    }
  }
  }
  current_status = HumanDetector(avg_distance);
  Serial.print("Current Status:");
  Serial.println(current_status);

  FirebaseJson json;

  json.set("Device/" + String(ID), F("cool!"));
  json.set(F("Device/ID2/Status/"), false);
  // Serial.printf("Set json... %s\n", Firebase.RTDB.set(&fbdo, F("/test/json"), &json) ? "ok" : fbdo.errorReason().c_str());
  Serial.println("Json Setted.");

  if(current_status != prev_status){
    newData = true; 
    prev_status = current_status;
  }
  // If newData is true
  if (newData == true)
  {
    newData = false;
    Firebase.RTDB.setInt(&fbdo, "Device/ID2",ID);
    Firebase.RTDB.setBool(&fbdo, "Device/ID2/Status",current_status);
    Serial.println("New Data Send to Firebase." );
    
  }
  else
  {
    Serial.println("Nothing new" );
    
    delay(300);
  }

  delay(500);
}