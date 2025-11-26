#include <Arduino.h>

// Вся конфигурация и платформозависимые настройки в pinout.h
#include "pinout.h"

//////////////////////////////////////////////////////////
#include "etl/etl_memory.h"
#include "etl/etl_optional.h"
#include <GTimer.h>

#include "etl/etl_led.h"
etl::shared_ptr<etl::led> lightLED = etl::make_shared<etl::led>(LED_PIN, false);
#include "light_control.h"
light_control light(lightLED, LIGHT_TOGGLE_DELAY);
volatile float brightness_level = 1.0; // запомненный уровень яркости 

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
uint32_t guesture_time = 0;
const uint32_t GUESTURE_DELAY = 2000;

void setup() {
    Serial.begin(115200);
    if(SERIAL_INIT_DELAY > 0) delay(SERIAL_INIT_DELAY);  // для ESP32 C3 supermini нуждо сделать задержку, чтобы выводилась отладочная информация
    
    Serial.printf("KitchenLight v%s started...\n", String(KL_VERSION_STRING).c_str());
  
    Serial.println("light control started...");
    light.init(LIGHT_CHANNEL, LIGHT_FREQUENCY, LIGHT_RESOLUTION); // Чтобы не было слышно пищания на низкой частоте - сделать 30КГц и максимально возможное разрешение 10 бит для плавности
    //  lightLED->fade(welcome_blink);
    light.set_brightness(brightness_level);
   
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
  // Управление светом
  if(light.tick()) { 
    auto brightness = light.brightness();
    auto light_status = light.is_active();
    Serial.printf("FADE END: brightness = %g, light_status = %s\n", brightness, light_status ? "ON" : "OFF");
  }

  // расстояние
  if (sensor.timeoutOccurred()) { Serial.println("distance sensor TIMEOUT"); }
  else {
    uint16_t distance = sensor.readRangeContinuousMillimeters();
    //Serial.printf("distance: %d mm\n", distance);
    if((millis() - guesture_time) > GUESTURE_DELAY)
    {
      if(distance < 100 && light.is_active())
      {
        // OFF light
        guesture_time = millis();
        light.set_active(false);
        Serial.printf("Toggle light by distance OFF\n");
      }
      else if(distance > 150 && distance < 500 && !light.is_active())
      {
        // ON light
        guesture_time = millis();
        light.set_active(true);
        Serial.printf("Toggle light by distance ON\n");
      }
      // else if(distance > 600 && distance < 700)
      // {
      //   guesture_time = millis();
      //   Serial.printf("Welcome blink test\n");
      //   light.welcome_blink();
      // }
    }
  }
  
  // Опрос кнопки переключения света
  btn.tick();
  if(btn.click())
  {
    if((millis() - guesture_time) > GUESTURE_DELAY)
    {
      guesture_time = millis();
      light.set_active(!light.is_active());
      Serial.printf("Toggle light by button %s\n", light.is_active() ? "ON":"OFF");
    }  
  }
  
}