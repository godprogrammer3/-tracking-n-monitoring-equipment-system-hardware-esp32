
#include<Arduino.h>
#include "driver/gpio.h"
#include "driver/can.h"

void setup() {
  Serial.begin(115200);
  
  can_general_config_t g_config = { // สร้างต้วแปร g_config ใช้กำหนดค่าเกี่ยวกับบัส CAN
    .mode = CAN_MODE_NORMAL,
    .tx_io = GPIO_NUM_26, // กำหนดขา TX ต่อกับ 26
    .rx_io = GPIO_NUM_27, // กำหนดขา TX ต่อกับ 27
    .clkout_io = ((gpio_num_t) - 1),
    .bus_off_io = ((gpio_num_t) - 1),
    .tx_queue_len = 5,
    .rx_queue_len = 5,
    .alerts_enabled = CAN_ALERT_NONE,
    .clkout_divider = 0
  };
  can_timing_config_t t_config = CAN_TIMING_CONFIG_500KBITS();
  can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();

  if (can_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
    Serial.println("Failed to install driver");
    return;
  }

  if (can_start() != ESP_OK) {
    Serial.println("Failed to start driver");
    return;
  }
}

void loop() {
  can_message_t message;
  
  message.identifier = 0x10;
  message.flags = CAN_MSG_FLAG_NONE; // 
  message.data_length_code = 6; // ส่งข้อมูลยาว 6 ไบต์
  message.data[0] = 'H'; // ข้อมูลตำแหน่งแรก กำหนดเป็น H
  message.data[1] = 'e'; // ข้อมูลตำแหน่ง 2 กำหนดเป็น e
  message.data[2] = 'l'; // ข้อมูลตำแหน่ง 3 กำหนดเป็น l
  message.data[3] = 'l'; // ข้อมูลตำแหน่ง 4 กำหนดเป็น l
  message.data[4] = 'o'; // ข้อมูลตำแหน่ง 5 กำหนดเป็น o
  message.data[5] = '!'; // ข้อมูลตำแหน่ง 6 กำหนดเป็น !

  if (can_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK) {
    Serial.println("Message queued for transmission");
  } else {
    Serial.println("Failed to queue message for transmission");
  }

  delay(1000);
}