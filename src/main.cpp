#include <Arduino.h>

// Вся конфигурация и платформозависимые настройки в pinout.h
#include "pinout.h"

//////////////////////////////////////////////////////////
#include "etl/etl_memory.h"
#include <GTimer.h>

#include "etl/etl_led.h"
etl::shared_ptr<etl::led> lightLED = etl::make_shared<etl::led>(LED_PIN, false);
volatile bool light_status = false;

// Сигнал - плавно мигнуть для пользователя в качестве обратной связи
uint32_t FADE_INTERVAL = 300;
uint32_t FADE_PAUSE = 300;
bool fade_direction = true;
etl::unique_ptr<GTimer<millis>> time_fade_pause;

// Кнопка управления переключением света
#include <EncButton.h>
Button btn(BTN_PIN);

// Лазерный датчик измерения расстояния
#include <Wire.h>
#include <VL53L0X.h>
VL53L0X sensor;

void setup() {
    Serial.begin(115200);
    if(SERIAL_INIT_DELAY > 0) delay(SERIAL_INIT_DELAY);  // для ESP32 C3 supermini нуждо сделать задержку, чтобы выводилась отладочная информация
    
    Serial.println("start...");

    if(lightLED)
    {
      Serial.println("light control started...");
      lightLED->init_pwm(LIGHT_CHANNEL, LIGHT_FREQUENCY, LIGHT_RESOLUTION); // Чтобы не было слышно пищания на низкой частоте - сделать 30КГц и максимально возможное разрешение 10 бит для плавности
      if(!light_status)
      {
        lightLED->fade_out(LIGHT_TOGGLE_DELAY);
      }
    }

    // Датчик расстояния
    Wire.begin();
    sensor.setTimeout(500);
    if (!sensor.init())
    {
      Serial.println("Failed to detect and initialize sensor!");
    //  while (1) {}
    }

    // Start continuous back-to-back mode (take readings as
    // fast as possible).  To use continuous timed mode
    // instead, provide a desired inter-measurement period in
    // ms (e.g. sensor.startContinuous(100)).
    sensor.startContinuous();
}

void loop() 
{
  // расстояние
  Serial.printf("distance: %d mm\n", sensor.readRangeContinuousMillimeters());
  if (sensor.timeoutOccurred()) { Serial.println("distance sensor TIMEOUT"); }

  // Управление светом
  if(lightLED) lightLED->tick();

  // Опрос кнопки переключения света
  btn.tick();
  if(btn.click())
  {
    light_status = !light_status;
    Serial.printf("Toggle light %s\n", light_status ? "ON" : "OFF");
    if(lightLED)
    {
      if(light_status)
      {
        //lightLED->on();
        lightLED->fade_in(LIGHT_TOGGLE_DELAY);
      }
      else
      {
        //lightLED->off();
        lightLED->fade_out(LIGHT_TOGGLE_DELAY);
      }
    }  
  }
  
}