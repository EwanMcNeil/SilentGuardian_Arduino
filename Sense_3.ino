#include <ArduinoBLE.h>
//custom uuid from GUID generator
BLEService service_pressure("19B10010-E8F2-537E-4F6C-D104768A1214");

BLEIntCharacteristic ble_threshold("19B10011-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify);
BLEDescriptor ble_descriptor_threshold("1800", "second field whaat");
//Corresponds to SEND_MESSAGE in App GattAttributes
BLEIntCharacteristic ble_notify("19B10012-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify);
BLEDescriptor ble_descriptor_notify("descriptor filed one", "descriptor field two");

BLECharacteristic ble_message("0001", BLERead | BLENotify, "declaration ble_message");


int sensor_pin = A7; //
int led_built_in = LED_BUILTIN;
const int THRESH_1 = 720;
const int THRESH_2 = 900;
const int led_T1 = 4 ; //D4
const int led_T2 = 5;//D5
const int TIMER = 80;//time to hold the sensor
int counter_T1 = 0;
int counter_T2 = 0;

unsigned long delayStart = 0;

void setup() {
  delayStart=millis();
  Serial.begin(9600);
  pinMode(led_built_in, OUTPUT);//uses built in led as indicator
  pinMode(sensor_pin, INPUT);
  pinMode(led_T1, OUTPUT);
  pinMode(led_T2, OUTPUT);
  BLESetup();
}

void loop() {
  delay(10);
  // poll for BLE events
  BLE.poll();
  int buttonValue = 0;// read thresholds
  ///////////////////have to filter here ////////////////////
  
 if ((millis() - delayStart) >= 10000){
    buttonValue = 800;
    Serial.println("threshold");
  }
 if((millis() - delayStart) >= 15000){
  delayStart = millis();
  buttonValue = 0;
 }
  
  byte msg = 0;
  if (buttonValue > THRESH_1) { //pressure > threshold 1
    digitalWrite(led_T1, HIGH);
    if (++counter_T1 == TIMER) { //held long enough
      for (int i = 0 ; i < 10; ++i) {//blinks T1 led to signal the user
        digitalWrite(led_T1, HIGH);
        delay(50);
        digitalWrite(led_T1, LOW);
        delay(50);
      }
      if (ble_threshold.value() != 1)//if value not already set
        SendMessage((byte)1);
    }
    else if (buttonValue > THRESH_2) { //pressure > threshold 2
      counter_T1 = 0;//reset T1 counter
      digitalWrite(led_T2, HIGH);
      if (++counter_T2 == TIMER) {
        for (int i = 0 ; i < 10; ++i) {
          digitalWrite(led_T2, HIGH);
          digitalWrite(led_T1, HIGH);
          delay(50);
          digitalWrite(led_T2, LOW);
          digitalWrite(led_T1, LOW);
          delay(50);
        }
        if (ble_threshold.value() != 2)
          SendMessage((byte)2);
      }
    } else digitalWrite(led_T2, LOW);//pressure < T2

  }
  else {//pressure < T1
    digitalWrite(led_T1, LOW);
    counter_T1 = 0;
    counter_T2 = 0;
    if (ble_threshold.value() != 0)
      SendMessage((byte)0);
  }
}//loop

void SendMessage(byte buttonValue) {
  boolean buttonPressed = ble_threshold.value() != buttonValue;
  // Serial.println(ble_threshold.value());
  if (buttonPressed) {
    ble_threshold.writeValue(buttonValue);
    ble_notify.writeValue((byte)buttonValue);
    ble_message.writeValue("Button was pressed.");
  }
}
void blePeripheralConnectHandler(BLEDevice central) {
  // central connected event handler
  //  Serial.print("Connected event, central: ");
  // Serial.println(central.address());
}
void blePeripheralDisconnectHandler(BLEDevice central) {
  // central disconnected event handler
  // Serial.print("Disconnected event, central: ");
  //  Serial.println(central.address());
}
void BLESetup() {
  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
    while (1);
  }
  BLE.setLocalName("Silent_Guardians");
  //initialize the characteristics
  ble_threshold.setValue(0);
  ble_notify.setValue(0);
  ble_message.setValue("No pressure.");
  //add charact to service
   service_pressure.addCharacteristic(ble_threshold);
  service_pressure.addCharacteristic(ble_notify);
  service_pressure.addCharacteristic(ble_message);
  //add descriptor
  ble_threshold.addDescriptor(ble_descriptor_threshold);
  ble_threshold.addDescriptor(ble_descriptor_notify);
  //add service
  BLE.addService(service_pressure);

  // set the UUID for the service this peripheral advertises:
  BLE.setAdvertisedService(service_pressure);

  // start advertising (so that device can connect)
  BLE.advertise();
  // Serial.println("Bluetooth device active, waiting for connections...");

  //event handler
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
}
