#include "ultrasonic.h"
#include "config.h"
#include "servo.h"
#include "websocket.h"
#include "encoder.h"
#include "motors.h"

#include <WiFi.h>
#include <WiFiManager.h>

// ========================================
// SETUP
// ========================================

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("================================");
    Serial.println("        ROBOT ESP32");
    Serial.println("================================");

    // ====================================
    // HOSTNAME
    // ====================================

    WiFi.setHostname(HOSTNAME);
    Serial.print("Hostname: ");
    Serial.println(HOSTNAME);

    // ====================================
    // SERVO
    // ====================================

    Serial.println();

    Serial.println("Inicializando servo...");
    initServo();
    Serial.println("Servo en 90 grados");


    // ====================================
    // ULTRASONICO
    // ====================================

    Serial.println();

    Serial.println("Inicializando SR04...");
    initUltrasonic();
    Serial.println("TRIG: GPIO 5");

    Serial.println("ECHO: GPIO 19");


    // ====================================
    // ENCODERS
    // ====================================

    Serial.println();
    Serial.println("Inicializando encoders...");
    initEncoders();
    Serial.println("Encoders listos.");


    // ====================================
    // MOTORES
    // ====================================

    Serial.println();
    Serial.println("Inicializando motores...");
    initMotors();
    Serial.println("Motores listos.");


    // ====================================
    // WIFI MANAGER
    // ====================================

    WiFiManager wifiManager;
    Serial.println();
    Serial.println("Conectando a WiFi...");

    bool connected =
        wifiManager.autoConnect(
            "ROBOT-SETUP"
        );

    if (!connected)
    {
        Serial.println();

        Serial.println(
            "No se pudo conectar al WiFi."
        );

        Serial.println(
            "Reiniciando..."
        );


        delay(3000);

        ESP.restart();
    }


    // ====================================
    // WIFI CONECTADO
    // ====================================

    Serial.println();

    Serial.println("================================");

    Serial.println("WiFi conectado!");

    Serial.println("================================");


    Serial.print("SSID: ");

    Serial.println(
        WiFi.SSID()
    );


    Serial.print("IP: ");

    Serial.println(
        WiFi.localIP()
    );


    Serial.print("Hostname: ");

    Serial.println(
        WiFi.getHostname()
    );


    // ====================================
    // WEBSOCKET
    // ====================================

    Serial.println();

    Serial.println(
        "Iniciando WebSocket..."
    );
    initWebSocket();


    Serial.println(
        "WebSocket iniciado."
    );


    Serial.print("Puerto: ");

    Serial.println(
        WEBSOCKET_PORT
    );


    // ====================================
    // LISTO
    // ====================================

    Serial.println();

    Serial.println("================================");

    Serial.println("        ROBOT LISTO");

    Serial.println("================================");


    Serial.println();

    Serial.println(
        "Servo: GPIO 18"
    );

    Serial.println(
        "TRIG: GPIO 5"
    );

    Serial.println(
        "ECHO: GPIO 19"
    );

    Serial.println();

    Serial.println(
        "Esperando comandos..."
    );
}


// ========================================
// LOOP
// ========================================

void loop()
{
    handleWebSocket();
}