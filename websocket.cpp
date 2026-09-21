#include <Arduino.h>
#include <WebSocketsServer.h>

#include "websocket.h"
#include "config.h"
#include "servo.h"
#include "ultrasonic.h"
#include "motores.h"
#include "encoder.h"

WebSocketsServer webSocket(WEBSOCKET_PORT);

void webSocketEvent(
    uint8_t clientNum,
    WStype_t type,
    uint8_t* payload,
    size_t length
)
{
    if (type != WStype_TEXT)
        return;

    String message = String((char*)payload);

    Serial.println();
    Serial.println("================================");
    Serial.println("Mensaje recibido:");
    Serial.println(message);

    // SERVO
    if (
        message.indexOf("\"cmd\"") >= 0 &&
        message.indexOf("\"servo\"") >= 0
    )
    {
        int anglePosition = message.indexOf("\"angle\":");

        if (anglePosition == -1)
            anglePosition = message.indexOf("\"angle\": ");

        if (anglePosition == -1)
        {
            webSocket.sendTXT(
                clientNum,
                "{\"ok\":false,\"error\":\"missing_angle\"}"
            );
            return;
        }

        int angle = message.substring(
            anglePosition + 8
        ).toInt();

        if (angle < 0 || angle > 180)
        {
            webSocket.sendTXT(
                clientNum,
                "{\"ok\":false,\"error\":\"angle_out_of_range\"}"
            );
            return;
        }

        if (!moveServo(angle))
        {
            webSocket.sendTXT(
                clientNum,
                "{\"ok\":false,\"error\":\"servo_error\"}"
            );
            return;
        }

        String response =
            "{\"ok\":true,"
            "\"cmd\":\"servo\","
            "\"angle\":" +
            String(getServoAngle()) +
            "}";

        webSocket.sendTXT(clientNum, response);

        Serial.print("Servo movido a ");
        Serial.print(getServoAngle());
        Serial.println(" grados");

        return;
    }

    // DISTANCIA
    if (
        message.indexOf("\"cmd\"") >= 0 &&
        message.indexOf("\"distance\"") >= 0
    )
    {
        long distance = readDistance();

        String response =
            "{\"ok\":true,"
            "\"cmd\":\"distance\","
            "\"distance\":" +
            String(distance) +
            "}";

        webSocket.sendTXT(clientNum, response);

        Serial.print("Distancia: ");
        Serial.print(distance);
        Serial.println(" cm");

        return;
    }

    // MOVIMIENTO EN CENTIMETROS
    if (
        message.indexOf("\"cmd\"") >= 0 &&
        message.indexOf("\"move_cm\"") >= 0
    )
    {
        int distancePosition = message.indexOf("\"distance\":");

        if (distancePosition == -1)
        {
            webSocket.sendTXT(
                clientNum,
                "{\"ok\":false,\"error\":\"missing_distance\"}"
            );
            return;
        }

        float distance = message.substring(
            distancePosition + 11
        ).toFloat();

        if (distance <= 0)
        {
            webSocket.sendTXT(
                clientNum,
                "{\"ok\":false,\"error\":\"invalid_distance\"}"
            );
            return;
        }

        // Aquí posteriormente llamaremos
        // a la función que mueve X centímetros.

        Serial.print("Movimiento solicitado: ");
        Serial.print(distance);
        Serial.println(" cm");

        webSocket.sendTXT(
            clientNum,
            "{\"ok\":true,\"cmd\":\"move_cm\"}"
        );

        return;
    }

    // PING
    if (
        message.indexOf("\"cmd\"") >= 0 &&
        message.indexOf("\"ping\"") >= 0
    )
    {
        webSocket.sendTXT(
            clientNum,
            "{\"ok\":true,\"cmd\":\"ping\"}"
        );
        return;
    }

    webSocket.sendTXT(
        clientNum,
        "{\"ok\":false,\"error\":\"unknown_command\"}"
    );
}

void initWebSocket()
{
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    Serial.println("WebSocket iniciado.");
    Serial.print("Puerto: ");
    Serial.println(WEBSOCKET_PORT);
}

void handleWebSocket()
{
    webSocket.loop();
}