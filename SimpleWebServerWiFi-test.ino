#include <SPI.h>
#include <WiFi.h>
#include <ArduinoJson.h>

char ssid[] = "sevencore2";      //  your network SSID (name)
char pass[] = "sevencore2010";   // your network password
int keyIndex = 0;                 // your network key Index number (needed only for WEP)
const int BUFFER_SIZE = 256;

const int switchPin = 2;
const int motorPin = 9;
const int okPin = 5;
int switchState = 0;

int status = WL_IDLE_STATUS;
WiFiServer server(81);
void setup() {
  // put input, output pin for motor and switch
  pinMode(motorPin, OUTPUT);
  pinMode(switchPin, INPUT);
  pinMode(okPin, OUTPUT);

  Serial.begin(9600);      // initialize serial communication
  //pinMode(9, OUTPUT);      // set the LED pin mode

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);       // don't continue
  }

  String fv = WiFi.firmwareVersion();
  if ( fv != "1.1.0" )
    Serial.println("Please upgrade the firmware");

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();                           // start the web server on port 80
  printWifiStatus();                        // you're connected now, so print out the status

  //Connection ok!
  digitalWrite(okPin, HIGH);
}

void run_motor() {
  switchState = digitalRead(switchPin);

  if (switchState == HIGH) {
    digitalWrite(motorPin, HIGH);
  }
  else {
    digitalWrite(motorPin, LOW);
  }
}

void example() {
  //char name[HTTP_REQ_PARAM_NAME_LENGTH], value[HTTP_REQ_PARAM_VALUE_LENGTH];
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,

        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        //httpReq.parseRequest(c);
        if (c == '\n') {                    // if the byte is a newline character

          Serial.println("New Line!!!!!!!");

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            Serial.println("length is 0!!!!!!!!!!!!!!!!!!!!");
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("Click <a href=\"/H\">here</a> turn the LED on pin 9 on<br>");
            client.print("Click <a href=\"/L\">here</a> turn the LED on pin 9 off<br>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          }
          else {      // if you got a newline, then clear currentLine:
            Serial.println("REset currentLine!!!");
            currentLine = "";
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          Serial.println("endwWirth!!!!!");
          digitalWrite(9, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(9, LOW);                // GET /L turns the LED off
        }

      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
}

bool readRequest(WiFiClient& client) {
  bool currentLineIsBlank = true;
  Serial.println("new client");           // print a message out the serial port
  while (client.connected()) {            // loop while the client's connected
    if (client.available()) {             // if there's bytes to read from the client,
      char c = client.read();             // read a byte, then
      Serial.write(c);                    // print it out the serial monitor

      if (c == '\n' && !currentLineIsBlank) {
        return true;
      } else if (c == '\n') {
        currentLineIsBlank = true;
      } else if (c != '\r') {
        currentLineIsBlank = false;
      }

    }
  }
  return false;
}

char json_buffer[BUFFER_SIZE];
//String jsonData = "";
//char *jsonData = "";

bool readRequest2(WiFiClient& client) {
  bool startJson = false;
  int cntOfBracket = 0;
  int cnt = 0;
  Serial.println("new client");           // print a message out the serial port
  while (client.connected()) {            // loop while the client's connected
    if (client.available()) {             // if there's bytes to read from the client,
      char c = client.read();             // read a byte, then
      Serial.write(c);                    // print it out the serial monitor
      
      if(c == '{') {
        startJson = true;
        cntOfBracket++;
      }
      else if(c == '}')
        cntOfBracket--;
        
      if(startJson) {
        if(cntOfBracket == 0) {
          //jsonData += c;
          json_buffer[cnt] = c;
          cnt++;
          //Serial.println(jsonData);
          Serial.println(json_buffer);
          return true; 
        }
        //jsonData += c;
        json_buffer[cnt] = c;
        cnt++;
      }
      
    }
  }
  return false;
}

JsonObject& prepareResponse(JsonBuffer& jsonBuffer) {
  JsonObject& root = jsonBuffer.createObject();
  root["success"] = "ok";
  return root;
}

void writeResponse(WiFiClient& client, JsonObject& json) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();

  json.prettyPrintTo(client);
}

void jsonmain() {
  WiFiClient client = server.available();
  int i, j, h;

  if (client) {
    bool success = readRequest2(client);
    if (success) {
      StaticJsonBuffer<BUFFER_SIZE> jsonBuffer; 

       // char json2[] = "{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}";
      //json2 = jsonData;
      JsonObject& root = jsonBuffer.parseObject(json_buffer);
      //JsonArray& array = jsonBuffer.parseArray(jsonData);
      
      if(!root.success()) {
        Serial.println("parseObject() failed");
      } else {
        Serial.println("Success parse???");
        root.printTo(Serial);
        /*
        for(i = 0; i < root["cnt"]; i++) {
          for(j = 0; j < root["data"][i]["loop"]; i++) {
            for(h = 0; h < root["data"][i]["cnt"]; h++) {
              //  Serial.println( root["data"][i]["card"][h] );
            }
          }
                    
        
            for(j = 0; j < root[i].loop; j++) {
              for(h = 0; h < root[i].card.length; h++) {
                  Serial.println(root[i].card[h]);
              }
            }
       
        }
        */
      }

      JsonObject& json = prepareResponse(jsonBuffer);
      writeResponse(client, json);
    }
    delay(1);
    client.stop();
    Serial.println("client disconnect");
  }
}


void loop() {
  jsonmain();
  run_motor();
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}


void jsontest() {
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    //Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
     
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          Serial.println("Clear!!!");
          StaticJsonBuffer<500> jsonBuffer;
          JsonObject& json = prepareResponse(jsonBuffer);
           writeResponse(client, json);
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
}


/*
      //IF request has ended -> handle response
      if (httpReq.endOfRequest()) {
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println("Connnection: close");
        client.println();
        client.println("<!DOCTYPE HTML>");
        client.println("<html>");
        client.println("<body>");
        //handle response to requesting client and access arguments
        //access object properties
        client.print("Method: ");
        client.print(httpReq.method);
        client.println("<br>");
        client.print("Uri: ");
        client.print(httpReq.uri);
        client.println("<br>");
        client.print("Version: ");
        client.print(httpReq.version);
        client.println("<br>");
        client.print("paramCount: ");
        client.print(httpReq.paramCount);
        client.println("<br>");
        //list received parameters GET and POST
        client.println("Parameters:<br>");
        for (int i = 1; i <= httpReq.paramCount; i++) {
          httpReq.getParam(i, name, value);
          client.print(name);
          client.print("-");
          client.print(value);
          client.println("<br>");

          Serial.print(name);
          Serial.print("-");
          Serial.println(value);

        }
        //list received cookies
        client.println("Cookies:<br>");
        for (int i = 1; i <= httpReq.cookieCount; i++) {
          httpReq.getCookie(i, name, value);
          client.print(name);
          client.print(" - ");
          client.print(value);
          client.println("<br>");
        }
        //Reset object and free dynamic allocated memory
        httpReq.resetRequest();
        break;
      }
    }
  }


  delay(1);
  client.stop();
  Serial.print("client disconneted");

}
*/
