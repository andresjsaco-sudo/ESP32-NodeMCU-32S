#include <ESP32Servo.h>

const int potPin = 34;   // Pin ADC del potenciómetro
const int servoPin = 33; // Pin de señal del servo

Servo myServo;
int potValue;
int servoAngle;

void setup() {
  Serial.begin(115200);
  myServo.attach(servoPin);
}

void loop() {
  potValue = analogRead(potPin);                  // Lectura ADC: 0–4095 (12 bits)
  servoAngle = map(potValue, 0, 4095, 0, 180);    // Mapear a 0–180 grados
  myServo.write(servoAngle);

  Serial.print("Pot: ");
  Serial.print(potValue);
  Serial.print(" -> Angulo: ");
  Serial.println(servoAngle);

  delay(15); // Tiempo de respuesta del servo
}