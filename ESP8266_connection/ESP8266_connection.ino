#include <ESP8266WiFi.h>



#define DONT_TOUCH_PIN 11

#define D3 5
#define D5 14
#define D6 12
#define D7 13
#define D8 0 // switch on everything




// Enter your wifi network name and Wifi Password
const char* ssid = "ofryhome";
const char* password = "Ofry218790";



// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;


// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;


struct Button{
  String name;
  String route;
  String state;
  int output_pin;
  String HTML;
} Light1;

struct Pump{
  Button button;

  int pump_freq;
  int pump_duration;
} Pump1 , Pump2;




void setup() {

  Light1.name = "Light1";
  Light1.route = "light1";
  Light1.state = "off";
  Light1.output_pin = D3;

  Pump1.button.name = "Pump1";
  Pump1.button.route = "pump1";
  Pump1.button.state = "off";
  Pump1.button.output_pin = D5;
  Pump1.pump_freq = 1;
  Pump1.pump_duration = 5;

  Pump2.button.name = "Pump2";
  Pump2.button.route = "pump2";
  Pump2.button.state = "off";
  Pump2.button.output_pin = D6;
  Pump2.pump_freq = 1;
  Pump2.pump_duration = 5;

  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(Light1.output_pin , OUTPUT);
  pinMode(Pump1.button.output_pin , OUTPUT);
  pinMode(Pump2.button.output_pin , OUTPUT);

  // Set outputs to LOW
  digitalWrite(Light1.output_pin, LOW);
  digitalWrite(Pump1.button.output_pin, LOW);
  digitalWrite(Pump2.button.output_pin, LOW);



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
  WiFiClient client = server.available(); // Listen for incoming clients

  if (client) { // If a new client connects,
    Serial.println("New Client."); // print a message out in the serial port
    String currentLine = ""; // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();
      if (client.available()) { // if there's bytes to read from the client,
        char c = client.read(); // read a byte, then
        Serial.write(c); // print it out the serial monitor
        header += c;
        if (c == '\n') { // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();



            // Display the HTML web page
            client.println("<!DOCTYPE html><html lang='en' dir='ltr'><head><meta charset='utf-8'><title>OfryHome</title></head>");


            // CSS to style the on/off buttons

            client.println(CSS());
            client.println(HTML_BODY());

            client.println("</html>");







            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') { // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
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

String ToggleButtonElement(Button* btn) {


  String btn_next_state = "";
  if (header.indexOf("GET /" + (*btn).route + "/on" ) >= 0) {
    (*btn).state = "on";
    digitalWrite((*btn).output_pin, HIGH);
  }
  else if (header.indexOf("GET /" + (*btn).route + "/off" ) >= 0) {
    (*btn).state = "off";
    digitalWrite((*btn).output_pin, LOW);
  }

  if ((*btn).state == "on") {
    btn_next_state = "off";
  }
  else {
    btn_next_state = "on";
  }

  return "<p><a href='/" + (*btn).route + "/" + btn_next_state + "'><button class='btn btn-" + (*btn).state + "'>" + (*btn).name + "</button></a></p>";
}
String PumpElement(Pump* pump) {


  if (header.indexOf("GET /" + (*pump).button.route + "/dec/pumptime" ) >= 0) {
    if ((*pump).pump_duration != 0) {
      (*pump).pump_duration--;
    }

  }
  else if (header.indexOf("GET /" + (*pump).button.route + "/inc/pumptime" ) >= 0) {
    (*pump).pump_duration++;
  }
  else if (header.indexOf("GET /" + (*pump).button.route + "/dec/freq" ) >= 0) {
    if ((*pump).pump_freq != 0) {
      (*pump).pump_freq --;
    }

  }
  else if (header.indexOf("GET /" + (*pump).button.route + "/inc/freq" ) >= 0) {
    (*pump).pump_freq ++;
  }

  String HTML = "";
  HTML = HTML +
         ToggleButtonElement(&((*pump).button)) +

         "<div class='pump-info'>" +

         ControllerElement("Once Every" , "/" + (*pump).button.route + "/inc/freq" , "/" + (*pump).button.route + "/dec/freq" , (*pump).pump_freq , "days") +
         ControllerElement("Pump time" , "/" + (*pump).button.route + "/inc/pumptime" , "/" + (*pump).button.route + "/dec/pumptime" , (*pump).pump_duration , "seconds") +

         "</div>";

  return HTML;
}
String Header(String h_level , String text) {
  return "<h" + h_level + ">" + text + "</h" + h_level + ">";
}
String ControllerElement(String title , String inc_route , String dec_route , int value , String text) {
  return "<div class='controler'><p>" + title + " :</p> <a href='" + inc_route + "'><button type='btn' name='button'>+</button></a> <p>" + String(value) + " " + text + "</p> <a href='" + dec_route + "'><button type='btn' name='button'>-</button></a></div>";
}
String CSS() {

  String style = "";
  style = style +
          "<style>" +
          ".btn{border-radius: 5px;font-size: 30px;width: 20rem;}" +
          ".btn-on{background-color: yellow;}" +
          ".btn-off{}" +
          ".buttns-container{border-width: medium;}" +
          ".controler{display: inline-block;margin: 7px;}" +
          "</style>";


  return style;



}
String HTML_BODY() {

  String Body = "";

  Body = Body +
         "<body>" +
         "<div class='buttns-container'>" +

         Header("2" , "Living Room") +
         ToggleButtonElement(&Light1) +



        Header("2" , "Plants") +
        PumpElement(&Pump1) +
        PumpElement(&Pump2) +


         "</div>" +




         "</body>";

  return Body;



}
