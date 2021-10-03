#include "driver/gpio.h"
#include "driver/can.h"
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
int scanTime = 5; //In seconds
BLEScan* pBLEScan;
String stringBuffers;
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
     if(advertisedDevice.haveName() && 
        strcmp(advertisedDevice.getName().c_str(),"UT9") == 0 && 
        stringBuffers.indexOf(String(advertisedDevice.getAddress().toString().c_str())) == -1
     ){
      //  Serial.printf("%s,mac_address:%s\n", advertisedDevice.toString().c_str(),advertisedDevice.getAddress().toString().c_str());
       stringBuffers += String(advertisedDevice.getAddress().toString().c_str())+String(',');
     }
    }
};

void macAddressStringToByteArray(String macAddress,uint8_t result[6]){
  for (int i = 0 ; i < 6 ; i++) {
    unsigned long tmpByte = strtoul(macAddress.substring(3*i,3*i+2).c_str(),nullptr,16);
    result[i] = tmpByte;
  }
}

void canBusSendMessage(uint8_t data[8],uint8_t length){
  can_message_t message;
  
  message.identifier = 0;
  message.flags = CAN_MSG_FLAG_NONE;
  message.data_length_code = length; 
  for (int i = 0 ; i < length ; i++) {
     message.data[i] = data[i];
  }
  if (can_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK) {
    Serial.println("Message queued for transmission");
  } else {
    Serial.println("Failed to queue message for transmission");
  }
}

void sendAllMacAddressViaCanBus(){
  String data = stringBuffers;
  int maxIndex = data.length() - 1;
  int index=0;
  int next_index;
  String data_word;
  Serial.println("mac addresses:");
  do{
      next_index=data.indexOf(',',index);
      data_word=data.substring(index, next_index);
      Serial.print("->");
      Serial.println(data_word);
      uint8_t bytes[6];
      macAddressStringToByteArray(data_word,bytes);
      canBusSendMessage(bytes,6);
      index=next_index+1;
    }while((next_index!=-1)&&(next_index<maxIndex));
}

void setup()
{
  Serial.begin(115200);
  can_general_config_t g_config = {// สร้างต้วแปร g_config ใช้กำหนดค่าเกี่ยวกับบัส CAN
                                   .mode = CAN_MODE_NORMAL,
                                   .tx_io = GPIO_NUM_26, // กำหนดขา TX ต่อกับ 26
                                   .rx_io = GPIO_NUM_27, // กำหนดขา TX ต่อกับ 27
                                   .clkout_io = (gpio_num_t)CAN_IO_UNUSED,
                                   .bus_off_io = (gpio_num_t)CAN_IO_UNUSED,
                                   .tx_queue_len = 65,
                                   .rx_queue_len = 65,
                                   .alerts_enabled = CAN_ALERT_ALL,
                                   .clkout_divider = 0};
  can_timing_config_t t_config = CAN_TIMING_CONFIG_500KBITS();
  can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();
  if (can_driver_install(&g_config, &t_config, &f_config) != ESP_OK)
  {
    Serial.println("Failed to install driver");
    return;
  }
  if (can_start() != ESP_OK)
  {
    Serial.println("Failed to start driver");
    return;
  }
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}

void loop()
{
  can_message_t message;
  if (can_receive(&message, pdMS_TO_TICKS(10000)) == ESP_OK)
  {
    Serial.println("Start SCAN");
    if(message.identifier == 1){
      stringBuffers = "";
      BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
      pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
      Serial.println(stringBuffers);
      if(stringBuffers.length()){
        sendAllMacAddressViaCanBus();
      }
      uint8_t data[] = {102,105,110,105,115,104,101,100};
      canBusSendMessage(data,8);
      Serial.println("finished");
    }
  }
  else
  {
    return;
  }
}