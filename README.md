# ESP32 Labs — Microcontroladores

Laboratorios de la materia **Microcontroladores** — Ingeniería Electrónica, Universidad del Norte.  
Plataforma: **ESP32 NodeMCU-32S** · IDE: Arduino · Core: **ESP32 Arduino Core 3.x**

---

## Estructura del repositorio

| Carpeta | Laboratorio | Tema principal |
|---|---|---|
| `01_LEDs_WiFi_WebServer` | Lab 01 | Servidor HTTP, control de GPIOs desde navegador |
| `02_Buzzer_Piano` | Lab 02 | PWM de audio, `ledcWriteTone()`, botones |
| `03_Potenciometro_VU_Lab1` | Lab 03 | ADC 12 bits, VU meter, PWM en LED (Core 3.x) |
| `04_Potenciometro_VU_Lab2` | Lab 04 | ADC 10 bits, `analogWrite()` simplificado |
| `05_VU_Meter_WebServer` | Lab 05 | Web server + 4 modos: VU, Manual, SOS, Intercalado |
| `06_I2C_Scanner` | Lab 06 | Bus I2C, `Wire.h`, scanner de direcciones |
| `07_I2C_LCD` | Lab 07 | LCD 16x2 por I2C, `LiquidCrystal_I2C` |
| `08_IPAddress_WiFi` | Lab 08 | Plantilla base conexión WiFi + print IP |
| `09_Servo_Potenciometro` | Lab 09 | ADC → servo, `ESP32Servo`, map() |

---

## Notas clave — ESP32 Core 3.x

```cpp
// PWM — API nueva (Core 3.x)
ledcAttach(pin, freq_hz, resolution_bits);   // antes: ledcSetup() + ledcAttachPin()
ledcWrite(pin, duty);                        // duty: 0 a (2^bits - 1)
ledcWriteTone(pin, freq_hz);                 // 0 = silencio

// ADC
analogReadResolution(12);     // 12 bits por defecto → 0-4095
analogReadResolution(10);     // 10 bits → 0-1023
// ADC2 (GPIO 0,2,4,12-15,25-27) NO usar con WiFi activo → usar ADC1 (GPIO 32-39)
// GPIO 34-39: solo entrada, sin pull-up interno

// I2C (pines por defecto NodeMCU-32S)
Wire.begin();   // SDA=21, SCL=22

// Utilidades
map(val, in_min, in_max, out_min, out_max);
constrain(val, min, max);
millis();   // ms desde boot, sin bloqueo
```

---

## Resumen por laboratorio

### Lab 01 — LEDs WiFi Web Server
Control de GPIO 15 y GPIO 23 desde una página web embebida en el firmware.  
**Flujo:** `WiFi.begin()` → `server.begin()` → `server.available()` → parse header HTTP → `digitalWrite()` → responder HTML.

```cpp
#include <WiFi.h>
WiFiServer server(80);
// En loop: client = server.available(); leer header; responder HTML
```

---

### Lab 02 — Buzzer Piano
5 botones → 5 notas. Si ningún botón está en HIGH, tono = 0.

```cpp
const int FREQS[] = { 262, 294, 330, 349, 392 };  // Do Re Mi Fa Sol
ledcAttach(PIN_SPEAKER, 262, 8);
// En loop:
ledcWriteTone(0, 0);  // silencio por defecto
for (int i = 0; i < N_NOTAS; i++)
    if (digitalRead(PINES_BTN[i]) == HIGH) { ledcWriteTone(0, FREQS[i]); break; }
```

---

### Lab 03 — Potenciometro VU (12 bits, calibrado)
ADC con rango calibrado (ADC_MIN=70, ADC_MAX=3750). LED azul con PWM, resto digital.

```cpp
analogReadResolution(12);
ledcAttach(LED_AZUL, 5000, 8);          // PWM en LED azul
int val    = analogRead(36);
int clamped = constrain(val, 70, 3750);
int nivel  = map(clamped, 70, 3750, 0, 5);
int pwm    = map(clamped, 70, 3750, 0, 255);
ledcWrite(LED_AZUL, pwm);
```

---

### Lab 04 — Potenciometro VU (10 bits, simplificado)
Misma idea pero sin calibración. `analogWrite()` directo.

```cpp
analogReadResolution(10);
int val = analogRead(36);                   // 0-1023
int pwm = map(val, 0, 1023, 0, 255);
analogWrite(LED_AZUL, pwm);
```

---

### Lab 05 — VU Meter Web Server (4 modos)
Máquina de modos controlada por rutas HTTP GET.

| Ruta | Modo |
|---|---|
| `/vu` | VU Meter aleatorio animado (random ponderado) |
| `/nivel/N` | Manual: enciende N LEDs (0-5) |
| `/sos` | Morse SOS: `···---···` en bucle |
| `/intercalado` | Par/impar alternando cada 3 s |

```cpp
// Modo SOS — estructura de máquina de estados
struct Pulso { unsigned int onMs; unsigned int offMs; };
const Pulso SOS_SEQ[] = {
    {150,150},{150,150},{150,450},   // S: . . .
    {450,150},{450,150},{450,450},   // O: - - -
    {150,150},{150,150},{150,1500}   // S: . . .
};
// tickSOS() avanza por fases: 0=arrancar ON, 1=esperar onMs, 2=esperar offMs
```

---

### Lab 06 — I2C Scanner
Escanea 0x01–0x7E. `endTransmission()` retorna 0 si dispositivo responde.

```cpp
#include <Wire.h>
Wire.begin();
Wire.beginTransmission(address);
byte error = Wire.endTransmission();
// error==0 → dispositivo encontrado
// error==4 → error desconocido
```

---

### Lab 07 — I2C LCD 16x2
Dirección típica: `0x27` (verificar con I2C Scanner).

```cpp
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
lcd.begin(); lcd.backlight();
lcd.print("Texto fila 1");
lcd.setCursor(0, 1);          // col=0, row=1
lcd.print(millis() / 1000);   // segundos transcurridos
```

---

### Lab 08 — IP Address (plantilla WiFi)
```cpp
#include <WiFi.h>
WiFi.begin(ssid, password);
while (WiFi.status() != WL_CONNECTED) { delay(500); }
Serial.println(WiFi.localIP());
```

---

### Lab 09 — Servo + Potenciometro
GPIO 34 solo entrada (ADC1). GPIO 33 para señal servo.

```cpp
#include <ESP32Servo.h>
Servo myServo;
myServo.attach(33);
int pot   = analogRead(34);               // 0-4095 (12 bits)
int angle = map(pot, 0, 4095, 0, 180);
myServo.write(angle);
delay(15);   // tiempo de respuesta del servo
```

---

## Pines de referencia — NodeMCU-32S

| GPIO | Función | Nota |
|---|---|---|
| 36, 39, 34, 35 | ADC1 solo entrada | Sin pull-up, solo lectura |
| 25, 26, 27 | ADC2 / salida digital / DAC | No usar ADC con WiFi |
| 21 | SDA I2C por defecto | |
| 22 | SCL I2C por defecto | |
| 0 | Boot / PWM audio | Cuidado en reset |
| 15, 23, 12, 14 | GPIO uso general | |
| 33 | PWM / servo | |
| 34–39 | Solo entrada | Sin salida posible |
