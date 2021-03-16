#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include <TinyGPS++.h>
// The TinyGPS++ object
TinyGPSPlus gps;


int scanTime = 5; //Seconds
BLEScan* pBLEScan;

int found_count = 0;
int enable_send_sms = 0;

unsigned long long int time_last = 0;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      if (strstr(advertisedDevice.toString().c_str(), "14:77:40:e0:72:47"))
      {
        //Serial.println("Found");
        found_count++;
      }
      //Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
    }
};

void setup() {
  Serial.begin(9600);
  //Serial.println("Scanning...");

  Serial2.begin(9600,SERIAL_8N1,14,15);

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}

void loop() {
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory

  if (millis() - time_last > 60000)
  {
    if (found_count == 0)
    {
      enable_send_sms = 1;
    }
    
    time_last = millis();
    found_count = 0;
  }

  if (check_location_and_send_sms() == 1)
  {
    delay(60000*60); // 1 Hour
  }
}

int check_location_and_send_sms()
{
  while (Serial2.available() > 0)
  {
    if (gps.encode(Serial2.read()))
    {
      //Serial.println("Iaaaaa");
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
    Serial.println("AT+CMGS=\"insert phone number here\"");
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
