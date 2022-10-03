#include <ESP8266WiFi.h>

// Enter your wifi network name and Wifi Password
const char* ssid = "ofryhome";
const char* password = "Ofry218790";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// These variables store current output state of LED
String light_state = "off";
String pump1_state = "off";
String pump2_state = "off";



// Assign output variables to GPIO pins

const int light = 4;
const int pump1 = 5;

int pump1_time = 5;
int pump1_freq = 1;



// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
Serial.begin(115200);
// Initialize the output variables as outputs
pinMode(light, OUTPUT);
pinMode(pump1, OUTPUT);
// Set outputs to LOW
digitalWrite(light, LOW);
digitalWrite(pump1, LOW);



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


String ToggleButton( String btn_name , String route_name , int pin_number){

  String btn_state = "";
  if (header.indexOf("GET /" + route_name + "/on") >= 0) {
    btn_state = "on";
    digitalWrite(pin_number, HIGH);
  } 
  else if (header.indexOf("GET /" + route_name + "/off") >= 0) {
    btn_state = "off";
    digitalWrite(pin_number, LOW);
  }

  String btn_next_state = "";
  if (btn_state == "on"){
    btn_next_state = "off";
  }else{
    btn_next_state = "on";
  }
  
  return "<p><a href='/"+ route_name +"/"+ btn_next_state +"\'><button class='btn btn-"+ btn_state +"'>" + btn_name + "</button></a></p>";
}
String Pump(String pump_name , String route_name , int pin_number , int* freq_value , int* duration_value){


   if (header.indexOf("GET /"+route_name+"/dec/pumptime" ) >= 0) {
      if (*duration_value != 0){
        *duration_value = *duration_value - 1;  
      }
      
    }
    else if (header.indexOf("GET /"+route_name+"/inc/pumptime" ) >= 0) {
      *duration_value = *duration_value + 1;
    }
    else if (header.indexOf("GET /"+route_name+"/dec/freq" ) >= 0) {
      if (*freq_value != 0){
      *freq_value = *freq_value -1;
      }
      
    }
    else if (header.indexOf("GET /"+route_name+"/inc/freq" ) >= 0) {
      *freq_value = *freq_value + 1;
    }
  
  String HTML = "";
  HTML = HTML +
  ToggleButton(pump_name , route_name , pin_number) +
  
  "<div class='pump-info'>" + 
  
  Controller("Freq" ,"/"+route_name+"/inc/freq" , "/"+route_name+"/dec/freq" , *freq_value , "times a day") +
  Controller("Pump time" ,"/"+route_name+"/inc/pumptime" , "/"+route_name+"/dec/pumptime" , *duration_value , "seconds") +
  
  "</div>";

   return HTML;
}
String Header(String h_level , String text){
  return "<h"+h_level+">"+text+"</h"+h_level+">";
}
String Controller(String title , String inc_route , String dec_route , int value , String text){
  return "<div class='controler'><p>"+ title +" :</p> <a href='" + inc_route + "'><button type='btn' name='button'>+</button></a> <p>"+ String(value) +" " + text + "</p> <a href='" + dec_route + "'><button type='btn' name='button'>-</button></a></div>"; 
}
String CSS(){

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
String HTML_BODY(){

  String Body = "";

  Body = Body + 
  "<body>" + 
  "<div class='buttns-container'>" + 
  
  Header("2" , "Living Room") +
  ToggleButton("Light" , "light" , 4) +
  
   
  Header("2" , "Plants") + 
  
  Pump("Water Pump 1" , "pump1" , 5 , &pump1_freq , &pump1_time) +

  
  "</div>" +



  
  "</body>";
 



  return Body;


  
}
