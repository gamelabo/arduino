
#include <SPI.h>
#include <Ethernet.h>

byte mac[] = {};

EthernetClient client;

const char server[] = "192.168.0.2";
const char requestPage[] = "/test.txt";
//const char errorCheckWord[] = "red";

unsigned long lastConnectionTime = 0;
boolean lastConnected = false;
const unsigned long postingInterval = 10*1000;

boolean carriageReturn = false;
boolean inFirstLine = true;
String firstLine = "";
boolean lineBreak = false;
boolean inHttpBody = false;
String httpBody = "";

boolean hasError = false;
boolean statusUnknown = false;

unsigned long lastBlinkTime = 0;
const int blinkNgInterval = 75;
const int lightOkInterval = 1000;

void setup() {
  Serial.begin(9600);
  // give the ethernet module time to boot up:
  delay(1000);
  Ethernet.begin(mac);

  initializePinMode();
}

void loop() {

  if(lastConnected){
    if(client.connected()){
      if (client.available()) {
        char c = client.read();

        if(inHttpBody){
          httpBody += c;
        }
        else if(inFirstLine){
          firstLine += c;
        }

        if (c == '\r') {
          carriageReturn = true;
        }
        else if( c == '\n' && carriageReturn) {
          if(inFirstLine){
            inFirstLine = false;
            if( firstLine.indexOf("HTTP/1.1 200 OK") < 0){
              Serial.println("not 200");
              client.stop();

              resetFlags();

              statusUnknown = true;
            }
            else {
              Serial.println("200 OK");
            }
          }

          if(lineBreak){
            inHttpBody = true;
          }

          lineBreak = true;
          carriageReturn = false;
        }
        else {
          lineBreak = false;
        }
      }
    }
    else {

      Serial.println();
      Serial.println("disconnecting.");
      client.stop();

      Serial.println();
      Serial.println(httpBody);

      if(httpBody.indexOf("red") >= 0){
        hasError = true;
      }

      resetFlags();
    }
  }
  else{
    if(hasError){
      if (millis() -  lastBlinkTime > blinkNgInterval) {
        flowLed();
        lastBlinkTime = millis();
      }
    }
    else if(statusUnknown){
      if (millis() -  lastConnectionTime < lightOkInterval) {
        if(!digitalRead(5)){
          digitalWrite(5, HIGH);
        }
      }
      else if(digitalRead(5)) {
        digitalWrite(5, LOW);
      }
    }
    else
    {
      if (millis() -  lastConnectionTime < lightOkInterval) {
        if(!digitalRead(3)){
          digitalWrite(3, HIGH);
        }
      }
      else if(digitalRead(3)) {
        digitalWrite(3, LOW);
      }
    }
  }

  if(!client.connected() && (millis() - lastConnectionTime > postingInterval)) {
    httpRequest();
  }
}

// this method makes a HTTP connection to the server:
void httpRequest() {
  hasError = false;
  statusUnknown = false;
  allLedOff();

  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("connecting...");
    // send the HTTP PUT request:
    client.print("GET ");
    client.print(requestPage);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(server);
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made:
    Serial.println("connection saccsess");
    lastConnectionTime = millis();
    lastConnected = true;

    Serial.println();
  } 
  else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
    Serial.println("disconnecting.");
    Serial.println();
    client.stop();
    statusUnknown = true;

    lastConnectionTime = millis();
  }
}

void initializePinMode() {
  pinMode(3, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
}

void resetFlags() {
  lastConnected = false;
  carriageReturn = false;
  lineBreak = false;
  inHttpBody = false;
  firstLine = "";
  httpBody = "";
  inFirstLine = true;
}


void flowLed() {
  if(digitalRead(5)) {
    digitalWrite(5, LOW);
    digitalWrite(6, HIGH);
  } 
  else if(digitalRead(6)) {
    digitalWrite(6, LOW);
    digitalWrite(7, HIGH);
  }    
  else if(digitalRead(7)) {   
    digitalWrite(7, LOW);
    digitalWrite(8, HIGH);
  }    
  else if(digitalRead(8)) {
    digitalWrite(8, LOW);
    digitalWrite(5, HIGH);
  }    
  else {
    digitalWrite(5, HIGH);
  }
}

void allLedOff() {
  digitalWrite(3, LOW);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
  digitalWrite(8, LOW);
}

























