#include <Arduino.h>
#include <WebSocketsServer.h>

#include "websocket.h"
#include "config.h"
#include "servo.h"
#include "ultrasonic.h"
#include "motors.h"
#include "encoder.h"

WebSocketsServer webSocket(WEBSOCKET_PORT);


// Ayuda para leer un campo numérico de un JSON compacto.
// Devuelve true si lo encontró y escribe el valor en `outValue`.
// Tolera espacios después del ":" (por ejemplo, "angle": 90).
static bool extractNumber(
    const String& message,
    const String& key,
    float& outValue
)
{
    String pattern = "\"" + key + "\"";

    int keyPos = message.indexOf(pattern);

    if (keyPos == -1)
        return false;

    int colonPos = message.indexOf(':', keyPos);

    if (colonPos == -1)
        return false;

    int start = colonPos + 1;

    while (start < (int)message.length() && message[start] == ' ')
        start++;

    int end = start;

    while (
        end < (int)message.length() &&
        (
            isdigit(message[end]) ||
            message[end] == '.' ||
            message[end] == '-'
        )
    )
        end++;

    if (start == end)
        return false;

    outValue = message.substring(start, end).toFloat();
    return true;
}


// Extrae el valor (entre comillas) de un campo string de un JSON compacto.
static bool extractString(
    const String& message,
    const String& key,
    String& outValue
)
{
    String pattern = "\"" + key + "\"";

    int keyPos = message.indexOf(pattern);

    if (keyPos == -1)
        return false;

    int colonPos = message.indexOf(':', keyPos);

    if (colonPos == -1)
        return false;

    int firstQuote = message.indexOf('"', colonPos + 1);

    if (firstQuote == -1)
        return false;

    int secondQuote = message.indexOf('"', firstQuote + 1);

    if (secondQuote == -1)
        return false;

    outValue = message.substring(firstQuote + 1, secondQuote);
    return true;
}


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
        float angleFloat;

        if (!extractNumber(message, "angle", angleFloat))
        {
            webSocket.sendTXT(
                clientNum,
                "{\"ok\":false,\"error\":\"missing_angle\"}"
            );
            return;
        }

        int angle = (int)angleFloat;

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
        message.indexOf("\"distance\"") >= 0 &&
        message.indexOf("\"move_cm\"") < 0
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

    // MOVIMIENTO EN CENTÍMETROS
    if (
        message.indexOf("\"cmd\"") >= 0 &&
        message.indexOf("\"move_cm\"") >= 0
    )
    {
        float distance;

        if (!extractNumber(message, "distance", distance))
        {
            webSocket.sendTXT(
                clientNum,
                "{\"ok\":false,\"error\":\"missing_distance\"}"
            );
            return;
        }

        if (distance <= 0)
        {
            webSocket.sendTXT(
                clientNum,
                "{\"ok\":false,\"error\":\"invalid_distance\"}"
            );
            return;
        }

        // Dirección: por defecto adelante.
        String direction = "forward";
        extractString(message, "direction", direction);

        bool goForward = true;

        if (direction == "backward" || direction == "back")
        {
            goForward = false;
        }
        else if (direction != "forward" && direction != "fwd")
        {
            webSocket.sendTXT(
                clientNum,
                "{\"ok\":false,\"error\":\"invalid_direction\"}"
            );
            return;
        }

        Serial.print("Moviendo hacia ");
        Serial.print(goForward ? "adelante" : "atras");
        Serial.print(" ");
        Serial.print(distance);
        Serial.println(" cm");

        bool finished = moveDistanceCm(goForward, distance);

        long leftPulses = getLeftPulses();
        long rightPulses = getRightPulses();

        String response =
            "{\"ok\":" +
            String(finished ? "true" : "false") +
            ",\"cmd\":\"move_cm\","
            "\"direction\":\"" +
            (goForward ? "forward" : "backward") +
            "\",\"distance\":" +
            String(distance) +
            ",\"left_pulses\":" +
            String(leftPulses) +
            ",\"right_pulses\":" +
            String(rightPulses) +
            (finished ? "" : ",\"error\":\"timeout\"") +
            "}";

        webSocket.sendTXT(clientNum, response);
        return;
    }

    // ESCANEO DE RANGO
    // Realiza el barrido completo del servo y las lecturas de
    // distancia dentro del propio ESP32, devolviendo todas las
    // mediciones en una sola respuesta. Reduce mucho la latencia
    // frente a mandar N comandos servo + distance desde el cliente.
    if (
        message.indexOf("\"cmd\"") >= 0 &&
        message.indexOf("\"scan_range\"") >= 0
    )
    {
        float startAngle = 0;
        float endAngle = 180;
        float step = 10;

        extractNumber(message, "start", startAngle);
        extractNumber(message, "end", endAngle);
        extractNumber(message, "step", step);

        if (step <= 0)
            step = 10;

        // Normalizar rango.
        if (startAngle < 0) startAngle = 0;
        if (endAngle > 180) endAngle = 180;
        if (endAngle < startAngle)
        {
            float tmp = startAngle;
            startAngle = endAngle;
            endAngle = tmp;
        }

        String measurements = "[";
        bool first = true;

        for (
            float angle = startAngle;
            angle <= endAngle;
            angle += step
        )
        {
            moveServo((int)angle);
            delay(120);

            long distance = readDistance();

            if (!first)
                measurements += ",";

            measurements += "{\"angle\":";
            measurements += String((int)angle);
            measurements += ",\"distance\":";
            measurements += String(distance);
            measurements += "}";

            first = false;
        }

        measurements += "]";

        // Deja el servo mirando al frente al terminar.
        moveServo(90);

        String response =
            "{\"ok\":true,"
            "\"cmd\":\"scan_range\","
            "\"measurements\":" +
            measurements +
            "}";

        webSocket.sendTXT(clientNum, response);
        return;
    }

    // GIRO EN EL SITIO
    if (
        message.indexOf("\"cmd\"") >= 0 &&
        message.indexOf("\"turn\"") >= 0
    )
    {
        String direction = "left";
        extractString(message, "direction", direction);

        bool goLeft = true;

        if (direction == "right")
        {
            goLeft = false;
        }
        else if (direction != "left")
        {
            webSocket.sendTXT(
                clientNum,
                "{\"ok\":false,\"error\":\"invalid_direction\"}"
            );
            return;
        }

        float ms;

        if (!extractNumber(message, "ms", ms))
        {
            webSocket.sendTXT(
                clientNum,
                "{\"ok\":false,\"error\":\"missing_ms\"}"
            );
            return;
        }

        if (ms <= 0)
        {
            webSocket.sendTXT(
                clientNum,
                "{\"ok\":false,\"error\":\"invalid_ms\"}"
            );
            return;
        }

        Serial.print("Girando hacia la ");
        Serial.print(goLeft ? "izquierda" : "derecha");
        Serial.print(" durante ");
        Serial.print((unsigned long)ms);
        Serial.println(" ms");

        bool finished = turnMs(goLeft, (unsigned long)ms);

        String response =
            "{\"ok\":" +
            String(finished ? "true" : "false") +
            ",\"cmd\":\"turn\","
            "\"direction\":\"" +
            (goLeft ? "left" : "right") +
            "\",\"ms\":" +
            String((unsigned long)ms) +
            "}";

        webSocket.sendTXT(clientNum, response);
        return;
    }

    // STOP
    if (
        message.indexOf("\"cmd\"") >= 0 &&
        message.indexOf("\"stop\"") >= 0
    )
    {
        stopMotors();

        webSocket.sendTXT(
            clientNum,
            "{\"ok\":true,\"cmd\":\"stop\"}"
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
