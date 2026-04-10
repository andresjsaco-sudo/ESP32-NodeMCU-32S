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

// ── Calibración ADC ───────────────────────────────────
const int ADC_MIN = 70;
const int ADC_MAX = 3750;

// ── Resolución activa ─────────────────────────────────
const int ADC_RESOLUTION = 12;

// ── PWM para analogWrite ──────────────────────────────
const int PWM_FREQ       = 5000;
const int PWM_RESOLUTION = 8; 

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

  // Configurar PWM en LED_AZUL para analogWrite (Core 3.x)
  ledcAttach(LED_AZUL, PWM_FREQ, PWM_RESOLUTION);

  // Configurar potenciómetro como entrada analógica
  pinMode(POT_PIN, INPUT);

  // Configurar resolución del ADC
  analogReadResolution(ADC_RESOLUTION);

  // Animación de arranque en cascada
  for (int i = 0; i < 5; i++) { digitalWrite(leds[i], HIGH); delay(120); }
  delay(300);
  for (int i = 4; i >= 0; i--) { digitalWrite(leds[i], LOW);  delay(120); }

  // Encabezado de tabla en Serial Monitor
  int pasos = (1 << ADC_RESOLUTION) - 1;   // 2^res - 1
  Serial.println("==============================================");
  Serial.print  ("  Resolucion ADC: "); Serial.print(ADC_RESOLUTION);
  Serial.print  (" bits | Pasos: 0 - "); Serial.println(pasos);
  Serial.println("  Rango calibrado: ADC_MIN=" + String(ADC_MIN) +
                 "  ADC_MAX=" + String(ADC_MAX));
  Serial.println("==============================================");
  Serial.println("Val.Digital | Voltaje calc.(V) | V.Multim.(V) | PWM(0-255) | LEDs");
  Serial.println("------------|-----------------|--------------|------------|-----");
}

// ═══════════════════════════════════════════════════════
void loop() {
  // Leer potenciómetro
  int valorDigital = analogRead(POT_PIN);

  // Clampear al rango calibrado para evitar valores fuera de rango
  int valorClamped = constrain(valorDigital, ADC_MIN, ADC_MAX);

  // Voltaje calculado con la resolución actual
  int    pasos         = (1 << ADC_RESOLUTION) - 1;  // 2^res - 1
  float  voltajeCalc   = valorDigital * 3.3 / pasos;

  // Nivel de LEDs (0-5) mapeado al rango calibrado
  int nivel = map(valorClamped, ADC_MIN, ADC_MAX, 0, 5);

  // Brillo PWM para LED_AZUL: mapear rango calibrado a 0-255
  int brilloPWM = map(valorClamped, ADC_MIN, ADC_MAX, 0, 255);

  // Aplicar PWM al LED_AZUL (escritura "analógica") — Core 3.x usa pin directo
  ledcWrite(LED_AZUL, brilloPWM);

  // Aplicar nivel digital al resto de LEDs
  aplicarNivelDigital(nivel);
  // El LED_AZUL ya está encendido por PWM cuando nivel >= 1
  // (brillo > 0), así que visualmente funciona igual

  // Imprimir fila de tabla
  Serial.print(valorDigital);
  Serial.print("\t\t| ");
  Serial.print(voltajeCalc, 3);
  Serial.print("\t\t| (medir)\t| ");
  Serial.print(brilloPWM);
  Serial.print("\t\t| ");
  Serial.println(nivel);

  delay(LECTURA_MS);
}
