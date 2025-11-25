#pragma once
// Настройки проекта с сохранение в EEPROM или операционную систему

#include <Arduino.h>
#include <FileData.h>
#include <LittleFS.h>

namespace settings
{
    const String kitchen_light_path = "/kitchen_light.cfg";
    const uint16_t kitchen_light_update_delay = 30000;  // 30s
    struct kitchen_light_t
    {
        bool    state      = false;    // Велючен свет или нет
        float   brightness = 1.0;      // Целевой уровень яркости
    };

    // Управление всеми настройками
    class mgr 
    {
        kitchen_light_t _light; // Настройки света
        FileData _fs_light; // Управление загрузкой данных в файловую система

    public:
        mgr(); 
        virtual ~mgr() = default;

        virtual bool init();    // Инициализировать все настройки и считать значения из памяти или записать по-умолчанию в первый раз

        // Получить настройки света 
        virtual kitchen_light_t get_light() const;  
        // Изменить настройки света в памяти (отложенная запись по таймату, чтобы запоминать только последние данные)
        // update_now == true - записать без задержки
        virtual bool set_light(const kitchen_light_t& light, bool update_now = false);
    };
}// settings