// Load Wi-Fi library
#include <WiFi.h>

// Replace with your network credentials
const char* ssid = "S24 Ultra de DIANA";
const char* password = "ChangeMe";

// Set web server port number to 80
WiFiServer server(80);

void setup() {
 Serial.begin(115200);

// Connect to Wi-Fi network with SSID and password
 Serial.print("Connecting to ");
 Serial.println(ssid);
 WiFi.begin(ssid, password);
 while (WiFi.status() != WL_CONNECTED) {
 delay(500);
 Serial.print(".");
 }
// Print local IP address and start web server
 Serial.println("");
 Serial.println("WiFi connected.");
 Serial.println("IP address: ");
 Serial.println(WiFi.localIP());
 server.begin();
}

void loop() {
}
