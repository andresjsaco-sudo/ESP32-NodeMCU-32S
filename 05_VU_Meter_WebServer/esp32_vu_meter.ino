/*********
  Adaptado de: Rui Santos - randomnerdtutorials.com
*********/

#include <WiFi.h>

// ── Wi-Fi ──────────────────────────────────────────────
const char* ssid     = "iPhone de Juan";
const char* password = "Straight";

WiFiServer server(80);
String header;

// ── Pines de los LEDs (azul → rojo) ───────────────────
const int LED_AZUL     = 25;
const int LED_VERDE    = 26;
const int LED_AMARILLO = 27;
const int LED_NARANJA  = 14;
const int LED_ROJO     = 12;

const int leds[5] = { LED_AZUL, LED_VERDE, LED_AMARILLO, LED_NARANJA, LED_ROJO };

// ── Estado visual de cada LED (para la web) ───────────
String ledState[5] = {"off","off","off","off","off"};

// ── Modos ─────────────────────────────────────────────
// 0=VU Meter  1=Manual  2=SOS  3=Intercalado
int modo = 0;

// ── Modo 0: VU Meter ──────────────────────────────────
int  nivelActual   = 0;
int  nivelObjetivo = 0;
unsigned long ultimoTickVU   = 0;
unsigned long ultimoCambioVU = 0;
const long TICK_MS   = 80;
const long CAMBIO_MS = 350;

// ── Modo 1: Manual ────────────────────────────────────
int nivelManual = 0;

// ── Modo 2: SOS ───────────────────────────────────────
// Morse SOS: · · · — — — · · ·
// Cada entrada = { ms encendido, ms apagado }
struct Pulso { unsigned int onMs; unsigned int offMs; };
const Pulso SOS_SEQ[] = {
  {150, 150}, {150, 150}, {150, 450},  // S: . . .
  {450, 150}, {450, 150}, {450, 450},  // O: - - -
  {150, 150}, {150, 150}, {150, 1500}  // S: . . .  (pausa larga al final)
};
const int SOS_LEN = 9;

int           sosIdx   = 0;
int           sosFase  = 0;   // 0=esperando ON  1=ON activo  2=esperando OFF
unsigned long sosTimer = 0;

// ── Modo 3: Intercalado ───────────────────────────────
bool          intercaladoFaseA = true;
unsigned long intercaladoTimer = 0;
const long    INTERCALADO_MS   = 3000;

// ── Timeout HTTP ──────────────────────────────────────
unsigned long currentTime  = 0;
unsigned long previousTime = 0;
const long    timeoutTime  = 2000;

// ═══════════════════════════════════════════════════════
void setLed(int i, bool on) {
  digitalWrite(leds[i], on ? HIGH : LOW);
  ledState[i] = on ? "on" : "off";
}

void apagarTodos() {
  for (int i = 0; i < 5; i++) setLed(i, false);
}

void encenderTodos() {
  for (int i = 0; i < 5; i++) setLed(i, true);
}

void aplicarNivel(int n) {
  for (int i = 0; i < 5; i++) setLed(i, i < n);
}

int generarNivelObjetivo() {
  int r = random(0, 100);
  if (r < 10) return 5;
  if (r < 25) return 4;
  if (r < 50) return 3;
  if (r < 75) return 2;
  return 1;
}

void resetSOS() {
  sosIdx   = 0;
  sosFase  = 0;
  sosTimer = millis();
  apagarTodos();
}

// ═══════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 5; i++) {
    pinMode(leds[i], OUTPUT);
    digitalWrite(leds[i], LOW);
  }

  // Animación de arranque en cascada
  for (int i = 0; i < 5; i++) { digitalWrite(leds[i], HIGH); delay(120); }
  delay(300);
  for (int i = 4; i >= 0; i--) { digitalWrite(leds[i], LOW);  delay(120); }

  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi conectado. IP:");
  Serial.println(WiFi.localIP());
  server.begin();
}

// ═══════════════════════════════════════════════════════
void tickSOS(unsigned long ahora) {
  // Máquina de 3 fases por pulso:
  //   Fase 0: esperar antes de encender (inicio o después de apagar)
  //   Fase 1: LEDs encendidos durante onMs
  //   Fase 2: LEDs apagados durante offMs
  switch (sosFase) {
    case 0:  // Arrancar el pulso: encender
      encenderTodos();
      sosTimer = ahora;
      sosFase  = 1;
      break;

    case 1:  // Esperando que termine el tiempo ON
      if (ahora - sosTimer >= SOS_SEQ[sosIdx].onMs) {
        apagarTodos();
        sosTimer = ahora;
        sosFase  = 2;
      }
      break;

    case 2:  // Esperando que termine el tiempo OFF
      if (ahora - sosTimer >= SOS_SEQ[sosIdx].offMs) {
        sosIdx = (sosIdx + 1) % SOS_LEN;  // avanzar al siguiente pulso (cicla solo)
        sosFase = 0;
      }
      break;
  }
}

// ═══════════════════════════════════════════════════════
void loop() {
  unsigned long ahora = millis();

  // ── Modo 0: VU Meter ────────────────────────────────
  if (modo == 0) {
    if (ahora - ultimoCambioVU >= CAMBIO_MS) {
      ultimoCambioVU = ahora;
      nivelObjetivo  = generarNivelObjetivo();
    }
    if (ahora - ultimoTickVU >= TICK_MS) {
      ultimoTickVU = ahora;
      if      (nivelActual < nivelObjetivo) nivelActual++;
      else if (nivelActual > nivelObjetivo) nivelActual--;
      aplicarNivel(nivelActual);
    }
  }

  // ── Modo 2: SOS ─────────────────────────────────────
  if (modo == 2) {
    tickSOS(ahora);
  }

  // ── Modo 3: Intercalado ─────────────────────────────
  if (modo == 3) {
    if (ahora - intercaladoTimer >= INTERCALADO_MS) {
      intercaladoTimer = ahora;
      intercaladoFaseA = !intercaladoFaseA;
      for (int i = 0; i < 5; i++) {
        bool esPar = (i % 2 == 0);
        setLed(i, intercaladoFaseA ? esPar : !esPar);
      }
    }
  }

  // ── Servidor Web ────────────────────────────────────
  WiFiClient client = server.available();
  if (!client) return;

  currentTime  = millis();
  previousTime = currentTime;
  Serial.println("Nuevo cliente.");
  String currentLine = "";

  while (client.connected() && millis() - previousTime <= timeoutTime) {
    if (client.available()) {
      char c = client.read();
      Serial.write(c);
      header += c;

      if (c == '\n') {
        if (currentLine.length() == 0) {

          // ── Rutas GET ──────────────────────────────
          if (header.indexOf("GET /vu") >= 0) {
            modo = 0;
            nivelActual = 0; nivelObjetivo = 0;
            ultimoTickVU = millis(); ultimoCambioVU = millis();

          } else if (header.indexOf("GET /nivel/") >= 0) {
            modo = 1;
            int idx     = header.indexOf("GET /nivel/") + 11;
            nivelManual = constrain((int)(header[idx] - '0'), 0, 5);
            nivelActual = nivelManual;
            aplicarNivel(nivelManual);

          } else if (header.indexOf("GET /sos") >= 0) {
            modo = 2;
            resetSOS();

          } else if (header.indexOf("GET /intercalado") >= 0) {
            modo             = 3;
            intercaladoFaseA = true;
            intercaladoTimer = millis() - INTERCALADO_MS; // dispara tick inmediato
          }

          // ── HTTP Response ──────────────────────────
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/html");
          client.println("Connection: close");
          client.println();

          // ── HTML ────────────────────────────────────
          client.println("<!DOCTYPE html><html lang='es'>");
          client.println("<head><meta charset='UTF-8'>");
          client.println("<meta name='viewport' content='width=device-width,initial-scale=1'>");
          client.println("<link rel='icon' href='data:,'>");
          client.println("<link href='https://fonts.googleapis.com/css2?family=Share+Tech+Mono&display=swap' rel='stylesheet'>");
          client.println("<title>ESP32 LED Control</title>");
          client.println("<style>");
          client.println("*,*::before,*::after{box-sizing:border-box;margin:0;padding:0}");
          client.println("html{background:#080c14;color:#c8d6e5;font-family:'Share Tech Mono',monospace;display:flex;justify-content:center;min-height:100vh;padding:32px 16px}");
          client.println("body{width:100%;max-width:460px}");
          client.println("h1{font-size:1.2rem;letter-spacing:.2em;text-transform:uppercase;color:#00e5ff;margin-bottom:4px;text-align:center}");
          client.println(".sub{font-size:.65rem;color:#3a5268;letter-spacing:.15em;text-align:center;margin-bottom:28px}");
          client.println(".vu{display:flex;justify-content:center;gap:14px;margin-bottom:10px}");
          client.println(".bar{width:36px;height:110px;border-radius:6px;border:1px solid #1a2a3a}");
          client.println(".bar.off{opacity:.1}");
          client.println(".b0{background:#00bfff}.b1{background:#00ff88}.b2{background:#ffee00}.b3{background:#ff8800}.b4{background:#ff2244}");
          client.println(".labels{display:flex;justify-content:center;gap:14px;margin-bottom:24px}");
          client.println(".lbl{width:36px;font-size:.55rem;text-align:center;color:#2a4258}");
          client.println(".section{background:#0f1825;border:1px solid #1e3a4a;border-radius:12px;padding:20px 18px;margin-bottom:14px}");
          client.println(".section h2{font-size:.72rem;letter-spacing:.2em;text-transform:uppercase;color:#4a7a9a;margin-bottom:10px}");
          client.println(".desc{font-size:.62rem;color:#3a5a6a;margin-bottom:12px;line-height:1.5}");
          client.println(".btn{display:inline-block;padding:11px 22px;text-decoration:none;font-family:'Share Tech Mono',monospace;font-size:.82rem;letter-spacing:.1em;text-transform:uppercase;border-radius:6px;margin:3px;border:none;cursor:pointer;transition:all .2s}");
          client.println(".btn-vu{background:linear-gradient(135deg,#00e5ff,#0099cc);color:#080c14;box-shadow:0 0 14px rgba(0,229,255,.2)}");
          client.println(".btn-vu:hover{box-shadow:0 0 26px rgba(0,229,255,.5)}");
          client.println(".btn-sos{background:linear-gradient(135deg,#ff2244,#aa0022);color:#fff;box-shadow:0 0 14px rgba(255,34,68,.2)}");
          client.println(".btn-sos:hover{box-shadow:0 0 26px rgba(255,34,68,.5)}");
          client.println(".btn-alt{background:linear-gradient(135deg,#8844ff,#5500cc);color:#fff;box-shadow:0 0 14px rgba(136,68,255,.2)}");
          client.println(".btn-alt:hover{box-shadow:0 0 26px rgba(136,68,255,.5)}");
          client.println(".btn-man{background:#111e2e;color:#5a8aaa;border:1px solid #1e3a4a}");
          client.println(".btn-man:hover{background:#1a2e42;color:#90caee}");
          client.println(".btn-man-active{background:#0e2840;color:#00e5ff;border:1px solid #00e5ff;box-shadow:0 0 10px rgba(0,229,255,.15)}");
          client.println(".status{font-size:.65rem;color:#3a6278;letter-spacing:.12em;text-align:center;margin-bottom:18px}");
          client.println(".badge{display:inline-block;padding:2px 10px;border-radius:20px;font-size:.56rem;letter-spacing:.1em;margin-left:6px;vertical-align:middle;font-weight:bold}");
          client.println(".bvu{background:#00e5ff;color:#080c14}.bman{background:#ffaa00;color:#080c14}.bsos{background:#ff2244;color:#fff}.balt{background:#8844ff;color:#fff}");
          client.println("</style></head><body>");

          client.println("<h1>ESP32 LED Control</h1>");
          client.println("<p class='sub'>NodeMCU-32S &mdash; 5 LEDs</p>");

          // Barras visuales
          client.println("<div class='vu'>");
          for (int i = 0; i < 5; i++) {
            String cls = "bar b" + String(i);
            if (ledState[i] == "off") cls += " off";
            client.println("<div class='" + cls + "'></div>");
          }
          client.println("</div>");
          client.println("<div class='labels'><span class='lbl'>AZL</span><span class='lbl'>VRD</span><span class='lbl'>AML</span><span class='lbl'>NRN</span><span class='lbl'>RJO</span></div>");

          // Badge modo activo
          String badge = "";
          if      (modo == 0) badge = "VU METER <span class='badge bvu'>ACTIVO</span>";
          else if (modo == 1) badge = "MANUAL <span class='badge bman'>ACTIVO</span>";
          else if (modo == 2) badge = "SOS <span class='badge bsos'>ACTIVO</span>";
          else if (modo == 3) badge = "INTERCALADO <span class='badge balt'>ACTIVO</span>";
          client.println("<p class='status'>PATRON: " + badge + "</p>");

          // ── Seccion 1: VU Meter ──────────────────────
          client.println("<div class='section'><h2>&#9835; VU Meter</h2>");
          client.println("<p class='desc'>Simula un medidor de volumen: azul=bajo, rojo=alto.</p>");
          client.println("<a href='/vu'><button class='btn btn-vu'>Activar VU Meter</button></a>");
          client.println("</div>");

          // ── Seccion 2: Manual ────────────────────────
          client.println("<div class='section'><h2>&#9654; Nivel Manual</h2>");
          client.println("<p class='desc'>Fija cuantos LEDs encender (de azul hacia rojo).</p>");
          const char* lbl[6] = {"0 Apagado","1 Azul","2 Verde","3 Amarillo","4 Naranja","5 Rojo"};
          for (int n = 0; n <= 5; n++) {
            String cls = "btn btn-man";
            if (modo == 1 && nivelActual == n) cls = "btn btn-man-active";
            client.println("<a href='/nivel/" + String(n) + "'><button class='" + cls + "'>" + lbl[n] + "</button></a>");
          }
          client.println("</div>");

          // ── Seccion 3: SOS ───────────────────────────
          client.println("<div class='section'><h2>&#128681; SOS</h2>");
          client.println("<p class='desc'>Todos los LEDs parpadean en codigo Morse SOS ( . . . - - - . . . ) en bucle.</p>");
          client.println("<a href='/sos'><button class='btn btn-sos'>Activar SOS</button></a>");
          client.println("</div>");

          // ── Seccion 4: Intercalado ───────────────────
          client.println("<div class='section'><h2>&#9664;&#9654; Intercalado</h2>");
          client.println("<p class='desc'>LEDs 1-3-5 y 2-4 se alternan cada 3 segundos.</p>");
          client.println("<a href='/intercalado'><button class='btn btn-alt'>Activar Intercalado</button></a>");
          client.println("</div>");

          client.println("</body></html>");
          client.println();
          break;

        } else {
          currentLine = "";
        }
      } else if (c != '\r') {
        currentLine += c;
      }
    }
  }

  header = "";
  client.stop();
  Serial.println("Cliente desconectado.\n");
}
