/*************************************************************
Motor Shield 1-Channel DC Motor Demo
by Randy Sarafan

For more information see:
http://www.instructables.com/id/Arduino-Motor-Shield-Tutorial/

*************************************************************/

#include <SoftwareSerial.h>   //Software Serial Port
#include <ArduinoJson.h>

#define DEBUG_ENABLED  1

#define RxD         7
#define TxD         6

#define FRONT_LED 5
#define BACK_LED 10

#define DRV           12
#define WHL           13
#define DRV_BK        9
#define WHL_BK        8
#define DRV_SPEED_PIN 3
#define WHL_SPEED_PIN 11
#define DRV_SPEED     255
#define WHL_SPEED     255

#define WAIT_TIME 1000
#define DRIVE_TIME 600
#define WHL_DRIVE_TIME 1400
#define WHL_WAIT_TIME 200

#define LED_ON()                  FRONT_LED_ON();  BACK_LED_ON();
#define LED_OFF()                 FRONT_LED_OFF(); BACK_LED_OFF();
#define FRONT_LED_ON()            digitalWrite(FRONT_LED, HIGH);
#define BACK_LED_ON()             digitalWrite(BACK_LED, HIGH);
#define FRONT_LED_OFF()           digitalWrite(FRONT_LED, LOW);
#define BACK_LED_OFF()            digitalWrite(BACK_LED, LOW);

#define TURN_LEFT()\
  digitalWrite(WHL, HIGH);\
  digitalWrite(WHL_BK, LOW);\
  delay(WHL_WAIT_TIME);

#define TURN_RIGHT()\
  digitalWrite(WHL, LOW);\
  digitalWrite(WHL_BK, LOW);\
  delay(WHL_WAIT_TIME); 

#define GO_FW()\
  FRONT_LED_ON()\
  digitalWrite(DRV, HIGH);\
  digitalWrite(DRV_BK, LOW);
  
#define GO_BW()\
  BACK_LED_ON();\
  digitalWrite(DRV, LOW);\
  digitalWrite(DRV_BK, LOW);

#define GO_LFW()\
  TURN_LEFT(); GO_FW(); 

#define GO_RFW()\
  TURN_RIGHT(); GO_FW();

#define GO_LBW()\
  TURN_LEFT(); GO_BW();
  
#define GO_RBW()\
  TURN_RIGHT(); GO_BW();

#define STOP_ALL_THING()\
  digitalWrite(DRV_BK, HIGH);\
  digitalWrite(WHL_BK, HIGH);\
  FRONT_LED_OFF();\
  BACK_LED_OFF();

#define BLINK()\
  FRONT_LED_ON();\
  BACK_LED_ON();\
  delay(800);\
  FRONT_LED_OFF();\
  BACK_LED_OFF();\
  delay(800);\
  FRONT_LED_ON();\
  BACK_LED_ON();\
  delay(800);\
  FRONT_LED_OFF();\
  BACK_LED_OFF();

SoftwareSerial blueToothSerial(RxD,TxD);

/***************************************************************************
 * Function Name: setupBlueToothConnection
 * Description:  initilizing bluetooth connction
 * Parameters: 
 * Return: 
***************************************************************************/

void setupBlueToothConnection()
{  
  blueToothSerial.begin(9600);  
  
  blueToothSerial.print("AT");
  delay(400); 

  blueToothSerial.print("AT+DEFAULT");             // Restore all setup value to factory setup
  delay(2000); 
  
  blueToothSerial.print("AT+NAMESeeedBTSlave");    // set the bluetooth name as "SeeedBTSlave" ,the length of bluetooth name must less than 12 characters.
  delay(400);
  
  blueToothSerial.print("AT+PIN0000");             // set the pair code to connect 
  delay(400);
  
  blueToothSerial.print("AT+AUTH1");             //
  delay(400);    

  blueToothSerial.flush();
}

void setup() {
  Serial.begin(9600);

  pinMode(RxD, INPUT);
  pinMode(TxD, OUTPUT);

  //Init led, mortor
  pinMode(DRV, OUTPUT);
  pinMode(DRV_BK, OUTPUT);
  pinMode(WHL, OUTPUT);
  pinMode(WHL_BK, OUTPUT);
  pinMode(FRONT_LED, OUTPUT);
  pinMode(BACK_LED, OUTPUT);

  setupBlueToothConnection();
  BLINK();

  //Set speed
  analogWrite(DRV_SPEED_PIN, DRV_SPEED);
  analogWrite(WHL_SPEED_PIN, WHL_SPEED);

  //Off mortor, led
  digitalWrite(FRONT_LED, LOW);
  digitalWrite(BACK_LED, LOW);
  digitalWrite(WHL_BK, HIGH);
  digitalWrite(DRV_BK, HIGH);
}



char *data_buffer;

bool readData() {
  char c;
  String slen = "";
  bool isData = false;
  int content_len = 0;
  int cnt = 0;

  while (1) {
    while (blueToothSerial.available())
    {
      c = blueToothSerial.read();

      if (c == '&' && isData == false) {
        slen += '\n';
        content_len = slen.toInt();
        data_buffer = (char*) malloc(sizeof(char) * content_len);
        isData = true;
      } else if (isData == false) {
        slen += c;
      } else if (isData == true) {
        content_len--;
        if (content_len == 0) {
          //end data;
          data_buffer[cnt] = c;
          cnt++;
          return true;
        }
        data_buffer[cnt] = c;
        cnt++;
      }
    }
  }
  return false;
}

void operation(const char* op) {

  STOP_ALL_THING();
 
  if (strcmp(op, "fw") == 0) {
    GO_FW();
  } else if (strcmp(op, "bw") == 0) {
    GO_BW();
  } else if (strcmp(op, "lfw") == 0) {
    GO_LFW();
    delay(WHL_DRIVE_TIME);
    return;
  } else if (strcmp(op, "rfw") == 0) {
    GO_RFW();
    delay(WHL_DRIVE_TIME);
    return;
  } else if (strcmp(op, "lbw") == 0) {
    GO_LBW();
    delay(WHL_DRIVE_TIME);
    return;
  } else if (strcmp(op, "rbw") == 0) {
    GO_RBW();
    delay(WHL_DRIVE_TIME);
    return;
  } else if (strcmp(op, "bk") == 0) {
    BLINK();
    return;
  }
  delay(DRIVE_TIME); 
}

void loop() {
  int i, j, h;
  const char* op; 
  bool success = readData();
  Serial.println(data_buffer);
  if (success) {
    DynamicJsonBuffer jsonBuffer;
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
    // Response to toki
    blueToothSerial.print("OK");
    free(data_buffer);
  }
}

/*
void loop() {
  
  
  STOP_ALL_THING();
  delay(WAIT_TIME);
  
  GO_FW();
  delay(DRIVE_TIME);
  STOP_ALL_THING();
  BLINK();

  GO_BW();
  delay(DRIVE_TIME);
  STOP_ALL_THING();
  BLINK();

  GO_LFW();
  delay(DRIVE_TIME);
  STOP_ALL_THING();
  BLINK();
  
  GO_LBW();
  delay(DRIVE_TIME);
  STOP_ALL_THING();
  BLINK();
  
  GO_RFW();
  delay(DRIVE_TIME);
  STOP_ALL_THING();
  BLINK();
  
  GO_RBW();
  delay(DRIVE_TIME);
  STOP_ALL_THING();
  BLINK();

  delay(WAIT_TIME);
  
}
*/
