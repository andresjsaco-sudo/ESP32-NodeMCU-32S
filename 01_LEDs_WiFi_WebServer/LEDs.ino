
/*********

  Adaptado de: Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/

// Load Wi-Fi library
#include <WiFi.h>

// Replace with your network credentials
const char* ssid = "iPhone";
const char* password = "123456789";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output15State = "off";
String output23State = "off";

// Assign output variables to GPIO pins
const int output15 = 15;
const int output23 = 23;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(output15, OUTPUT);
  pinMode(output23, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output15, LOW);
  digitalWrite(output23, LOW);

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

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /15/on") >= 0) {
              Serial.println("GPIO 15 on");
              output15State = "on";
              digitalWrite(output15, HIGH);
            } else if (header.indexOf("GET /15/off") >= 0) {
              Serial.println("GPIO 15 off");
              output15State = "off";
              digitalWrite(output15, LOW);
            } else if (header.indexOf("GET /23/on") >= 0) {
              Serial.println("GPIO 23 on");
              output23State = "on";
              digitalWrite(output23, HIGH);
            } else if (header.indexOf("GET /23/off") >= 0) {
              Serial.println("GPIO 23 off");
              output23State = "off";
              digitalWrite(output23, LOW);
            }
            
           // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<link href=\"https://fonts.googleapis.com/css2?family=Share+Tech+Mono&display=swap\" rel=\"stylesheet\">");
            client.println("<style>");
            client.println("*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }");
            client.println("html { font-family: 'Share Tech Mono', monospace; background-color: #0a0e17; color: #c8d6e5; display: flex; justify-content: center; align-items: center; min-height: 100vh; }");
            client.println("body { width: 100%; max-width: 480px; padding: 32px 20px; text-align: center; }");
            client.println("h1 { font-size: 1.4rem; letter-spacing: 0.2em; text-transform: uppercase; color: #00e5ff; margin-bottom: 8px; }");
            client.println(".subtitle { font-size: 0.75rem; color: #4a6278; letter-spacing: 0.15em; margin-bottom: 40px; }");
            client.println(".card { background: #111827; border: 1px solid #1e3a4a; border-radius: 12px; padding: 28px 24px; margin-bottom: 20px; }");
            client.println(".status { font-size: 0.7rem; letter-spacing: 0.2em; text-transform: uppercase; color: #4a6278; margin-bottom: 12px; }");
            client.println(".button { display: inline-block; background: linear-gradient(135deg, #00e5ff, #0099cc); border: none; color: #0a0e17; padding: 14px 48px; text-decoration: none; font-family: 'Share Tech Mono', monospace; font-size: 1rem; letter-spacing: 0.15em; text-transform: uppercase; border-radius: 6px; margin: 6px; cursor: pointer; transition: all 0.2s ease; box-shadow: 0 0 20px rgba(0,229,255,0.2); }");
            client.println(".button:hover { box-shadow: 0 0 32px rgba(0,229,255,0.5); transform: translateY(-1px); }");
            client.println(".button2 { background: linear-gradient(135deg, #2a3a4a, #1a2a3a); color: #4a7a9a; box-shadow: none; border: 1px solid #1e3a4a; }");
            client.println(".button2:hover { box-shadow: 0 0 16px rgba(0,100,150,0.3); color: #7aaaca; }");
            client.println(".dot { display: inline-block; width: 8px; height: 8px; border-radius: 50%; background: #00e5ff; margin-right: 8px; animation: pulse 2s infinite; }");
            client.println("@keyframes pulse { 0%,100%{ opacity:1; } 50%{ opacity:0.3; } }");
            client.println("</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Servidor Web para ESP32</h1>");
              client.println("<p> Para la clase de MicroC </p>");
            // Display current state, and ON/OFF buttons for GPIO 15  
            client.println("<p>Estado del LED en GPIO 15 " + output15State + "</p>");
            // If the output15State is off, it displays the ON button       
            if (output15State=="off") {
              client.println("<p><a href=\"/15/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/15/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for GPIO 23  
            client.println("<p>Estado del LED en GPIO 23 " + output23State + "</p>");
            // If the output23State is off, it displays the ON button       
            if (output23State=="off") {
              client.println("<p><a href=\"/23/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/23/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}