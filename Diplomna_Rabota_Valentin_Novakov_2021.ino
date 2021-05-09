// For Bluetooth
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

// For GPS
#include <TinyGPS++.h>

// For Buzzer
#define BUZZER 2

// The TinyGPS++ object
TinyGPSPlus gps;

// The BLEScan object
BLEScan* pBLEScan;

int scanTime = 5; //Seconds

int found_devices = 0;
int found_devices_last = 0;
int first_connection = false;
int enable_send_sms = false;

unsigned long long int time_last = 0;

// Paramethers
#define FIRST_SMS_DELAY         30 // Seconds
#define REPEATED_SMS_DELAY      1  // Minutes
#define WANTED_BLUETOOTH_MAC    "14:77:40:e0:72:47"  // Insert bluetooth mac here
#define PHONE_NUMBER_AT_COMMAND "AT+CMGS=\"+359888836440\"" // Insert your phone number here

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      if (strstr(advertisedDevice.toString().c_str(), WANTED_BLUETOOTH_MAC))
      {
        //Serial.println("Found");
        if (found_devices == 0)
        {
          found_devices = 1;
        }
      }
      //Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
    }
};

void setup() {
  Serial.begin(9600);
  //Serial.println("Scanning...");

  Serial2.begin(9600,SERIAL_8N1,14,15);

  pinMode(BUZZER, OUTPUT);

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value

  // Initialization Melody
  for (int i = 0; i < 4; i++)
  {
    digitalWrite(BUZZER, HIGH);
    delay(200);
    digitalWrite(BUZZER, LOW);
    delay(200);
  }
}

void loop() {
  found_devices_last = found_devices;
  found_devices = 0;
  
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory

  check_for_first_connection();

  if (first_connection)
  {
    handle_connection_lost();
  
    if (check_location_and_send_sms() == 1)
    {
      delay(60000*REPEATED_SMS_DELAY);
    }
  }
}

void check_for_first_connection()
{
  if ((first_connection == false) && (found_devices != 0))
  {
    first_connection = true;
    
    digitalWrite(BUZZER, HIGH);
    delay(500);
    digitalWrite(BUZZER, LOW);
  }
}

void handle_connection_lost()
{
  // Connection Drops
  if (found_devices < found_devices_last)
  {
    digitalWrite(BUZZER, HIGH);
    delay(2000);
    digitalWrite(BUZZER, LOW);
    
    time_last = millis();
  }
  else if(found_devices > found_devices_last)
  {
    digitalWrite(BUZZER, HIGH);
    delay(500);
    digitalWrite(BUZZER, LOW);
    
    enable_send_sms = 0;
  }
  
  if (millis() - time_last > FIRST_SMS_DELAY * 1000)
  {
    if (found_devices == 0)
    {
      enable_send_sms = 1;
    }
  }
}

int check_location_and_send_sms()
{
  while (Serial2.available() > 0)
  {
    if (gps.encode(Serial2.read()))
    {
      if (enable_send_sms == 1)
      {
        return sendSMSData();
      }
    }
  }

  return 0;
}

//void displayInfo()
//{
//  Serial.print(F("Location: ")); 
//  if (gps.location.isValid())
//  {
//    Serial.print(gps.location.lat(), 6);
//    Serial.print(F(","));
//    Serial.print(gps.location.lng(), 6);
//  }
//  else
//  {
//    Serial.print(F("INVALID"));
//  }
//  Serial.println();
//}


int sendSMSData()
{
  if (gps.location.isValid())
  {
    Serial.println("AT"); //Once the handshake test is successful, it will back to OK
    delay(500);
    Serial.println("AT+CMGF=1"); // Configuring TEXT mode
    delay(500);
    Serial.println(PHONE_NUMBER_AT_COMMAND);
    delay(500);
    Serial.print("Location: \n https://maps.google.com/maps?f=q&q=("); //text content
    delay(500);
    Serial.print(gps.location.lat(), 6);
    delay(500);
    Serial.print(F(","));
    delay(500);
    Serial.print(gps.location.lng(), 6);
    delay(500);
    Serial.print(")");
    delay(500);
    Serial.write(26);
    //Serial.println("Sending");
    return 1;
  }
  else
  {
    // Serial.println("INVALID");
    return 0;
  }
  
}
