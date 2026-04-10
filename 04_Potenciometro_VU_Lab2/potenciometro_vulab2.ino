/*********
  Adaptado de: Rui Santos - randomnerdtutorials.com
*********/

// ── Pines LEDs (azul → rojo) ──────────────────────────
const int LED_AZUL     = 25;
const int LED_VERDE    = 26;
const int LED_AMARILLO = 27;
const int LED_NARANJA  = 14;
const int LED_ROJO     = 12;

const int leds[5] = { LED_AZUL, LED_VERDE, LED_AMARILLO, LED_NARANJA, LED_ROJO };

// ── Pin del potenciómetro ─────────────────────────────
// ADC1_0 = GPIO36
const int POT_PIN = 36;

// ── Delay entre lecturas ──────────────────────────────
const int LECTURA_MS = 2000;

// ═══════════════════════════════════════════════════════
// Enciende los primeros N LEDs con digitalWrite
void aplicarNivelDigital(int n) {
  for (int i = 0; i < 5; i++) {
    // LED_AZUL (i=0) no usar digitalWrite si PWM está activo,
    // se maneja por separado con analogWrite
    if (i == 0) continue;
    digitalWrite(leds[i], (i < n) ? HIGH : LOW);
  }
}

// ═══════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);

  // Configurar LEDs como salida
  for (int i = 0; i < 5; i++) {
    pinMode(leds[i], OUTPUT);
    digitalWrite(leds[i], LOW);
  }

  
  // Configurar potenciómetro como entrada analógica
  pinMode(POT_PIN, INPUT);

  // Configurar resolución del ADC
  analogReadResolution(10);
}

// ═══════════════════════════════════════════════════════
void loop() {
  // Leer potenciómetro
  int valorDigital = analogRead(POT_PIN);

  // Brillo PWM para LED_AZUL: mapear rango calibrado a 0-255
  int brilloPWM = map(valorDigital, 0, 1023, 0, 255);

  // Aplicar PWM al LED_AZUL (escritura "analógica") — Core 3.x usa pin directo
  analogWrite(LED_AZUL, brilloPWM);

  // Aplicar nivel digital al resto de LEDs
 // aplicarNivelDigital(nivel);
  // El LED_AZUL ya está encendido por PWM cuando nivel >= 1
  // (brillo > 0), así que visualmente funciona igual

  // Imprimir fila de tabla
  Serial.print("Valor digital. ");
  Serial.print(valorDigital);
 
  Serial.print(" Valor led ");
  Serial.print(brilloPWM);
  Serial.println("\t\t| ");
 // Serial.println(nivel);

  delay(LECTURA_MS);
}
