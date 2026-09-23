# Arduino-Robot

Firmware para un robot con **ESP32** controlable por **WebSocket**.
El robot lleva dos motores DC con encoders, un servo con un sensor
ultrasónico HC-SR04 encima, y se conecta a la red WiFi por medio de
WiFiManager.

Todo el control se hace enviando mensajes JSON al puerto WebSocket
`81`. El cliente típico es el proyecto hermano **Python-llm**.

---

## 🧩 Componentes

| Cantidad | Componente | Nota |
|---|---|---|
| 1 | Placa **ESP32** (DevKit v1, WROOM, etc.) | Cerebro y WiFi |
| 1 | Puente H **L298N** (o TB6612FNG) | Control de los dos motores |
| 2 | Motores **DC** con reductora y **encoders** | Las ruedas |
| 1 | Servo **SG90** | Gira la cabeza del sensor |
| 1 | Sensor ultrasónico **HC-SR04** | Distancia al frente |
| 1 | Fuente de motores (**batería 7.4 V** o pack 4×AA) | **No los alimentes por USB** |
| — | Cables Dupont, chasis, ruedas, protoboard opcional | |

---

## 🔌 Diagrama de conexiones (pinout ESP32)

Los pines están definidos en [`config.h`](config.h). Si necesitas otros,
cámbialos ahí antes de flashear.

```
                        ┌──────────────────────────┐
                        │           ESP32          │
                        │                          │
     Servo signal   ────┤ GPIO 18  (SERVO_PIN)     │
                        │                          │
     HC-SR04 TRIG   ────┤ GPIO 5   (TRIG_PIN)      │
     HC-SR04 ECHO   ────┤ GPIO 19  (ECHO_PIN)      │
                        │                          │
     L298N IN1 (L)  ────┤ GPIO 25  (MOTOR_L_IN1)   │
     L298N IN2 (L)  ────┤ GPIO 26  (MOTOR_L_IN2)   │
     L298N IN3 (R)  ────┤ GPIO 27  (MOTOR_R_IN1)   │
     L298N IN4 (R)  ────┤ GPIO 14  (MOTOR_R_IN2)   │
                        │                          │
     Encoder izq A  ────┤ GPIO 32  (ENCODER_L)     │
     Encoder der A  ────┤ GPIO 33  (ENCODER_R)     │
                        │                          │
     5V salida      ────┤ 5V     ── al SG90 y HC-SR04
     3.3V           ────┤ 3.3V   ── VCC de encoders (si lo aceptan)
     GND            ────┤ GND    ── GND común con TODO
                        └──────────────────────────┘
```

### Reglas eléctricas importantes

1. **Los motores NUNCA se alimentan por USB.** Usa una batería aparte
   (7.4 V para el L298N, o 5 V dependiendo del driver) conectada a
   `VS`/`VM` del puente H.
2. **GND común**: todos los GND (ESP32, puente H, batería de motores,
   sensor, servo) deben estar unidos.
3. **Servo y HC-SR04**: se alimentan con 5 V del ESP32 si el USB
   entrega suficiente corriente; si el servo tiembla, alimentarlo
   aparte y compartir GND.
4. **Encoders**: usa GPIOs con pull-up interno. Los pines por defecto
   son 32 y 33. **Evita 34/35/36/39** porque son input-only y
   `INPUT_PULLUP` no funciona en esos pines.
5. Si tu L298N tiene los jumpers **ENA/ENB**, déjalos puestos (los
   motores irán a velocidad máxima). Si tu driver tiene PWM, este
   firmware aún no lo usa — se puede añadir después.

---

## 📁 Estructura del proyecto

```
Arduino-Robot/
├── robot/
│   └── robot.ino          ← sketch principal (setup / loop)
├── config.h               ← pines y calibración
├── servo.h / .cpp         ← control del servo
├── ultrasonic.h / .cpp    ← lectura del HC-SR04
├── motors.h / .cpp        ← movimiento y giros
├── encoder.h / .cpp       ← lectura de encoders por ISR
├── websocket.h / .cpp     ← servidor WebSocket y parser de comandos
└── README.md
```

> **Nota sobre Arduino IDE clásico**: el IDE espera que todos los
> archivos `.h`/`.cpp` estén en la **misma carpeta** que el `.ino`.
> Actualmente los headers están un nivel arriba. Si compilas con
> Arduino IDE, mueve todos los `.h`/`.cpp` a `robot/`. Si usas
> **PlatformIO** o `arduino-cli` con configuración custom, no hay
> problema con la estructura actual.

---

## 📚 Librerías necesarias

Instálalas desde el **Library Manager** del Arduino IDE:

| Librería | Autor | Uso |
|---|---|---|
| **WiFiManager** | tzapu | Portal cautivo para conectar a WiFi |
| **WebSockets** | Markus Sattler | `WebSocketsServer` |
| **ESP32Servo** | Kevin Harrington | Servo en ESP32 |

Y por supuesto el core del **ESP32** de Espressif en el Board Manager.

---

## ⚙️ Configuración

Todo está en [`config.h`](config.h). Los que probablemente tengas
que ajustar:

| Constante | Valor por defecto | Qué es |
|---|---|---|
| `HOSTNAME` | `"robot"` | Nombre del robot en la red |
| `WEBSOCKET_PORT` | `81` | Puerto del servidor |
| `ENCODER_L` / `ENCODER_R` | 32 / 33 | Pines de encoders |
| `PULSES_PER_CM` | `1.0f` | **Calibrar** midiendo pulsos por cm |
| `MOVE_TIMEOUT_MS` | `8000` | Corta un movimiento si tarda demasiado |

### Calibrar `PULSES_PER_CM`

1. Sube el firmware con el valor por defecto.
2. Manda por WebSocket: `{"cmd":"move_cm","direction":"forward","distance":100}`.
3. Mide con regla cuántos cm avanzó realmente.
4. Ajusta:
   ```
   PULSES_PER_CM_nuevo = left_pulses / cm_reales_medidos
   ```
   Los `left_pulses` vienen en la respuesta del ESP32.

---

## 🚀 Cómo compilar y flashear

1. Abre `robot/robot.ino` en el Arduino IDE.
2. Selecciona la placa: **Tools → Board → ESP32 Dev Module**.
3. Selecciona el puerto: **Tools → Port → /dev/ttyUSB0** (o similar).
4. Presiona **Upload**.
5. Abre el **Serial Monitor** a **115200 baud**.

La primera vez, el ESP32 arrancará en modo AP y creará una red WiFi
llamada **`ROBOT-SETUP`**. Conéctate desde tu teléfono y dale las
credenciales de tu WiFi. Después de eso, siempre se conectará solo.

En el Serial Monitor verás algo como:

```
================================
        ROBOT ESP32
================================
Inicializando servo...
Inicializando SR04...
Inicializando encoders...
Inicializando motores...
Conectando a WiFi...
WiFi conectado!
SSID: MiRed
IP: 192.168.100.23         ← ¡esta IP es la que necesita Python!
WebSocket iniciado.
Puerto: 81
        ROBOT LISTO
```

**Anota la IP** para configurarla del lado Python.

---

## 📡 Protocolo WebSocket

Toda la comunicación son mensajes JSON compactos.

### Comandos que acepta el robot

| Comando | Ejemplo | Descripción |
|---|---|---|
| `servo` | `{"cmd":"servo","angle":45}` | Mueve el servo a un ángulo 0-180 |
| `distance` | `{"cmd":"distance"}` | Lee el HC-SR04 |
| `move_cm` | `{"cmd":"move_cm","direction":"forward","distance":20}` | Avanza N cm (`forward` o `backward`) |
| `turn` | `{"cmd":"turn","direction":"left","ms":500}` | Gira en el sitio N ms (`left` o `right`) |
| `stop` | `{"cmd":"stop"}` | Detiene los motores |
| `ping` | `{"cmd":"ping"}` | Prueba de conexión |

### Respuestas

Todas responden con un JSON que incluye `"ok": true|false`:

```json
{"ok":true,"cmd":"move_cm","direction":"forward","distance":20.00,
 "left_pulses":20,"right_pulses":19}
```

En caso de error:

```json
{"ok":false,"error":"timeout"}
{"ok":false,"error":"invalid_direction"}
{"ok":false,"error":"missing_distance"}
```

---

## 🔍 Prueba rápida sin Python

Con cualquier cliente WebSocket (extensión de Chrome, `websocat`, etc.):

```bash
websocat ws://192.168.100.23:81
{"cmd":"ping"}
{"cmd":"servo","angle":45}
{"cmd":"distance"}
{"cmd":"move_cm","direction":"forward","distance":10}
```

---

## 🐛 Problemas comunes

| Síntoma | Causa | Solución |
|---|---|---|
| No conecta a WiFi | Contraseña mal escrita en el portal | Mantén presionado un botón de reset o borra flash |
| Motores no giran | Falta alimentación externa del puente H | Conecta la batería a `VS` |
| Motores giran pero robot no avanza cm | `PULSES_PER_CM` mal calibrado | Ver sección de calibración |
| El robot responde `timeout` en `move_cm` | Encoders no conectados o mal cableados | Revisa GPIO 32/33 |
| Encoders siempre en 0 | Pines de encoder son input-only (34-39) | Usa 32/33 u otros con pull-up |
| ESP32 se reinicia al mover motores | Fuente insuficiente / no hay GND común | Usa batería externa con GND al ESP32 |
