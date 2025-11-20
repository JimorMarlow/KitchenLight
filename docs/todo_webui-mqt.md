### Телеметрия на ESP8266 + MQTT. Пошаговое руководство по созданию DIY-проекта с удаленным управлением
https://kotyara12.ru/iot/esp8266mqtt/

### Arduino ESP32 шаг за шагом. Телеметрия через WiFi и MQTT для чайников
https://kotyara12.ru/iot/esp32mqtt-arduino/


### deeepseek promt:

    WiFi для esp8266 и ESP32 работал не в режиме WIFI_STA, а одновременно, чтобы по локальной точке доступа к нему можно было подключаться как к серверу на сраницу настроек и контроля статуса, а он сам подлючался ко внешней точке доступа для связи с MQTT сервером


# Вам нужен режим **WiFi STA + AP** (Station + Access Point). Вот полная реализация:

## Для ESP8266 и ESP32 (совместимый код)


#include <ESP8266WiFi.h>  // Для ESP8266
// #include <WiFi.h>      // Для ESP32 - раскомментируйте
#include <ESPAsyncWebServer.h>

// Настройки WiFi
const char* ssid = "Your_Main_WiFi";      // Основная сеть для MQTT
const char* password = "Your_Main_Password";

// Настройки точки доступа
const char* base_ap_ssid = "KLight";
String full_ap_ssid;
String ap_password;

// Веб-сервер на порту 80
AsyncWebServer server(80);

// Флаги статуса
bool wifi_connected = false;
bool mqtt_connected = false;

void setup() {
  Serial.begin(115200);
  
  // Генерируем уникальное имя и пароль
  generateUniqueAPNameAndPassword();

  // Запускаем точку доступа ПЕРВОЙ
  setupAccessPoint();

  Serial.println("=== Device Info ===");
  Serial.println("AP Name: " + full_ap_ssid);
  Serial.println("Password: " + ap_password);
  Serial.println("===================");
  
  // Запускаем веб-сервер
  setupWebServer();
  
  // Пытаемся подключиться к основной WiFi
  connectToMainWiFi();
  
  // Здесь ваш код подключения к MQTT
  setupMQTT();
}

void generateUniqueAPNameAndPassword() {
  // Получаем MAC-адрес
  String mac = WiFi.macAddress();
  mac.toUpperCase();
  
  // Формируем имя точки доступа с полным MAC
  full_ap_ssid = String(base_ap_ssid) + "_" + mac;
  
  // Генерируем пароль по алгоритму
  ap_password = generatePasswordFromMAC(mac);
}

String generatePasswordFromMAC(const String& mac) {
  String clean_mac = mac;
  clean_mac.replace(":", "");  // Убираем двоеточия
  
  String password = "";
  int length = clean_mac.length();
  
  // Алгоритм: с конца + с начала, двигаемся к центру
  for (int i = 0; i < length / 2; i++) {
    // Берем символ с конца
    password += clean_mac[length - 1 - i];
    // Берем символ с начала  
    password += clean_mac[i];
  }
  
  // Если нечетное количество символов, добавляем средний
  if (length % 2 == 1) {
    password += clean_mac[length / 2];
  }
  
  return password;
}

void setupAccessPoint() {
  Serial.println("Starting Access Point...");
  
  // Запускаем точку доступа с уникальным именем
  WiFi.softAP(full_ap_ssid.c_str(), ap_password);
  
  Serial.print("AP Name: ");
  Serial.println(full_ap_ssid);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
}

void connectToMainWiFi() {
  Serial.println("Connecting to main WiFi...");
  
  WiFi.begin(ssid, password);
  
  // Неблокирующее подключение - не ждем вечно
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifi_connected = true;
    Serial.println("\nConnected to main WiFi!");
    Serial.print("STA IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to main WiFi");
    Serial.println("Device will work in AP mode only");
  }
}

void setupWebServer() {
  // Главная страница с настройками и статусом
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Kitchen Light Control</title>
    <meta charset="UTF-8">
    <style>
        body { font-family: Arial; margin: 20px; }
        .status { padding: 10px; margin: 10px 0; border-radius: 5px; }
        .connected { background: #d4edda; color: #155724; }
        .disconnected { background: #f8d7da; color: #721c24; }
        button { padding: 10px 20px; margin: 5px; font-size: 16px; }
        .on { background: #28a745; color: white; }
        .off { background: #dc3545; color: white; }
    </style>
</head>
<body>
    <h1>Kitchen Light Control</h1>
    
    <div class="status %WIFI_STATUS%">WiFi: %WIFI_IP%</div>
    <div class="status %MQTT_STATUS%">MQTT: %MQTT_STATUS_TEXT%</div>
    
    <h2>Light Control</h2>
    <button class="on" onclick="sendCommand('on')">ON</button>
    <button class="off" onclick="sendCommand('off')">OFF</button>
    
    <h2>WiFi Settings</h2>
    <form action="/configure" method="post">
        <input type="text" name="ssid" placeholder="WiFi SSID" required><br>
        <input type="password" name="password" placeholder="WiFi Password"><br>
        <input type="submit" value="Save and Reconnect">
    </form>
    
    <script>
    function sendCommand(cmd) {
        fetch('/control?command=' + cmd)
        .then(response => response.text())
        .then(data => alert('Command sent: ' + data));
    }
    </script>
</body>
</html>
    )rawliteral";
    
    // Заменяем плейсхолдеры реальными данными
    html.replace("%WIFI_STATUS%", wifi_connected ? "connected" : "disconnected");
    html.replace("%WIFI_IP%", wifi_connected ? WiFi.localIP().toString() : "Not connected");
    html.replace("%MQTT_STATUS%", mqtt_connected ? "connected" : "disconnected");
    html.replace("%MQTT_STATUS_TEXT%", mqtt_connected ? "Connected" : "Disconnected");
    
    request->send(200, "text/html", html);
  });
  
  // API для управления светом
  server.on("/control", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("command")) {
      String command = request->getParam("command")->value();
      
      if (command == "on") {
        // Включить свет
        // your_light_control.set_active(true);
        request->send(200, "text/plain", "Light ON");
      } else if (command == "off") {
        // Выключить свет  
        // your_light_control.set_active(false);
        request->send(200, "text/plain", "Light OFF");
      } else {
        request->send(400, "text/plain", "Invalid command");
      }
    } else {
      request->send(400, "text/plain", "No command specified");
    }
  });
  
  // API для настройки WiFi
  server.on("/configure", HTTP_POST, [](AsyncWebServerRequest *request){
    // Здесь можно сохранить настройки в EEPROM
    // и переподключиться к новой сети
    
    if (request->hasParam("ssid", true)) {
      String new_ssid = request->getParam("ssid", true)->value();
      String new_password = request->getParam("password", true)->value();
      
      // Сохраняем в EEPROM (добавьте свою реализацию)
      saveWiFiCredentials(new_ssid, new_password);
      
      request->send(200, "text/html", 
        "<html><body>"
        "<h1>Settings Saved</h1>"
        "<p>Device will reconnect to new network</p>"
        "<a href='/'>Back to control panel</a>"
        "</body></html>");
      
      // Перезагружаем для применения настроек
      delay(2000);
      ESP.restart();
      
    } else {
      request->send(400, "text/plain", "SSID required");
    }
  });
  
  // Запускаем сервер
  server.begin();
  Serial.println("HTTP server started");
}

void saveWiFiCredentials(const String& ssid, const String& password) {
  // Реализуйте сохранение в EEPROM
  // Пример:
  // EEPROM.begin(512);
  // // Запись ssid и password
  // EEPROM.commit();
  Serial.println("Saving WiFi credentials: " + ssid);
}

void setupMQTT() {
  // Здесь ваш код подключения к MQTT
  // Подключаемся только если WiFi подключен
  if (wifi_connected) {
    // mqttClient.connect(...);
    mqtt_connected = true; // Временно для демонстрации
  }
}

void loop() {
  // Основной цикл - здесь ваш MQTT и другая логика
  
  // Периодически проверяем подключение WiFi
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 30000) { // Каждые 30 секунд
    lastCheck = millis();
    
    if (WiFi.status() != WL_CONNECTED && wifi_connected) {
      wifi_connected = false;
      mqtt_connected = false;
      Serial.println("WiFi disconnected!");
    } else if (WiFi.status() == WL_CONNECTED && !wifi_connected) {
      wifi_connected = true;
      Serial.println("WiFi reconnected!");
      // Переподключаем MQTT
      setupMQTT();
    }
  }
  
  // Ваш основной код...
  delay(100);
}
```

## Ключевые особенности:

1. **Двойной режим**: ESP создает свою точку доступа И подключается к основной WiFi
2. **Неблокирующее подключение**: Не зависает при проблемах с WiFi
3. **Веб-интерфейс**: Настройки и управление через браузер
4. **API endpoints**: `/control` для управления, `/configure` для настроек
5. **Автоматическое восстановление**: Мониторинг подключения

## Как подключиться:

1. **Для управления**: Подключитесь к WiFi `KitchenLight_Config` (пароль: `12345678`)
2. **Откройте в браузере**: `http://192.168.4.1`
3. **Управляйте светом** и настраивайте подключение к основной WiFi

## Дополнительные улучшения:

- Сохранение настроек в EEPROM
- OTA-обновления через веб-интерфейс  
- Защита паролем веб-интерфейса
- DNS сервер для красивого URL

Этот подход дает вам полный контроль над устройством через локальную сеть + доступ к MQTT для интеграции с другими системами!

### TODO:
- Имя устройства будет содержать суффикс с MAC-адресом, чтобы понимтаь, какое из однотипных устройств мне нужно, например "KLight_12:34:56:78:90:AB"
- Пароль по умолчанию для Web-UI сервера будет вычисляться из MAC-адреса, чтобы не забыть потом, и от простых любопытных защитит. Можно сделать, например, лесенку справа-налево, от края к центру: для адреса 12:34:56:78:90:AB пароль будет B1A203948576, все лучше, чем забивать стандартный, или делеть сложный, который потом сам забудешь.

### Добавил доступ по hostname
  а можно задать точке доступа какой-то hostname, чтобы подключаться в браузере по нему, а не по IP-адреск
  Да, конечно! Это отличная идея. Вот как настроить hostname для ESP:

Для ESP8266
cpp
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

const char* base_ap_ssid = "KLight";
String full_ap_ssid;
String ap_password;
String hostname;

void setup() {
  Serial.begin(115200);
  
  // Генерируем уникальное имя, пароль и hostname
  generateUniqueAPNameAndPassword();
  
  // Генерируем hostname на основе MAC
  generateHostname();
  
  // Запускаем точку доступа
  setupAccessPoint();
  
  // Настраиваем mDNS (для доступа по имени)
  setupMDNS();
  
  Serial.println("=== Device Info ===");
  Serial.println("AP Name: " + full_ap_ssid);
  Serial.println("Password: " + ap_password);
  Serial.println("Hostname: " + hostname + ".local");
  Serial.println("IP address: " + WiFi.softAPIP().toString());
  Serial.println("===================");
}

void generateHostname() {
  String clean_mac = WiFi.macAddress();
  clean_mac.replace(":", "");
  
  // Берем последние 6 символов MAC для короткого имени
  String short_mac = clean_mac.substring(6);
  
  // Формируем hostname (только латинские буквы, цифры и дефисы)
  hostname = "klight-" + short_mac.toLowerCase();
  
  // Устанавливаем hostname для WiFi
  WiFi.hostname(hostname);
}

void setupMDNS() {
  if (MDNS.begin(hostname.c_str())) {
    Serial.println("mDNS responder started");
    Serial.println("You can now access device at: http://" + hostname + ".local");
    
    // Добавляем сервисы (опционально)
    MDNS.addService("http", "tcp", 80);
  } else {
    Serial.println("Error setting up mDNS responder!");
  }
}

// Ваши предыдущие функции остаются без изменений
void generateUniqueAPNameAndPassword() {
  String mac = WiFi.macAddress();
  mac.toUpperCase();
  full_ap_ssid = String(base_ap_ssid) + "_" + mac;
  ap_password = generatePasswordFromMAC(mac);
}

String generatePasswordFromMAC(const String& mac) {
  String clean_mac = mac;
  clean_mac.replace(":", "");
  String password = "";
  int start = 0;
  int end = clean_mac.length() - 1;
  while (start < end) {
    password += clean_mac[end--];
    password += clean_mac[start++];
  }
  return password;
}

void setupAccessPoint() {
  WiFi.softAP(full_ap_ssid.c_str(), ap_password.c_str());
}
Для ESP32
cpp
#include <WiFi.h>
#include <ESPmDNS.h>

// Остальной код аналогичен, только setupMDNS() для ESP32:

void setupMDNS() {
  if (MDNS.begin(hostname.c_str())) {
    Serial.println("mDNS responder started");
    Serial.println("You can now access device at: http://" + hostname + ".local");
    MDNS.addService("http", "tcp", 80);
  } else {
    Serial.println("Error setting up mDNS responder!");
  }
}
Улучшенная версия с обработкой разных случаев
cpp
void generateHostname() {
  String clean_mac = WiFi.macAddress();
  clean_mac.replace(":", "");
  String short_mac = clean_mac.substring(6);
  
  // Создаем валидный hostname (только a-z, 0-9, -)
  hostname = "klight-" + short_mac.toLowerCase();
  
  // Устанавливаем hostname
  #ifdef ESP8266
    WiFi.hostname(hostname);
  #else
    WiFi.setHostname(hostname.c_str());
  #endif
}

void setupMDNS() {
  Serial.print("Starting mDNS with hostname: ");
  Serial.println(hostname);
  
  #ifdef ESP8266
    if (MDNS.begin(hostname.c_str())) {
  #else
    if (MDNS.begin(hostname.c_str())) {
  #endif
    Serial.println("✓ mDNS responder started successfully");
    Serial.println("📍 Access your device at: http://" + hostname + ".local");
    
    // Добавляем веб-сервер
    MDNS.addService("http", "tcp", 80);
    
  } else {
    Serial.println("✗ Error starting mDNS responder");
    Serial.println("⚠️  You can still access via IP: " + WiFi.softAPIP().toString());
  }
}
Полная версия с веб-сервером
cpp
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  
  generateUniqueAPNameAndPassword();
  generateHostname();
  setupAccessPoint();
  setupMDNS();
  setupWebServer();
  
  printDeviceInfo();
}

void setupWebServer() {
  // Главная страница
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Kitchen Light</title>
    <meta charset="UTF-8">
    <style>
        body { font-family: Arial; margin: 40px; text-align: center; }
        .info { background: #f0f0f0; padding: 20px; border-radius: 10px; margin: 20px; }
        button { padding: 15px 30px; font-size: 18px; margin: 10px; border: none; border-radius: 5px; cursor: pointer; }
        .on { background: #4CAF50; color: white; }
        .off { background: #f44336; color: white; }
    </style>
</head>
<body>
    <h1>🍳 Kitchen Light Control</h1>
    <div class="info">
        <h3>Device Information</h3>
        <p><strong>Hostname:</strong> %HOSTNAME%</p>
        <p><strong>IP:</strong> %IP%</p>
        <p><strong>MAC:</strong> %MAC%</p>
    </div>
    <h2>Light Control</h2>
    <button class="on" onclick="controlLight('on')">💡 Turn ON</button>
    <button class="off" onclick="controlLight('off')">🔌 Turn OFF</button>
    
    <script>
    function controlLight(state) {
        fetch('/control?state=' + state)
        .then(r => r.text())
        .then(result => alert('Light: ' + result));
    }
    </script>
</body>
</html>
    )rawliteral";
    
    html.replace("%HOSTNAME%", hostname + ".local");
    html.replace("%IP%", WiFi.softAPIP().toString());
    html.replace("%MAC%", WiFi.macAddress());
    
    request->send(200, "text/html", html);
  });
  
  server.on("/control", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("state")) {
      String state = request->getParam("state")->value();
      // Ваш код управления светом
      request->send(200, "text/plain", state.toUpperCase());
    }
  });
  
  server.begin();
  Serial.println("HTTP server started on port 80");
}

void printDeviceInfo() {
  Serial.println("\n═══════════════════════════════════════");
  Serial.println("           KITCHEN LIGHT READY");
  Serial.println("═══════════════════════════════════════");
  Serial.println("📶 AP Name: " + full_ap_ssid);
  Serial.println("🔑 Password: " + ap_password);
  Serial.println("🌐 Hostname: " + hostname + ".local");
  Serial.println("📍 IP: " + WiFi.softAPIP().toString());
  Serial.println("📟 MAC: " + WiFi.macAddress());
  Serial.println("═══════════════════════════════════════");
  Serial.println("💻 Access via: http://" + hostname + ".local");
  Serial.println("═══════════════════════════════════════\n");
}
Как теперь подключаться:
Подключитесь к WiFi: KLight_12:34:56:78:90:AB

Пароль: B1A203948576 (сгенерированный из MAC)

Откройте в браузере: http://klight-7890ab.local

Преимущества hostname:
Удобно - не нужно запоминать IP-адрес

Постоянно - IP может меняться, hostname - нет

Профессионально - как настоящее сетевое устройство

Легко идентифицировать - по имени видно какое это устройство

Теперь вы можете заходить на устройство по красивому URL вместо IP! 🎯

## переделать сохранение данных в EEPROM на эту библиотеку для более рационального использования флеш-памяти https://github.com/GyverLibs/FileData

FileData гораздо эффективнее использует флеш-память. Переделаем нашу систему хранения настроек:

Установка библиотеки
Сначала установите FileData через менеджер библиотек Arduino:

GyverLibs > FileData

Или через URL: https://github.com/GyverLibs/FileData

Переработанная система хранения настроек с FileData
cpp
#include <FileData.h>

class KeyValueStorage {
private:
    FileData _fileData;
    bool _initialized = false;
    bool _dirty = false; // Флаг изменений для отложенной записи

public:
    KeyValueStorage() : _fileData("/config.dat", 300, 'B') {} // 300 байт, бинарный режим
    
    bool begin() {
        if (!_initialized) {
            if (_fileData.init()) {
                Serial.println("KeyValueStorage initialized with FileData");
                _initialized = true;
                return true;
            } else {
                Serial.println("Failed to initialize FileData");
                return false;
            }
        }
        return true;
    }
    
    // Запись строкового значения
    bool putString(const char* key, const char* value) {
        if (!_initialized && !begin()) return false;
        
        String current = _fileData.readString(key, "");
        if (current != value) {
            _fileData.updateString(key, value);
            _dirty = true;
            return true;
        }
        return false; // Значение не изменилось
    }
    
    // Чтение строкового значения
    String getString(const char* key, const char* defaultValue = "") {
        if (!_initialized && !begin()) return String(defaultValue);
        return _fileData.readString(key, defaultValue);
    }
    
    // Запись целого числа
    bool putInt(const char* key, int value) {
        if (!_initialized && !begin()) return false;
        
        int current = _fileData.readInt(key, value - 1); // Гарантируем различие
        if (current != value) {
            _fileData.updateInt(key, value);
            _dirty = true;
            return true;
        }
        return false;
    }
    
    // Чтение целого числа
    int getInt(const char* key, int defaultValue = 0) {
        if (!_initialized && !begin()) return defaultValue;
        return _fileData.readInt(key, defaultValue);
    }
    
    // Запись булева значения
    bool putBool(const char* key, bool value) {
        return putInt(key, value ? 1 : 0);
    }
    
    // Чтение булева значения
    bool getBool(const char* key, bool defaultValue = false) {
        return getInt(key, defaultValue ? 1 : 0) != 0;
    }
    
    // Запись float
    bool putFloat(const char* key, float value) {
        if (!_initialized && !begin()) return false;
        
        float current = _fileData.readFloat(key, value - 1.0f);
        if (abs(current - value) > 0.001f) {
            _fileData.updateFloat(key, value);
            _dirty = true;
            return true;
        }
        return false;
    }
    
    // Чтение float
    float getFloat(const char* key, float defaultValue = 0.0) {
        if (!_initialized && !begin()) return defaultValue;
        return _fileData.readFloat(key, defaultValue);
    }
    
    // Удаление ключа
    bool remove(const char* key) {
        if (!_initialized && !begin()) return false;
        
        if (_fileData.checkKey(key)) {
            _fileData.remove(key);
            _dirty = true;
            return true;
        }
        return false;
    }
    
    // Принудительное сохранение изменений
    bool commit() {
        if (_dirty) {
            _dirty = false;
            return _fileData.updateNow();
        }
        return true;
    }
    
    // Очистка всех данных
    bool clear() {
        if (!_initialized && !begin()) return false;
        _dirty = true;
        return _fileData.removeAll();
    }
    
    // Автоматическое сохранение при разрушении объекта
    ~KeyValueStorage() {
        commit();
    }
    
    // Вывод всех ключей (для отладки)
    void listAll() {
        if (!_initialized && !begin()) return;
        
        Serial.println("=== KeyValueStorage Contents ===");
        Serial.printf("File size: %d bytes\n", _fileData.size());
        Serial.printf("Keys count: %d\n", _fileData.listSize());
        
        // FileData не имеет прямого метода для листинга ключей,
        // но мы можем проверить основные ключи
        String keys[] = {
            "wifi_ssid", "wifi_pass", "mqtt_server", "mqtt_port",
            "mqtt_user", "mqtt_pass", "device_name", "brightness", "auto_off"
        };
        
        for (const String& key : keys) {
            if (_fileData.checkKey(key.c_str())) {
                String value = _fileData.readString(key.c_str(), "[not set]");
                Serial.printf("  %s = %s\n", key.c_str(), value.c_str());
            }
        }
        Serial.println("=================================");
    }
};

// Глобальный экземпляр
KeyValueStorage Settings;
Оптимизированный класс настроек устройства
cpp
class DeviceSettings {
private:
    bool _settingsLoaded = false;
    unsigned long _lastAutoSave = 0;
    const unsigned long AUTO_SAVE_INTERVAL = 30000; // 30 секунд

public:
    // Ключи для хранения настроек
    struct Keys {
        static const char* WIFI_SSID;
        static const char* WIFI_PASSWORD;
        static const char* MQTT_SERVER;
        static const char* MQTT_PORT;
        static const char* MQTT_USER;
        static const char* MQTT_PASSWORD;
        static const char* DEVICE_NAME;
        static const char* BRIGHTNESS;
        static const char* AUTO_OFF;
        static const char* AP_PASSWORD;
        static const char* HOSTNAME;
    };

    DeviceSettings() = default;

    bool begin() {
        if (Settings.begin()) {
            loadDefaultsIfNeeded();
            _settingsLoaded = true;
            Serial.println("DeviceSettings initialized successfully");
            return true;
        }
        return false;
    }

    // Автоматическое сохранение в loop
    void update() {
        if (millis() - _lastAutoSave > AUTO_SAVE_INTERVAL) {
            Settings.commit();
            _lastAutoSave = millis();
        }
    }

    // Загрузка настроек по умолчанию при первом запуске
    void loadDefaultsIfNeeded() {
        if (Settings.getString(Keys::DEVICE_NAME).length() == 0) {
            Serial.println("First boot - loading default settings");
            
            // Генерируем уникальные значения на основе MAC
            String mac = WiFi.macAddress();
            mac.replace(":", "");
            String shortMac = mac.substring(6);
            
            setDeviceName("KitchenLight_" + shortMac);
            setWiFiCredentials("", "");
            setMQTTSettings("wqtt.ru", 1883, "", "");
            setBrightness(100);
            setAutoOff(false);
            
            // Генерируем пароль для точки доступа
            String apPassword = generatePasswordFromMAC(mac);
            setAPPassword(apPassword);
            
            // Генерируем hostname
            setHostname("klight-" + shortMac.toLowerCase());
            
            // Принудительно сохраняем настройки по умолчанию
            Settings.commit();
            
            Serial.println("Default settings loaded and saved");
        }
    }

    // WiFi настройки
    void setWiFiCredentials(const String& ssid, const String& password) {
        if (Settings.putString(Keys::WIFI_SSID, ssid.c_str()) ||
            Settings.putString(Keys::WIFI_PASSWORD, password.c_str())) {
            Serial.println("WiFi settings updated");
        }
    }

    String getWiFiSSID() { return Settings.getString(Keys::WIFI_SSID); }
    String getWiFiPassword() { return Settings.getString(Keys::WIFI_PASSWORD); }

    // MQTT настройки
    void setMQTTSettings(const String& server, int port, const String& user, const String& password) {
        bool changed = false;
        changed |= Settings.putString(Keys::MQTT_SERVER, server.c_str());
        changed |= Settings.putInt(Keys::MQTT_PORT, port);
        changed |= Settings.putString(Keys::MQTT_USER, user.c_str());
        changed |= Settings.putString(Keys::MQTT_PASSWORD, password.c_str());
        
        if (changed) {
            Serial.println("MQTT settings updated");
        }
    }

    String getMQTTServer() { return Settings.getString(Keys::MQTT_SERVER, "wqtt.ru"); }
    int getMQTTPort() { return Settings.getInt(Keys::MQTT_PORT, 1883); }
    String getMQTTUser() { return Settings.getString(Keys::MQTT_USER); }
    String getMQTTPassword() { return Settings.getString(Keys::MQTT_PASSWORD); }

    // Настройки устройства
    void setDeviceName(const String& name) {
        if (Settings.putString(Keys::DEVICE_NAME, name.c_str())) {
            Serial.println("Device name updated: " + name);
        }
    }

    String getDeviceName() { 
        return Settings.getString(Keys::DEVICE_NAME, "KitchenLight");
    }

    void setBrightness(int brightness) {
        if (Settings.putInt(Keys::BRIGHTNESS, brightness)) {
            Serial.println("Brightness updated: " + String(brightness));
        }
    }

    int getBrightness() {
        return Settings.getInt(Keys::BRIGHTNESS, 100);
    }

    void setAutoOff(bool enabled) {
        if (Settings.putBool(Keys::AUTO_OFF, enabled)) {
            Serial.println("Auto-off updated: " + String(enabled ? "enabled" : "disabled"));
        }
    }

    bool getAutoOff() {
        return Settings.getBool(Keys::AUTO_OFF, false);
    }

    // Пароль точки доступа
    void setAPPassword(const String& password) {
        Settings.putString(Keys::AP_PASSWORD, password.c_str());
    }

    String getAPPassword() {
        String password = Settings.getString(Keys::AP_PASSWORD);
        if (password.length() == 0) {
            // Генерируем новый если нет сохраненного
            String mac = WiFi.macAddress();
            password = generatePasswordFromMAC(mac);
            setAPPassword(password);
        }
        return password;
    }

    // Hostname
    void setHostname(const String& hostname) {
        Settings.putString(Keys::HOSTNAME, hostname.c_str());
    }

    String getHostname() {
        return Settings.getString(Keys::HOSTNAME, "klight");
    }

private:
    String generatePasswordFromMAC(const String& mac) {
        String clean_mac = mac;
        clean_mac.replace(":", "");
        String password = "";
        int start = 0;
        int end = clean_mac.length() - 1;
        while (start < end) {
            password += clean_mac[end--];
            password += clean_mac[start++];
        }
        return password;
    }

public:
    // Экспорт всех настроек
    void printAllSettings() {
        Serial.println("\n=== Device Settings ===");
        Serial.printf("Device Name: %s\n", getDeviceName().c_str());
        Serial.printf("Hostname: %s\n", getHostname().c_str());
        Serial.printf("WiFi SSID: %s\n", getWiFiSSID().c_str());
        Serial.printf("MQTT Server: %s:%d\n", getMQTTServer().c_str(), getMQTTPort());
        Serial.printf("Brightness: %d\n", getBrightness());
        Serial.printf("Auto Off: %s\n", getAutoOff() ? "enabled" : "disabled");
        Serial.printf("AP Password: %s\n", getAPPassword().c_str());
        Serial.println("=======================\n");
    }

    // Сброс к заводским настройкам
    void factoryReset() {
        Settings.clear();
        Settings.commit();
        _settingsLoaded = false;
        begin(); // Перезагрузит настройки по умолчанию
        Serial.println("Factory reset completed");
    }

    // Принудительное сохранение всех изменений
    void save() {
        Settings.commit();
        Serial.println("Settings saved to flash");
    }
};

// Определение статических констант
const char* DeviceSettings::Keys::WIFI_SSID = "wifi_ssid";
const char* DeviceSettings::Keys::WIFI_PASSWORD = "wifi_pass";
const char* DeviceSettings::Keys::MQTT_SERVER = "mqtt_server";
const char* DeviceSettings::Keys::MQTT_PORT = "mqtt_port";
const char* DeviceSettings::Keys::MQTT_USER = "mqtt_user";
const char* DeviceSettings::Keys::MQTT_PASSWORD = "mqtt_pass";
const char* DeviceSettings::Keys::DEVICE_NAME = "device_name";
const char* DeviceSettings::Keys::BRIGHTNESS = "brightness";
const char* DeviceSettings::Keys::AUTO_OFF = "auto_off";
const char* DeviceSettings::Keys::AP_PASSWORD = "ap_password";
const char* DeviceSettings::Keys::HOSTNAME = "hostname";

// Глобальный экземпляр
DeviceSettings Config;
Пример использования в проекте
cpp
void setup() {
    Serial.begin(115200);
    
    // Инициализация настроек
    if (!Config.begin()) {
        Serial.println("Failed to initialize settings!");
        return;
    }
    
    // Печатаем текущие настройки
    Config.printAllSettings();
    
    // Используем настройки
    setupWiFi();
    setupAccessPoint();
    setupWebServer();
}

void setupWiFi() {
    String ssid = Config.getWiFiSSID();
    String password = Config.getWiFiPassword();
    
    if (ssid.length() > 0) {
        WiFi.begin(ssid.c_str(), password.c_str());
        Serial.println("Connecting to WiFi: " + ssid);
    }
}

void setupAccessPoint() {
    String apPassword = Config.getAPPassword();
    String hostname = Config.getHostname();
    
    WiFi.softAP(Config.getDeviceName().c_str(), apPassword.c_str());
    WiFi.hostname(hostname.c_str());
    
    Serial.println("AP started: " + Config.getDeviceName());
    Serial.println("AP Password: " + apPassword);
}

void loop() {
    // Автоматическое сохранение настроек (если были изменения)
    Config.update();
    
    // Только чтение настроек
    int brightness = Config.getBrightness();
    String deviceName = Config.getDeviceName();
    
    // Ваша основная логика...
    delay(100);
}

// Обработчик веб-формы для сохранения настроек
void handleSaveSettings(const String& ssid, const String& password, int brightness) {
    Config.setWiFiCredentials(ssid, password);
    Config.setBrightness(brightness);
    Config.save(); // Принудительно сохраняем после важных изменений
}
Преимущества использования FileData:
Экономия памяти - использует файловую систему эффективнее

Автоматическое сжатие - данные хранятся компактно

Надежность - встроенные механизмы восстановления

Простота - не нужно управлять адресами EEPROM

Производительность - быстрое чтение/запись

Теперь ваши настройки хранятся максимально эффективно! 🚀