#pragma once
// Настройки проекта с сохранение в EEPROM или операционную систему

#include <Arduino.h>
namespace settings
{
    const String kitchen_light_path = "/kitchen_light.cfg";
    const uint16_t kitchen_light_update_delay = 30000;  // 30s
    struct kitchen_light_t
    {
        //bool    state      = false;    // Велючен свет или нет
        float   brightness = 1.0;      // Целевой уровень яркости

        void trace() {
            Serial.println("=== kitchen_light_t settings ===");
            //Serial.printf("state = %s\n", state ? "ON" : "OFF");
            Serial.printf("brightness = %g\n", brightness);
            Serial.println("========================");
        }            
    };
}// settings
