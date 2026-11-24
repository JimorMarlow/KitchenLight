#include "light_control.h"
#include "etl/etl_utility.h"

light_control::light_control(etl::shared_ptr<etl::led> led, uint32_t fade_toggle_duration /*= 1000*/)
: _led(led)
, _fade_toggle_duration(fade_toggle_duration)
#ifdef USE_SETTINGS
, _settings(settings::kitchen_light_path, settings::kitchen_light_update_delay)
#endif//USE_SETTINGS
{
}

bool light_control::init(int pwm_channel, uint32_t frequency, uint8_t resolution)
{
#ifdef USE_SETTINGS
    _settings.init();
#endif//USE_SETTINGS

    if(_led)
    {
        _led->init_pwm(pwm_channel, frequency, resolution);
        return true;
    }
    return false;
}

bool light_control::tick() // true - fade timer finished
{
#ifdef USE_SETTINGS
    _settings.tick();
#endif//USE_SETTINGS

    if(_led) return _led->tick();
    return false;
}

bool light_control::is_active() const { return _state; }
void light_control::set_active(bool state) 
{ 
    _state = state; 
    // включить свет до уровня brightness()
    fade_to(is_active() ? brightness() : 0.0);
}

float light_control::brightness() const { 
#ifndef USE_SETTINGS
    return _brightness; 
#else 
    return _settings.get().brightness;
#endif//USE_SETTINGS
}
void light_control::set_brightness(float brightness_value) 
{ 
#ifndef USE_SETTINGS
    _brightness = etl::clamp<float>(brightness_value, 0.0, 1.0);
#else 
    auto data = _settings.get();
    data.brightness = etl::clamp<float>(brightness_value, 0.0, 1.0);
    _settings.set(data);
#endif//USE_SETTINGS

    if(is_active()) {
        fade_to(brightness());
    }
}

void light_control::fade_to(float brightness)
{
     // Плавное изменение ярковсти от текущего значения до указанного
     if(_led)
     {
        float cur_brightness = _led->get_brightness();
        float delta = fabsf(brightness - cur_brightness);
        uint32_t duration = static_cast<uint32_t>(_fade_toggle_duration * delta);

        etl::vector<etl::led::fade_t> brigtness_points;
        brigtness_points.push_back({0, cur_brightness});
        brigtness_points.push_back({int(duration), brightness});
        _led->fade(brigtness_points);
     }
}

void light_control::welcome_blink() // отладочное мигание 
{
    if(_led)
    {
        float cur = _led->get_brightness();
        etl::led::fade_t welcome_blink[] = {{0, cur}, {500, 1.0}, {1000, 0.0}, {2000, cur}};
        _led->fade(welcome_blink);
    }
}