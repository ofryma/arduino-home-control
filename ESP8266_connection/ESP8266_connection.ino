#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

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



int str2int(String str){
  int i;
  int value = 0;
  int number = 0;
  char v [] = {"0"};
  int min_val = 48;
  int max_val = 48 + 9;
  for (i=0 ; i<sizeof(str)/sizeof(v[0]) ; i++){
    
    if (int(str[i]) >= min_val && int(str[i])<= max_val){
      value = int(str[i]) - 48;
      if( value >= 0 && value <= 9){
        number = number * 10;
        number = number + value;
      }
      
    }
  }
  return number;
}

struct TimeStamp{
 int day_of_year;
 int day_of_week;
 int week_number;
 String datetime;
 int ye;
 int mo;
 int da;
 int ho;
 int mi;
 int se;
 void get_time(TimeStamp* ts ,String json){
    int last_year;
    String date_time;

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, json);
    (*ts).day_of_year = doc["day_of_year"];
    (*ts).day_of_week = doc["day_of_week"];
    (*ts).day_of_week = doc["week_number"];
    (*ts).datetime = String(doc["datetime"]);
    
    date_time = (*ts).datetime;
    Serial.println(date_time);

    (*ts).ye = str2int(String(date_time[0]) + String(date_time[1]) + String(date_time[2]) + String(date_time[3]));
    
    (*ts).mo = str2int(String(date_time[5]) + String(date_time[6]));
    (*ts).da = str2int(String(date_time[8]) + String(date_time[9]));
    (*ts).ho = str2int(String(date_time[11]) + String(date_time[12]));
    (*ts).mi = str2int(String(date_time[14]) + String(date_time[15]));
    (*ts).se = str2int(String(date_time[17]) + String(date_time[18]));
 }

} time_clock;

struct Button{

  String name;
  String route;
  String state;
  int output_pin;
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

  return "<p><a href='/" + (*btn).route + "/" + btn_next_state + "'><button class='btn general-text btn-" + (*btn).state + "'>" + (*btn).name + "</button></a></p>";
}
  void Initialize(String name , String route , String state , int output_pin , Button* btn){
      (*btn).name = name;
      (*btn).route = route;
      (*btn).state = state;
      (*btn).output_pin = output_pin;
      pinMode(output_pin , OUTPUT);
      digitalWrite(output_pin, LOW);
  }
} Light1;
struct Pump{
  Button button;
  int pump_freq;
  int pump_duration;
  TimeStamp last_pump;
  String ControllerElement(String title , String inc_route , String dec_route , int value , String text) {
  return "<div class='controler'><p class='general-text float'>" + title + " :</p> <a class='float' href='" + inc_route + "'><button type='btn' name='button' class='btn-controler general-text'>+</button></a> <p class='general-text float'>" + String(value) + " " + text + "</p> <a class='float' href='" + dec_route + "'><button type='btn' name='button' class='btn-controler general-text'>-</button></a></div>";
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
         (*pump).button.ToggleButtonElement(&((*pump).button)) +

         "<div class='pump-info'>" +

         ControllerElement("Once Every" , "/" + (*pump).button.route + "/inc/freq" , "/" + (*pump).button.route + "/dec/freq" , (*pump).pump_freq , "days") +
         ControllerElement("Pump time" , "/" + (*pump).button.route + "/inc/pumptime" , "/" + (*pump).button.route + "/dec/pumptime" , (*pump).pump_duration , "seconds") +

         "</div>";

  return HTML;
}
  void Initialize(String name , String route , String state , int output_pin ,int pump_freq , int pump_duration , Pump* pump){
    (*pump).button.name = name;
    (*pump).button.route = route;
    (*pump).button.state = state;
    (*pump).button.output_pin = output_pin;
    (*pump).pump_freq = pump_freq;
    (*pump).pump_duration = pump_duration;
    pinMode(output_pin , OUTPUT);
    digitalWrite(output_pin, LOW);
  }
  void activate(Pump* pump){
    digitalWrite((*pump).button.output_pin, HIGH);
    Serial.println((*pump).button.name);
    delay((*pump).pump_duration*1000);
    Serial.println("finished!");
    digitalWrite((*pump).button.output_pin, LOW);
  }

} Pump1 , Pump2;

Pump* pumps [] = {&Pump1 , &Pump2};

int one_time = 1;
int counter = 0;
int last_counter = 0;

void setup() {

  Serial.begin(115200);

  Light1.Initialize("Light1", "light1" , "off" ,D3 , &Light1);
  Pump1.Initialize("Pump1" , "pump1" , "off" ,D5 ,5 , 1 , &Pump1);
  Pump2.Initialize("Pump2" , "pump2" , "off" ,D6 ,5 , 1 , &Pump2);



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
  if (one_time == 1){
    one_time = 0;
    time_clock.get_time(&time_clock , http_get_request("http://worldtimeapi.org","/api/timezone/Asia/Jerusalem" , client));
    checks(client);
  }


//    every 10 min checks:
    if (mins() < counter){
      counter = mins();
      last_counter = counter;
    }
    if(counter - last_counter >= 10){
      last_counter = counter;
      checks(client);
    }else if(mins() > counter){
      counter = mins();
    }
  



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






String Header(String h_level , String text) {
  return "<h" + h_level + " class='header'>" + text + "</h" + h_level + ">";
}

String CSS() {

  String style = "";
  style = style +
          "<style>" +
          "body{background-color: #EFEFEF;}" +
          ".header{font-size: 5rem;color: #277BC0;text-shadow: 2px 2px 5px black;}" +
          ".btn{border-radius: 25px;border-width: thick;font-size: 30px;width: 100%;height: 12rem;}" +
          ".btn-on{background-color: #3B9AE1;}" +
          ".btn-off{background-color: #F1EFDC;box-shadow: 8px 8px 35px black;}" +
          ".buttns-container{border-width: medium;}" +
          ".pump-info{display: inline-block;}" +
          ".controler{margin: 7px;}" +
          ".float{display: inline-block;}" +
          ".general-text{font-size: 2.5rem;color: black;}" +
          ".btn-controler{width: 2.5rem; height: 2.5rem;}" +
          "</style>";


  return style;



}
String HTML_BODY() {

  String Body = "";

  Body = Body +
         "<body>" +
         "<div class='buttns-container'>" +

         Header("2" , "Living Room") +
         Light1.ToggleButtonElement(&Light1) +



        Header("2" , "Plants") +
        Pump1.PumpElement(&Pump1) +
        Pump2.PumpElement(&Pump2) +


         "</div>" +




         "</body>";

  return Body;



}

String http_get_request(String endpoint , String route ,WiFiClient open_client){

    String json;

    String req = endpoint + route;
    HTTPClient http;

    http.useHTTP10(true);
    http.begin(open_client , req);
    http.GET();
    json = http.getString();


    http.end();

    return json;
}


void checks(WiFiClient open_client){
  checkPumps(open_client);
}

void checkPumps(WiFiClient open_client){

  
  int i=0;
  time_clock.get_time(&time_clock , http_get_request("http://worldtimeapi.org","/api/timezone/Asia/Jerusalem" , open_client));
  for (i=0 ; i < sizeof(pumps)/sizeof(Pump*); i++){
      Serial.print((*pumps[i]).button.name);
      Serial.print(" -> state: ");
      Serial.print((*pumps[i]).button.state);
      Serial.print(", ");
      Serial.print( (*pumps[i]).pump_freq + (*pumps[i]).last_pump.day_of_year - time_clock.day_of_year);
      Serial.println(" days to next pump");
    if ((*pumps[i]).button.state == "off") {
      continue;
    }
    if(time_clock.day_of_year - (*pumps[i]).pump_freq >= (*pumps[i]).last_pump.day_of_year){
      
      (*pumps[i]).last_pump.day_of_year = time_clock.day_of_year;
      (*pumps[i]).activate(pumps[i]);
    }
    
  }
}



int secs(){
  return millis()/1000;
}
int mins(){
  return secs()/60;
}
int hours(){
  return mins()/60;
}
