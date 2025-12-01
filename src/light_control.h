/*
    Главной класс управления светом
*/

#pragma once 
#include "Arduino.h"
#include "etl/etl_led.h"
#include "etl/etl_memory.h"
#include "etl/etl_settings.h"
#include "settings.h"

// #define USE_SETTINGS

class light_control
{
public:
    light_control(etl::shared_ptr<etl::led> led, uint32_t fade_toggle_duration = 1000);
    virtual ~light_control() = default;

    virtual bool init(int pwm_channel, uint32_t frequency, uint8_t resolution);
    virtual bool tick(); // call in loop(), true - fade timer finished

    virtual bool is_active() const;
    virtual void set_active(bool state);

    virtual float brightness() const;
    virtual void set_brightness(float brightness_value);

    virtual void welcome_blink(); // отладочное мигание 

protected:
    virtual void fade_to(float brightness); // Плавное изменение ярковсти от текущего значения до указанного

private:
    bool    _state      = false;    // Велючен свет или нет
#ifndef USE_SETTINGS
    float   _brightness = 1.0;      // Целевой уровень яркости
#else 
    etl::settings::data<settings::kitchen_light_t> _settings;   // Сохранение настроек в постоянной памяти
#endif//USE_SETTINGS

    etl::shared_ptr<etl::led> _led;
    uint32_t    _fade_toggle_duration = 1000; // ms, Вреия полного цикла переключения  
};
