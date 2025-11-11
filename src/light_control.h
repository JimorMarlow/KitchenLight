/*
    Главной класс управления светом
*/

#pragma once 
#include "Arduino.h"

class light_control
{
public:
    light_control() = default;
    virtual ~light_control() = default;

    virtual bool is_active() const;
    virtual void set_active(bool state);

    virtual float brightness() const;
    virtual void set_brightness(float brightness);

private:
    bool    _state      = false;    // Велючен свет или нет
    float   _brightness = 1.0;      // Целевой уровень яркости
};
