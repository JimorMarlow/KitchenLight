#include "settings.h"

namespace settings
{
    // Управление всеми настройками
mgr::mgr()
: _fs_light (&LittleFS, kitchen_light_path.c_str(), 'B', &_light, sizeof(_light), kitchen_light_update_delay)
{
    
}

// Инициализировать все настройки и считать значения из памяти или записать по-умолчанию в первый раз
bool mgr::init()    
{
    if(!LittleFS.begin())
    {
        Serial.println("Error LittleFS.begin(). Setting can not be stored in memory");
        return false;
    }

    Serial.println("setting::mgr init...");

    // прочитать данные из файла в переменную
    // при первом запуске в файл запишутся данные из структуры
    FDstat_t stat = _fs_light.read();

    switch (stat) {
        case FD_FS_ERR: Serial.printf("FS Error (%s)\n", kitchen_light_path.c_str());
            break;
        case FD_FILE_ERR: Serial.printf("FS File Open Error (%s)\n", kitchen_light_path.c_str());
            break;
        case FD_WRITE: Serial.printf("Data Write (%s)\n", kitchen_light_path.c_str());
            break;
        case FD_ADD: Serial.printf("Data Add (%s)\n", kitchen_light_path.c_str());
            break;
        case FD_READ: Serial.printf("Data Read (%s)\n", kitchen_light_path.c_str());
            break;
        default:
            break;
    }

    Serial.printf("settings::light.state = %s\n", _light.state ? "ON" : "OFF");
    Serial.printf("settings::light.brightness = %g\n", _light.brightness);
        
    return true;
}

// Получить настройки света 
kitchen_light_t mgr::get_light() const
{
    return _light;
}

// Изменить настройки света в памяти (отложенная запись по таймату, чтобы запоминать только последние данные)
// update_delay = 0 - записать без задержки
bool mgr::set_light(const kitchen_light_t& light, bool update_now /*= false*/)
{
    _light = light;
    if(update_now) 
        return _fs_light.updateNow() == FD_WRITE;
    else 
        _fs_light.update();
    return true;    
}

}// settings