## Освещение рабочей зоны кухни 
- Используется одноцветная полоса (2м) белой светодиодной ленты
- Управление яркостью с ESP32 C3 mini
- Жесты руками с лазерным дальномером

### Схема подключения / Wring and Pinouts
:TODO: нарисовать схему подключения 

### TODO
- разобраться с точностью измерения расстояния лазерным датчиком (пока на 300 мм более-менее точно, но на 450 мм уже милиметров 50 ошибка)
- Подумать над автоматической калибровкой расстояния от шкафчика до рабочей поверхности, чтобы разбивать его на "слои" управления
    - зона выключения < 100mm
    - максимальное расстояние для регулировки msx - 50 mm
    - мертвая зоны для регулировки яркости рукой +/- 50mm и внутренний диапазон 0.1 - 1.0 для яркости
- Сохранение настроек в память
- подключение к mqtt серверу и получение управляющих команд для интеграции с умным домом Яндекса (Алисой)
- webUI для настроек устройства

### Dependancy
- etl моя библиотека для работы с esp8266/esp32 под Ардуино платформу https://github.com/JimorMarlow/etl@^1.1.5
- GTimer от AlexGyver: https://github.com/GyverLibs/GTimer
- EncButton от AlexGyver: https://github.com/GyverLibs/EncButton
- https://github.com/pololu/vl53l0x-arduino библиотека работы с лазерным датчиком расстояния VL53L0X
- gyverlibs/FileData@^1.0.3 - библиотека сохранения данных во флеш-память вместо EEEPROM от AlexGyver

### Обновление библиотек
pio pkg update

### История изменений

#### 0.2.2 (2026-11-23)
- etl обновлён до 1.1.5 (было 1.1.2), проверено сборкой на espressif32 8.0.0 / Arduino core 3.3.12
- добавлен AGENTS.md — правила для AI-агентов, адаптированные из проекта Moonshine: ETL вместо STL, команды сборки, что нельзя ломать
- .vscode/extensions.json и .DS_Store убраны из индекса git и добавлены в .gitignore

#### 0.2.1 (2026-10-29)
- etl обновлён до 1.1.2
- заложены зависимости под будущие MQTT и webUI: GyverIO 1.4.2, ArduinoJson 7, PubSubClient 2.8; EncButton 3.7.3 -> 3.7.6
- в .vscode/extensions.json добавлены Jason2866.esp-decoder и pioarduino

#### 0.1.9 (2026-03-05)
- etl обновлён до 0.9.20
- до смены версии настройки переведены на etl::settings::data, убран самописный etl::FSManager (c3ffd41, e583a8b); до этого etl 0.9.8 (2ad75a9)
- сохранение настроек пока выключено: USE_SETTINGS закомментирован в light_control.h

#### 0.1.8 (2025-11-26)
- Переделал механизм для хранения настроек в постоянной памяти etl::settings::data<T>
- подключил в light_countrol

#### 0.1.7 (2025-11-25)
- Подключил сохранение данных в памяти для света. Пока не тестировал

#### 0.1.6 (2025-11-20)
- библиотека сохранения данных во флеш-память вместо EEEPROM от AlexGyver gyverlibs/FileData@^1.0.3
- шаблоны todo для сохранения настроек, mqtt, webui, добавлена папка /docs с документами по разработке

#### 0.1.5 (2025-11-18)
- появился класс light_control (яркость, вкл/выкл, плавный fade), loop() разгружен от работы с etl::led напрямую
- LIGHT_TOGGLE_DELAY и LIGHT_GUESTURE_DELAY стали const

#### 0.1.4 (2025-11-11)
- Реорганизация управляющих элемнтов

#### 0.1.3 (2025-11-05)
- version scrypt update
- fade using etl::led::fade with etl::lookup table (in progress)
- simple guesture control, wake on hand moving in range 150-500mm, switch of if hand close than 100mm (todo: sync with button control and led current brightness)
