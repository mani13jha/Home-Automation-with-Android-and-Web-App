#include <Firebase_ESP_Client.h> //Including Firebase Library by Mobitz
#include <addons/TokenHelper.h>  //Provide the token generation process info.
#include <addons/RTDBHelper.h>   //Provide the RTDB payload printing info and other helper functions.
#include <MFRC522.h>             //Including Library for RFID reader
#include <WiFi.h>                //Including WiFi Library for ESP32
#include <SPI.h>                 //Library for RFID SPI communication with ESP32
#include <DHT.h>                 //Including Library for Temperature Sensor

#define WIFI_SSID     "Maninder Jha"                             //Define the WiFi SSID
#define WIFI_PASSWORD "M@ni13jh@1999"                            //Define the WiFi Password
#define DATABASE_URL  "home-automation-e2edd.firebaseio.com"     //Define the RTDB URL
#define API_KEY       "AIzaSyAb-sg7W8ehCG69EdS08KMYEYtSGhI86lg"  //Define the API Key
#define USER_EMAIL    "mani13jha1999@gmail.com"                  //Define the user Email 
#define USER_PASSWORD "M@ni13jh@1999"                            //and password that alreadey registerd or added in your project

#define dht_type     DHT22        //Define DHT type that is in using(DHT22)
#define onboard_led      2        //Enabling On Board LED of ESP32 on pin D2
#define dht22            4        //Connecting DHT22 Data Pin to D4 of ESP32
#define buzzer_pin       5        //Connecting 5 Volt Buzzer +ve pin(Long Leg) to D5 pin of ESP32 and -ve pin(Short Leg) to GND
#define led_relay       12        //Connecting LED Relay IN pin to D12 pin of ESP32
#define fan_relay       13        //Connecting FAN Relay IN pin to D13 pin of ESP32
#define RST_PIN         22        //Connecting Reset pin of RFID to D22 of ESP32 
#define SS_PIN          21        //Connecting SDA pin of RFID to D21 of ESP32
#define tv_relay        25        //Connecting TV Relay IN pin to D25 pin of ESP32
#define ac_relay        32        //Connecting AC Relay IN pin to D32 pin of ESP32
#define fridge_relay    33        //Connecting Fridge Relay IN pin to D33 pin of ESP32
#define ir_sensor       34        //Connecting IR Sensor's OUT pin to D36 pin of ESP32
#define pir_sensor      36        //Connecting PIR Sensor's OUT pin to D39 pin of ESP32
#define ldr_sensor      39        //Connecting LDR Sensor's OUT pin to D34 pin of ESP32

FirebaseData   fbdo;              //Define Firebase Data object
FirebaseAuth   auth;              //Define Firebase Authentication object
FirebaseConfig config;            //Define Firebase Configuration object

DHT dht(dht22, dht_type);         //Initialize DHT sensor
MFRC522 rfid(SS_PIN, RST_PIN);    //Create MFRC522 instance


String ac_relay_val     = "0", ac_relay_val2     = "0";           //Defining AC Status
String tv_relay_val     = "0", tv_relay_val2     = "0";           //Defining TV Status
String led_relay_val    = "0", led_relay_val2    = "0";           //Defining Led Status
String fan_relay_val    = "0", fan_relay_val2    = "0";           //Defining Fan Status
String fridge_relay_val = "0", fridge_relay_val2 = "0";           //Defining Fridge Status 

bool entry1 = 0;  //Monitoring the entry or leave
bool entry2 = 0;  //Monitoring the entry or leave

void setup() {
  pinMode(onboard_led, OUTPUT);  //Initializing  D2 as OUTPUT Pin
  pinMode(buzzer_pin, OUTPUT);   //Initializing  D5 as OUTPUT Pin
  pinMode(led_relay, OUTPUT);    //Initializing D12 as OUTPUT Pin
  pinMode(fan_relay, OUTPUT);    //Initializing D13 as OUTPUT Pin
  pinMode(tv_relay, OUTPUT);     //Initializing D25 as OUTPUT Pin
  pinMode(ac_relay, OUTPUT);     //Initializing D32 as OUTPUT Pin
  pinMode(fridge_relay, OUTPUT); //Initializing D33 as OUTPUT Pin
  pinMode(ir_sensor, INPUT);     //Initializing D34 as INPUT Pin
  pinMode(pir_sensor, INPUT);    //Initilaizing D39 as INPUT Pin
  pinMode(ldr_sensor, INPUT);    //Initializing D36 as INPUT Pin

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);     //Starting ESP32 to connect to given WiFi
  while (WiFi.status() != WL_CONNECTED) {   //Waiting for the ESP32 to connect to WiFi
    digitalWrite(onboard_led, HIGH);
    delay(10);
    digitalWrite(onboard_led, LOW);
    delay(10);
  }

  SPI.begin();                                        //Initialize SPI bus
  rfid.PCD_Init();                                    //Initialize MFRC522 RFID

  config.api_key = API_KEY;                           //Assign the api key (required)
  auth.user.email = USER_EMAIL;                       //Assign the user email id in credentials
  auth.user.password = USER_PASSWORD;                 //Assign the user password in credentials
  config.database_url = DATABASE_URL;                 //Assign the RTDB URL (required)
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h Assign the callback function for the long running token generation task
  Firebase.begin(&config, &auth);                     //Accesing our Firebase RTDB
  Firebase.reconnectWiFi(true);                       //Comment or pass false value when WiFi reconnection will control by your code or third party library
  Firebase.setDoubleDigits(5);

  dht.begin();                                        //Accessing DHT22 Values

  for (int i = 1; i <= 60 ; i+=2) {
    Firebase.RTDB.setInt(&fbdo, "/Wait Counter", i);
    digitalWrite(onboard_led, HIGH);
    delay(1000);
    Firebase.RTDB.setInt(&fbdo, "/Wait Counter", i+1);
    digitalWrite(onboard_led, LOW);
    delay(1000);
  }
}

void loop() {
    light_intensity();
    door_bell();
    door_lock();
    led_control();
    temp_humidity();
    relay_control();
}

void light_intensity() {
    Firebase.RTDB.setInt(&fbdo, "/Environment/Light Intensity", analogRead(ldr_sensor));
}

void door_bell() {
    if(digitalRead(ir_sensor) == 0) {
        digitalWrite(buzzer_pin, HIGH);
        Firebase.RTDB.setString(&fbdo, "/Door Bell/Door Alarm", "\"Someone at the Door\"");
        while(!digitalRead(ir_sensor)){}
        digitalWrite(buzzer_pin, LOW);
        Firebase.RTDB.setString(&fbdo, "/Door Bell/Door Alarm", "\"No one\'s at the Door\"");
    }
}

void led_control() { 
    if (digitalRead(pir_sensor) == 1) {
      digitalWrite(led_relay, HIGH);
      Firebase.RTDB.setString(&fbdo, "/Appliances/Led", "1");
      Firebase.RTDB.setString(&fbdo, "/Appliances/Motion", "Motion Detected");
    }
    
    else if (digitalRead(pir_sensor) == 0){
      digitalWrite(led_relay, LOW);
      Firebase.RTDB.setString(&fbdo, "/Appliances/Led", "0");
      Firebase.RTDB.setString(&fbdo, "/Appliances/Motion", "No Motion Detected");
    }
}

void temp_humidity() {
    float h = dht.readHumidity();                   // Read Humidity 
    float t = dht.readTemperature();                // Read temperature as Celsius (the default)
    float f = dht.readTemperature(true);            // Read temperature as Fahrenheit (isFahrenheit = true)
    float hif = dht.computeHeatIndex(f, h);         // Compute heat index in Fahrenheit (the default)
    float hic = dht.computeHeatIndex(t, h, false);  // Compute heat index in Celsius (isFahreheit = false)
  
    Firebase.RTDB.setFloat(&fbdo, "/Environment/Humidity", h);
    Firebase.RTDB.setFloat(&fbdo, "/Environment/Temperature °C", t);
    Firebase.RTDB.setFloat(&fbdo, "/Environment/Temperature °F", f);
    Firebase.RTDB.setFloat(&fbdo, "/Environment/Heat Index °C", hic);
    Firebase.RTDB.setFloat(&fbdo, "/Environment/Heat Index °F", hif);
    
}

void relay_control() {
    Firebase.RTDB.getString(&fbdo, "/Appliances/AC", &ac_relay_val2);
    Firebase.RTDB.getString(&fbdo, "/Appliances/TV", &tv_relay_val2);
    Firebase.RTDB.getString(&fbdo, "/Appliances/Fan", &fan_relay_val2);
    Firebase.RTDB.getString(&fbdo, "/Appliances/Fridge", &fridge_relay_val2);
    
    delay(10);
    
    if (ac_relay_val != ac_relay_val2) {
      ac_relay_val = ac_relay_val2;
      digitalWrite(ac_relay, ac_relay_val.toInt());
    }
    
    if (tv_relay_val != tv_relay_val2) {
      tv_relay_val = tv_relay_val2;
      digitalWrite(tv_relay, tv_relay_val.toInt());
    }
    
    if (fan_relay_val != fan_relay_val2) {
      fan_relay_val = fan_relay_val2;
      digitalWrite(fan_relay, fan_relay_val.toInt());
    }
    
    if (fridge_relay_val != fridge_relay_val2) {
      fridge_relay_val = fridge_relay_val2;
      digitalWrite(fridge_relay, fridge_relay_val.toInt());
    }
  
}

void door_lock() {
    String tag_id1 = "182128215246";
    String tag_id2 = "4068675";
    String name1   = "Maninder";
    String name2   = "Prapti" ;
    String tag     = "";
    
    if ( !rfid.PICC_IsNewCardPresent())
      return;
    if (rfid.PICC_ReadCardSerial()) {
      for (byte i = 0; i < 4; i++) {
        tag += rfid.uid.uidByte[i];
        digitalWrite(buzzer_pin, HIGH);
      }
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
    }
  
    if ( tag == tag_id1) {
       digitalWrite(onboard_led, HIGH);
       entry1 = entry1 == 0 ? 1 : 0;
       Firebase.RTDB.setString(&fbdo, "/Door Lock/Entered Person ID", tag_id1);
       Firebase.RTDB.setString(&fbdo, "/Door Lock/Entered Person Name", name1);
       Firebase.RTDB.setString(&fbdo, "/Door Lock/Access", "Granted");
       Firebase.RTDB.setString(&fbdo, "/Door Lock/Entering Status", entry1 == 1 ? "Entered" : "Left");
    }
    else if ( tag == tag_id2) {
       digitalWrite(onboard_led, HIGH);
       entry2 = entry2 == 0 ? 1 : 0;
       Firebase.RTDB.setString(&fbdo, "/Door Lock/Entered Person ID", tag_id2);
       Firebase.RTDB.setString(&fbdo, "/Door Lock/Entered Person Name", name2);
       Firebase.RTDB.setString(&fbdo, "/Door Lock/Access", "Granted");
       Firebase.RTDB.setString(&fbdo, "/Door Lock/Entering Status", entry2 == 1 ? "Entered" : "Left");     
    }
    else {
       digitalWrite(onboard_led, LOW);
       Firebase.RTDB.setString(&fbdo, "/Door Lock/Entered Person ID", tag);
       Firebase.RTDB.setString(&fbdo, "/Door Lock/Entered Person Name", "NA");
       Firebase.RTDB.setString(&fbdo, "/Door Lock/Access", "Denied");
       Firebase.RTDB.setString(&fbdo, "/Door Lock/Entering Status", "NA");
    }
  
    digitalWrite(buzzer_pin, LOW);
}
