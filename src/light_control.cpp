#include "light_control.h"
#include "etl/etl_utility.h"

bool light_control::is_active() const { return _state; }
void light_control::set_active(bool state) 
{ 
    _state = state; 
    // :TODO: включить свет до уровня brightness()
}

float light_control::brightness() const { return _brightness; }
void light_control::set_brightness(float brightness) 
{ 
    _brightness = etl::clamp<float>(brightness, 0.0, 1.0);

    if(is_active())
    {
        // :TODO:
    }
}