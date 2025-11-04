#include <Arduino.h>

// Вся конфигурация и платформозависимые настройки в pinout.h
#include "pinout.h"

//////////////////////////////////////////////////////////
#include "etl/etl_memory.h"
#include <GTimer.h>

#include "etl/etl_led.h"
etl::shared_ptr<etl::led> fadeLED;// = etl::make_shared<etl::led>(LED_FADE, false);

etl::shared_ptr<etl::led> lightLED = etl::make_shared<etl::led>(LED_FADE, false);
volatile bool light_status = false;

// Запуск по интервалу
uint32_t FADE_INTERVAL = 3000;
uint32_t FADE_PAUSE = 3000;
bool fade_direction = true;
etl::unique_ptr<GTimer<millis>> time_fade_pause;

// Кнопка управления переключением света
#include <EncButton.h>
Button btn(BTN_PIN);

void setup() {
    Serial.begin(115200);
    if(SERIAL_INIT_DELAY > 0) delay(SERIAL_INIT_DELAY);  // для ESP32 C3 supermini нуждо сделать задержку, чтобы выводилась отладочная информация
    
    Serial.println("start...");

    if(fadeLED) {
      Serial.println("fade started...");
      fadeLED->init_pwm(FADE_CHANNEL, 30000, 10); // Чтобы не было слышно пищания на низкой частоте - сделать 30КГц и максимально возможное разрешение 10 бит для плавности
      if(fade_direction) fadeLED->fade_in(FADE_INTERVAL); else fadeLED->fade_out(FADE_INTERVAL);
      Serial.printf("start fade %s\n", fade_direction ? "in" : "out");
    }

    if(lightLED)
    {
      Serial.println("light control started...");
      lightLED->init_pwm(LIGHT_CHANNEL, LIGHT_FREQUENCY, LIGHT_RESOLUTION); // Чтобы не было слышно пищания на низкой частоте - сделать 30КГц и максимально возможное разрешение 10 бит для плавности
      if(!light_status)
      {
        lightLED->fade_out(LIGHT_TOGGLE_DELAY);
      }
    }

    // Ручная настройка
    // задаём настройки ШИМ-канала:                                         
 //   ledcSetup(LIGHT_CHANNEL, LIGHT_FREQUENCY, LIGHT_RESOLUTION);
    // подключаем ШИМ-канал к пину светодиода:                                         
  //  ledcAttachPin(LED_FADE, LIGHT_CHANNEL);
}

void loop() 
{
  if(lightLED) lightLED->tick();
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
  // плавное включение и выключение LED, в выключенном положении пауза, чтобы оценить правильность затемнения без мерцания
  if(time_fade_pause && time_fade_pause->tick())
  {
    // start new cycle
    time_fade_pause.reset();
    fade_direction = true;
    if(fadeLED) fadeLED->fade_in(FADE_INTERVAL); 
  }
  else
  {
    if(fadeLED && fadeLED->tick()) // 
    {
      fade_direction = !fade_direction;
      if(fade_direction)
      {
        // На новом цикле делаем паузу в выключенном состоянии, чтобы посмотреть, не мигает ли лента
        time_fade_pause = etl::make_unique<GTimer<millis>>(FADE_PAUSE, true, GTMode::Interval);
      }
      else
      {
        fadeLED->fade_out(FADE_INTERVAL);
      }
      Serial.printf("main: fade %s\n", fade_direction ? "in" : "out");
        
    }
  }
}