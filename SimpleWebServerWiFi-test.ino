#include <SPI.h>
#include <WiFi.h>
#include <ArduinoJson.h>

char ssid[] = "sevencore2";      //  your network SSID (name)
char pass[] = "sevencore2010";   // your network password
int keyIndex = 0;                 // your network key Index number (needed only for WEP)

const int switchPin = 2;
const int motorPin = 9;
const int okPin = 5;
const int ledleft = 3;
const int ledright = 6;

int switchState = 0;

byte mac[6];

int status = WL_IDLE_STATUS;
WiFiServer server(81);
void setup() {
  // put input, output pin for motor and switch
  String fv;
  Serial.begin(9600);      // initialize serial communication
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);       // don't continue
  }

  fv = WiFi.firmwareVersion();
  Serial.println(fv);
  if ( fv != "1.1.0" ) {
    Serial.println("Please upgrade the firmware");
    Serial.println(fv);
    return;
  }

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
  set_pin();
}

void set_pin() {
  pinMode(motorPin, OUTPUT);
  pinMode(switchPin, INPUT);
  pinMode(okPin, OUTPUT);
  pinMode(ledleft, OUTPUT);
  pinMode(ledright, OUTPUT);

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

char *data_buffer;

bool readRequest(WiFiClient& client) {
  bool startJson = false;
  int cntOfBracket = 0;
  int cnt = 0;
  String currentLine = "";
  int content_len = 0;
  String slen = "";
  bool iscontentlength = false;
  Serial.println("new client");           // print a message out the serial port
  while (client.connected()) {            // loop while the client's connected
    if (client.available()) {             // if there's bytes to read from the client,
      char c = client.read();             // read a byte, then
      Serial.write(c);                    // print it out the serial monitor

      if (c == '\n') {
        currentLine = "";
        if (iscontentlength) {
          slen += '\n';
          iscontentlength = false;
          content_len = slen.toInt();
          //Serial.println(content_len);
          data_buffer = (char*) malloc(sizeof(char) * content_len);
        }
      } else if (c != '\r') {
        currentLine += c;
      }

      if (currentLine.startsWith("Content-Length: ", 0)) {
        slen += c;
        iscontentlength = true;
      }


      if (c == '{') {
        startJson = true;
        cntOfBracket++;
      }
      else if (c == '}')
        cntOfBracket--;

      if (startJson) {
        if (cntOfBracket == 0) {
          //jsonData += c;
          data_buffer[cnt] = c;
          cnt++;
          //Serial.println(jsonData);
          //Serial.println(data_buffer);
          return true;
        }
        //jsonData += c;
        data_buffer[cnt] = c;
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

void operation(const char* op) {

  //Serial.println(op);
  //Serial.println("All off");
  digitalWrite(motorPin, LOW);
  digitalWrite(ledleft, LOW);
  digitalWrite(ledright, LOW);

  if (strcmp(op, "forward") == 0) {
    digitalWrite(motorPin, HIGH);
    //Serial.println("forward!");
  } else if (strcmp(op, "stop") == 0) {
    //Serial.println("stop!");
    digitalWrite(motorPin, LOW);
  } else if (strcmp(op, "ledon1") == 0) {
    //Serial.println("ledon1!");
    digitalWrite(ledleft, HIGH);
  } else if (strcmp(op, "ledon2") == 0) {
    //Serial.println("ledon2!");
    digitalWrite(ledright, HIGH);
  } else if (strcmp(op, "ledoff1") == 0) {
    //Serial.println("ledoff1!");
    digitalWrite(ledleft, LOW);
  } else if (strcmp(op, "ledoff2") == 0) {
    //Serial.println("ledoff2!");
    digitalWrite(ledright, LOW);
  }
  //Serial.println("delay!!!");
  delay(1000);

}

void jsonmain() {
  WiFiClient client = server.available();
  int i, j, h;
  const char* op;
  if (client) {
    bool success = readRequest(client);
    if (success) {
      DynamicJsonBuffer jsonBuffer;
      //StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(data_buffer);


      if (!root.success()) {
        Serial.println("parseObject() failed");
      } else {
        Serial.println("Success parse");
        //root.printTo(Serial);

        for (i = 0; i < root["cnt"]; i++) {
          for (j = 0; j < root["data"][i]["loop"]; j++) {
            for (h = 0; h < root["data"][i]["cnt"]; h++) {
              op = root["data"][i]["card"][h].asString();
              operation(op);
            }
          }
        }
      }

      //StaticJsonBuffer<100> okbuffer;
      JsonObject& json = prepareResponse(jsonBuffer);
      writeResponse(client, json);

      //End of response. Free all of allocation memory
      for (i = 0; i < 3; i++) {
        digitalWrite(okPin, LOW);
        delay(500);
        digitalWrite(okPin, HIGH);
        delay(500);
      }

      free(data_buffer);
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

  WiFi.macAddress(mac);
  Serial.print("MAC: ");
  Serial.print(mac[5], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.println(mac[0], HEX);

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
