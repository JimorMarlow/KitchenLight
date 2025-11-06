## Освещение рабочей зоны кухни 
- Используется одноцветная полоса (2м) белой светодиодной ленты
- Управление яркостью с ESP32 C3 mini
- Жесты руками с лазерным дальномером

### Dependancy
- etl моя библиотека для работы с esp8266/esp32 под Ардуино платформу https://github.com/JimorMarlow/etl@^0.9.1
- GTimer от AlexGyver: https://github.com/GyverLibs/GTimer
- EncButton от AlexGyver: https://github.com/GyverLibs/EncButton
- https://github.com/pololu/vl53l0x-arduino библиотека работы с лазерным датчиком расстояния VL53L0X

### Обновление библиотек
pio pkg update

### История измениний

#### 0.1.3 (2025-11-05)
- version scrypt update
- fade using etl::led::fade with etl::lookup table (in progress)
- simple guesture control, wake on hand moving in range 150-500mm, switch of if hand close than 100mm (todo: sync with button control and led current brightness)
