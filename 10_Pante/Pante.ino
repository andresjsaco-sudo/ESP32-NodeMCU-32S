// ===== DEFINICIÓN DE PINES =====
// LED RGB (Ánodo Común - Lógica Invertida)
const int PIN_LED_R = 25;
const int PIN_LED_G = 26;
const int PIN_LED_B = 27;

// Potenciómetros (ADC)
const int PIN_POT_R = 34;
const int PIN_POT_G = 35;
const int PIN_POT_B = 32;

// Pulsador
const int PIN_BUTTON = 23;

// ===== VARIABLES GLOBALES =====
bool modoManual = true;  // true = Modo 1 (Manual), false = Modo 2 (Automático)
bool buttonState = HIGH;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// ===== CONFIGURACIÓN INICIAL =====
void setup() {
  Serial.begin(115200);
  Serial.println("=================================");
  Serial.println("Sistema de Control LED RGB");
  Serial.println("LED: Anodo Comun (3.3V)");
  Serial.println("=================================");
  
  // Configurar pines LED como salida
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  
  // Configurar pulsador con pull-up interno
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  
  // Apagar LED al inicio (todos HIGH en ánodo común)
  analogWrite(PIN_LED_R, 255);
  analogWrite(PIN_LED_G, 255);
  analogWrite(PIN_LED_B, 255);
  
  Serial.println("\n>>> MODO 1: MANUAL activado");
}

// ===== BUCLE PRINCIPAL =====
void loop() {
  // Leer estado del pulsador con debounce
  leerPulsador();
  
  if (modoManual) {
    // MODO 1: MANUAL - Control por potenciómetros
    modoManualFunc();
  } else {
    // MODO 2: AUTOMÁTICO - Luz blanca fija al 80%
    modoAutomaticoFunc();
  }
  
  delay(10);  // Pequeño retardo para estabilidad
}

// ===== FUNCIONES =====

// Leer pulsador con debounce
void leerPulsador() {
  int reading = digitalRead(PIN_BUTTON);
  
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      
      // Detectar flanco de bajada (pulsación)
      if (buttonState == LOW) {
        modoManual = !modoManual;  // Cambiar modo
        
        Serial.println("\n=================================");
        if (modoManual) {
          Serial.println(">>> MODO 1: MANUAL activado");
          Serial.println("Control por potenciometros");
        } else {
          Serial.println(">>> MODO 2: AUTOMATICO activado");
          Serial.println("Luz blanca fija al 80%");
        }
        Serial.println("=================================\n");
      }
    }
  }
  
  lastButtonState = reading;
}

// MODO 1: Control manual con potenciómetros
void modoManualFunc() {
  // Leer valores de potenciómetros (0-4095 en ESP32)
  int valorPotR = analogRead(PIN_POT_R);
  int valorPotG = analogRead(PIN_POT_G);
  int valorPotB = analogRead(PIN_POT_B);
  
  // Mapear de ADC (0-4095) a PWM (0-255)
  int intensidadR = map(valorPotR, 0, 4095, 0, 255);
  int intensidadG = map(valorPotG, 0, 4095, 0, 255);
  int intensidadB = map(valorPotB, 0, 4095, 0, 255);
  
  // INVERTIR para ánodo común: 255 = apagado, 0 = máxima intensidad
  int pwmR = 255 - intensidadR;
  int pwmG = 255 - intensidadG;
  int pwmB = 255 - intensidadB;
  
  // Aplicar valores PWM al LED
  analogWrite(PIN_LED_R, pwmR);
  analogWrite(PIN_LED_G, pwmG);
  analogWrite(PIN_LED_B, pwmB);
  
  // Mostrar valores en monitor serial (cada segundo)
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    Serial.print("Intensidades: R=");
    Serial.print(intensidadR);
    Serial.print(" G=");
    Serial.print(intensidadG);
    Serial.print(" B=");
    Serial.println(intensidadB);
    lastPrint = millis();
  }
}

// MODO 2: Luz blanca automática al 80%
void modoAutomaticoFunc() {
  // 80% de intensidad = 204 en escala 0-255
  // Para ánodo común: PWM = 255 - 204 = 51
  const int pwmBlanco80 = 51;  // Lógica invertida
  
  // Fijar todos los colores al 80%
  analogWrite(PIN_LED_R, pwmBlanco80);
  analogWrite(PIN_LED_G, pwmBlanco80);
  analogWrite(PIN_LED_B, pwmBlanco80);
  
  // Mostrar estado (solo una vez al entrar)
  static bool yaImpreso = false;
  if (!yaImpreso) {
    Serial.println("Luz blanca fija: 80% intensidad");
    Serial.println("R=204 G=204 B=204 (PWM=51 invertido)");
    yaImpreso = true;
  }
  
  // Resetear flag cuando se salga del modo
  if (modoManual) {
    yaImpreso = false;
  }
}