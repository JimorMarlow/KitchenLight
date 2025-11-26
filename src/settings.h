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

// Инициализация файловой системы
namespace etl
{
    class FSManager 
    {
    private:
        static bool _initialized;
        
    public:
        static bool begin() {
            if (!_initialized) {
                Serial.print("Initializing LittleFS... ");
                if (LittleFS.begin()) 
                {
                    _initialized = true;
                    Serial.println("OK");
                } 
                else 
                {
                    Serial.println("FAILED");
                }
            }
            return isReady(); // Уже инициализирован
        }
        
        static bool isReady() {
            return _initialized;
        }
    };

    namespace settings 
    {
        // Детектор метода trace()
        template<typename T, typename = void>
        struct has_trace : std::false_type {};
        
        template<typename T>
        struct has_trace<T, std::void_t<decltype(std::declval<T>().trace())>> : std::true_type {};
        
        template<typename T>
        constexpr bool has_trace_v = has_trace<T>::value;

        // Управление всеми настройками
        template<typename T>
        class data 
        {
            T        _data; // структура данных
            FileData _fd;   // Управление загрузкой данных в файловую система
            String   _path; // Путь к файлу для сохранения настроек

        public:
            // Путь к настройкам для этой структуры и интервал записи после обновленя в мс
            data(const String& path, uint16_t update_timeout = 5000)
            : _path(path)
            , _fd (&LittleFS, path.c_str(), 'B', &_data, sizeof(_data), update_timeout) 
            {}
            virtual ~data() = default;

            bool init()    // Инициализировать все настройки и считать значения из памяти или записать по-умолчанию в первый раз
            {
                if(!etl::FSManager::begin())
                {
                    Serial.println("Error LittleFS.begin(). Setting can not be stored in memory");
                    return false;
                }

                Serial.printf("etl::setting::data init <%s> - ", _path.c_str());

                // прочитать данные из файла в переменную
                // при первом запуске в файл запишутся данные из структуры
                FDstat_t stat = _fd.read();

                switch (stat) {
                    case FD_FS_ERR: Serial.println("FS Error");
                        break;
                    case FD_FILE_ERR: Serial.println("FS File Open Error");
                        break;
                    case FD_WRITE: Serial.println("Data Write");
                        break;
                    case FD_ADD: Serial.println("Data Add");
                        break;
                    case FD_READ: Serial.println("Data Read");
                        break;
                    default:
                        Serial.println();
                        break;
                }

                // Вызываем trace() если он есть у структуры T
                if constexpr (has_trace_v<T>) {
                    _data.trace();
                }
                    
                return true;
            }

            void tick()    // Вызывать в loop() для контроля отложенной записи
            {
                _fd.tick();
            }

            // Получить настройки
            T get() const { return _data; }
            // Изменить настройки в памяти (отложенная запись по таймату, чтобы запоминать только последние данные)
            // update_now == true - записать без задержки
            bool set(const T& data, bool update_now = false)
            {
                _data = data;
                if(update_now) {
                    return _fd.updateNow() == FD_WRITE;
                } else {
                    _fd.update();
                }
                return true;  
            }
        };
    }//..settings 
}//..etl