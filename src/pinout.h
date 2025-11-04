#pragma once
#include <Arduino.h>

// НЕЛЬЗЯ встроенный, он не совпадает с нашей ESP32 C3 PRO MINI и уводит плату в панику при записи в 30 GPIO, нужно напрямую указать 8
#undef LED_BUILTIN
#define LED_BUILTIN 8
  
bool INVERSE_BUILTING_LED = true;
const int SERIAL_INIT_DELAY = 1000; // для ESP32 C3 supermini нуждо сделать задержку, чтобы выводилась отладочная информация

const int LED_FADE = 6;  // GPIO6 (MOSI)
const int FADE_CHANNEL = 0;  

const int BTN_PIN = 7; // GPIO 7
uint32_t LIGHT_TOGGLE_DELAY = 1500;
const int LIGHT_CHANNEL = 1;  
const uint32_t LIGHT_FREQUENCY = 30000;
const uint32_t LIGHT_RESOLUTION = 10;
