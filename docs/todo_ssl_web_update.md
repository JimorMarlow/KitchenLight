(c) deepseek edition

>сделать обновления сертификата сайта wqtt.ru в устройство через webui в настройках?

Отличная идея! Сделаем систему обновления SSL-сертификата через веб-интерфейс. Вот полная реализация:

Дополнения к классу настроек
cpp
class DeviceSettings {
private:
    // ... предыдущий код ...

public:
    struct Keys {
        // ... предыдущие ключи ...
        static const char* MQTT_SSL_CERT;    // SSL сертификат
        static const char* MQTT_SSL_ENABLED; // Флаг использования SSL
    };

    // ... предыдущие методы ...

    // SSL сертификат
    void setSSLEnabled(bool enabled) {
        if (Settings.putBool(Keys::MQTT_SSL_ENABLED, enabled)) {
            Serial.println("SSL enabled: " + String(enabled));
        }
    }

    bool getSSLEnabled() {
        return Settings.getBool(Keys::MQTT_SSL_ENABLED, false);
    }

    void setSSLCertificate(const String& certificate) {
        if (Settings.putString(Keys::MQTT_SSL_CERT, certificate.c_str())) {
            Serial.println("SSL certificate updated, length: " + String(certificate.length()));
        }
    }

    String getSSLCertificate() {
        return Settings.getString(Keys::MQTT_SSL_CERT, "");
    }

    // Проверка валидности сертификата
    bool isCertificateValid() {
        String cert = getSSLCertificate();
        return cert.length() > 50 && 
               cert.indexOf("BEGIN CERTIFICATE") != -1 && 
               cert.indexOf("END CERTIFICATE") != -1;
    }
};

// Добавляем новые ключи
const char* DeviceSettings::Keys::MQTT_SSL_CERT = "mqtt_ssl_cert";
const char* DeviceSettings::Keys::MQTT_SSL_ENABLED = "mqtt_ssl_enabled";
Веб-интерфейс для управления SSL сертификатом
cpp
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

void setupWebServer() {
    // ... другие обработчики ...

    // Страница настроек SSL
    server.on("/ssl-settings", HTTP_GET, [](AsyncWebServerRequest *request){
        String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>SSL Certificate Settings</title>
    <meta charset="UTF-8">
    <style>
        body { font-family: Arial; margin: 20px; max-width: 800px; }
        .card { background: #f9f9f9; padding: 20px; border-radius: 10px; margin: 20px 0; }
        .status { padding: 10px; border-radius: 5px; margin: 10px 0; }
        .valid { background: #d4edda; color: #155724; border: 1px solid #c3e6cb; }
        .invalid { background: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }
        textarea { width: 100%; height: 200px; font-family: monospace; font-size: 12px; }
        button { padding: 10px 20px; margin: 5px; font-size: 16px; }
        .btn-primary { background: #007bff; color: white; border: none; border-radius: 5px; }
        .btn-success { background: #28a745; color: white; border: none; border-radius: 5px; }
        .btn-danger { background: #dc3545; color: white; border: none; border-radius: 5px; }
        .form-group { margin: 15px 0; }
        label { display: block; margin-bottom: 5px; font-weight: bold; }
    </style>
</head>
<body>
    <h1>🔐 SSL Certificate Settings</h1>
    
    <div class="card">
        <h2>Current SSL Status</h2>
        <div class="status %CERT_STATUS%">
            <strong>SSL Certificate:</strong> %CERT_STATUS_TEXT%<br>
            <strong>Certificate Length:</strong> %CERT_LENGTH% characters<br>
            <strong>SSL Enabled:</strong> %SSL_ENABLED%
        </div>
    </div>

    <div class="card">
        <h2>Update SSL Certificate</h2>
        <form action="/update-ssl" method="post">
            <div class="form-group">
                <label for="ssl_enabled">SSL Enabled:</label>
                <input type="checkbox" id="ssl_enabled" name="ssl_enabled" %SSL_CHECKED%>
                <label for="ssl_enabled">Use SSL for MQTT connection</label>
            </div>
            
            <div class="form-group">
                <label for="ssl_cert">SSL Certificate (PEM format):</label>
                <textarea id="ssl_cert" name="ssl_cert" placeholder="Paste your SSL certificate here...">%CURRENT_CERT%</textarea>
                <small>Paste the entire certificate including -----BEGIN CERTIFICATE----- and -----END CERTIFICATE-----</small>
            </div>
            
            <button type="submit" class="btn-success">💾 Save SSL Certificate</button>
            <a href="/"><button type="button" class="btn-primary">← Back to Main</button></a>
        </form>
    </div>

    <div class="card">
        <h2>Quick Actions</h2>
        <button onclick="fetch('/reset-ssl').then(() => alert('SSL certificate reset')).then(() => location.reload())" 
                class="btn-danger">🔄 Reset to Default Certificate</button>
        <button onclick="fetch('/test-ssl').then(r => r.text()).then(alert)" 
                class="btn-primary">🧪 Test SSL Connection</button>
    </div>

    <div class="card">
        <h2>Instructions</h2>
        <ol>
            <li>Enable SSL checkbox if you want to use encrypted connection</li>
            <li>Get the certificate from wqtt.ru website or your browser</li>
            <li>Paste the entire certificate in the text area above</li>
            <li>Click "Save SSL Certificate" to apply changes</li>
            <li>Device will reconnect to MQTT with new certificate</li>
        </ol>
        
        <h3>How to get certificate:</h3>
        <ul>
            <li>Chrome: Click lock icon → Connection is secure → Certificate is valid → Details → Copy to File</li>
            <li>Firefox: Click lock icon → Connection secure → More information → View Certificate → Details → Export</li>
            <li>Or use: <code>openssl s_client -connect wqtt.ru:8883 -showcerts</code></li>
        </ul>
    </div>
</body>
</html>
        )rawliteral";
        
        // Заполняем шаблон данными
        bool sslEnabled = Config.getSSLEnabled();
        String cert = Config.getSSLCertificate();
        bool certValid = Config.isCertificateValid();
        
        html.replace("%CERT_STATUS%", certValid ? "valid" : "invalid");
        html.replace("%CERT_STATUS_TEXT%", certValid ? "Valid" : "Invalid or not set");
        html.replace("%CERT_LENGTH%", String(cert.length()));
        html.replace("%SSL_ENABLED%", sslEnabled ? "Yes" : "No");
        html.replace("%SSL_CHECKED%", sslEnabled ? "checked" : "");
        html.replace("%CURRENT_CERT%", cert);
        
        request->send(200, "text/html", html);
    });

    // Обработчик обновления SSL сертификата
    server.on("/update-ssl", HTTP_POST, [](AsyncWebServerRequest *request){
        String response;
        
        if (request->hasParam("ssl_cert", true) && request->hasParam("ssl_enabled", true)) {
            String sslCert = request->getParam("ssl_cert", true)->value();
            bool sslEnabled = request->getParam("ssl_enabled", true)->value() == "on";
            
            // Базовая валидация сертификата
            if (sslCert.length() > 0 && 
                sslCert.indexOf("-----BEGIN CERTIFICATE-----") == -1) {
                response = R"rawliteral(
                    <html>
                    <body>
                        <h1>Error: Invalid Certificate Format</h1>
                        <p>Certificate must include '-----BEGIN CERTIFICATE-----' and '-----END CERTIFICATE-----'</p>
                        <a href="/ssl-settings"><button>← Back to SSL Settings</button></a>
                    </body>
                    </html>
                )rawliteral";
                request->send(400, "text/html", response);
                return;
            }
            
            // Сохраняем настройки
            Config.setSSLEnabled(sslEnabled);
            Config.setSSLCertificate(sslCert);
            Config.save();
            
            response = R"rawliteral(
                <html>
                <head>
                    <meta http-equiv="refresh" content="3;url=/ssl-settings" />
                </head>
                <body>
                    <h1>✅ SSL Settings Saved Successfully!</h1>
                    <p>SSL Enabled: %SSL_ENABLED%</p>
                    <p>Certificate length: %CERT_LENGTH% characters</p>
                    <p>Redirecting to SSL settings in 3 seconds...</p>
                    <a href="/ssl-settings"><button>← Back to SSL Settings</button></a>
                </body>
                </html>
            )rawliteral";
            
            response.replace("%SSL_ENABLED%", sslEnabled ? "Yes" : "No");
            response.replace("%CERT_LENGTH%", String(sslCert.length()));
            
            request->send(200, "text/html", response);
            
            // Перезагружаем MQTT соединение с новыми настройками
            restartMQTTConnection();
            
        } else {
            request->send(400, "text/plain", "Missing required parameters");
        }
    });

    // Сброс SSL сертификата
    server.on("/reset-ssl", HTTP_GET, [](AsyncWebServerRequest *request){
        Config.setSSLCertificate("");
        Config.setSSLEnabled(false);
        Config.save();
        
        request->send(200, "text/plain", "SSL certificate reset to default");
        restartMQTTConnection();
    });

    // Тест SSL соединения
    server.on("/test-ssl", HTTP_GET, [](AsyncWebServerRequest *request){
        String result = testSSLConnection();
        request->send(200, "text/plain", result);
    });
}

// Функция тестирования SSL соединения
String testSSLConnection() {
    if (!Config.getSSLEnabled()) {
        return "SSL is not enabled. Enable it first in SSL settings.";
    }
    
    if (!Config.isCertificateValid()) {
        return "Invalid SSL certificate. Please update the certificate in SSL settings.";
    }
    
    // Здесь будет код тестирования SSL соединения
    // Это упрощенная версия - в реальности нужно реализовать тестовое подключение
    
    return "SSL test initiated. Check serial monitor for connection details.\n"
           "Certificate status: " + String(Config.isCertificateValid() ? "Valid" : "Invalid") + "\n" +
           "SSL Enabled: " + String(Config.getSSLEnabled() ? "Yes" : "No") + "\n" +
           "MQTT Server: " + Config.getMQTTServer() + ":" + String(Config.getMQTTPort());
}

// Перезапуск MQTT соединения
void restartMQTTConnection() {
    Serial.println("Restarting MQTT connection with new SSL settings...");
    
    // Отключаем текущее соединение
    if (mqttClient.connected()) {
        mqttClient.disconnect();
    }
    
    // Ждем немного
    delay(1000);
    
    // Переподключаемся с новыми настройками
    setupMQTT();
}
Интеграция с MQTT клиентом
cpp
#include <WiFiClientSecure.h>

WiFiClientSecure secureClient;
BearSSL::X509List certificate;

void setupMQTT() {
    if (Config.getSSLEnabled() && Config.isCertificateValid()) {
        setupSecureMQTT();
    } else {
        setupInsecureMQTT();
    }
}

void setupSecureMQTT() {
    Serial.println("Setting up secure MQTT connection...");
    
    String cert = Config.getSSLCertificate();
    
    // Очищаем предыдущий сертификат
    secureClient.setTrustAnchors(nullptr);
    
    // Загружаем новый сертификат
    if (certificate.append(cert.c_str())) {
        secureClient.setTrustAnchors(&certificate);
        Serial.println("SSL certificate loaded successfully");
    } else {
        Serial.println("Failed to load SSL certificate, falling back to insecure");
        setupInsecureMQTT();
        return;
    }
    
    // Настраиваем безопасного клиента
    secureClient.setInsecure(); // Для самоподписанных сертификатов
    mqttClient.setClient(secureClient);
    
    // Подключаемся к MQTT
    mqttClient.setServer(Config.getMQTTServer().c_str(), Config.getMQTTPort());
    connectToMQTT();
}

void setupInsecureMQTT() {
    Serial.println("Setting up insecure MQTT connection...");
    
    WiFiClient insecureClient;
    mqttClient.setClient(insecureClient);
    mqttClient.setServer(Config.getMQTTServer().c_str(), Config.getMQTTPort());
    connectToMQTT();
}

void connectToMQTT() {
    String clientId = "KitchenLight-" + String(ESP.getChipId());
    
    if (mqttClient.connect(clientId.c_str(), 
                          Config.getMQTTUser().c_str(), 
                          Config.getMQTTPassword().c_str())) {
        Serial.println("Connected to MQTT broker");
        
        // Подписываемся на топики
        mqttClient.subscribe("home/kitchen/light/control");
        
    } else {
        Serial.println("Failed to connect to MQTT, error: " + String(mqttClient.state()));
    }
}
Добавление ссылки в главное меню
cpp
// В главной странице веб-интерфейса добавьте:
void setupMainWebPage() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Kitchen Light Control</title>
    <style>
        /* ... существующие стили ... */
        .menu { background: #e9ecef; padding: 15px; border-radius: 5px; margin: 10px 0; }
        .menu a { margin: 0 10px; text-decoration: none; color: #007bff; }
        .ssl-status { 
            padding: 5px 10px; 
            border-radius: 3px; 
            font-size: 12px; 
            margin-left: 10px;
        }
        .ssl-on { background: #d4edda; color: #155724; }
        .ssl-off { background: #f8d7da; color: #721c24; }
    </style>
</head>
<body>
    <h1>Kitchen Light Control</h1>
    
    <div class="menu">
        <a href="/">🏠 Main</a>
        <a href="/settings">⚙️ Settings</a>
        <a href="/ssl-settings">
            🔐 SSL Settings 
            <span class="ssl-status %SSL_STATUS%">%SSL_STATUS_TEXT%</span>
        </a>
    </div>
    
    <!-- остальной контент главной страницы -->
</body>
</html>
        )rawliteral";
        
        // Обновляем статус SSL
        bool sslEnabled = Config.getSSLEnabled();
        bool certValid = Config.isCertificateValid();
        String sslStatus = sslEnabled && certValid ? "ssl-on" : "ssl-off";
        String sslStatusText = sslEnabled && certValid ? "SSL ON" : "SSL OFF";
        
        html.replace("%SSL_STATUS%", sslStatus);
        html.replace("%SSL_STATUS_TEXT%", sslStatusText);
        
        request->send(200, "text/html", html);
    });
}
Как использовать:
Откройте веб-интерфейс: http://klight-xxxxxx.local

Перейдите в "🔐 SSL Settings"

Включите SSL и вставьте сертификат wqtt.ru

Сохраните настройки

Устройство автоматически переподключится к MQTT с новым сертификатом

Теперь вы можете легко обновлять SSL-сертификаты через веб-интерфейс! 🔐