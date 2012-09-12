#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { };

EthernetClient client;
const char server[] = "jenkins.jp";
const char requestPage[] = "/api/json?tree=jobs%5Bcolor%5D";
const char outhKey[] = "";

unsigned long lastConnectionTime = 0;
boolean lastConnected = false;
const unsigned long postingInterval = 30*1000;

boolean carriageReturn = false;
boolean inFirstLine = true;
String firstLine = "";
boolean lineBreak = false;
boolean inHttpBody = false;
String httpBody = "";

boolean hasError = false;
boolean statusUnknown = false;

unsigned long lastBlinkTime = 0;
const int ngBlinkInterval = 100;

void setup() {
//  Serial.begin(9600);
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
          if(lineBreak){
            inHttpBody = true;
          }

          lineBreak = true;
          carriageReturn = false;

          if(inFirstLine){
            firstLineCheck();
          }
        }
        else {
          lineBreak = false;
        }
      }
    }
    else {
      finishing();
    }
  }
  else{
    lighting();
  }

  if(!client.connected() && (millis() - lastConnectionTime > postingInterval)) {
    httpRequest();
  }
}

void httpRequest() {
  hasError = false;
  statusUnknown = false;
  allLedOff();

  if (client.connect(server, 80)) {
    client.print("GET ");
    client.print(requestPage);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(server);
    client.print("Authorization: Basic ");
    client.println(outhKey);
    client.println("Connection: close");
    client.println();

    lastConnectionTime = millis();
    lastConnected = true;
  } 
  else {
    client.stop();
    statusUnknown = true;
    lastConnectionTime = millis();
  }
}

void firstLineCheck() {
  inFirstLine = false;
  if( firstLine.indexOf("HTTP/1.1 200 OK") < 0){
    client.stop();
    resetFlags();
    statusUnknown = true;
  }
}

void finishing(){
  client.stop();
  if(httpBody.indexOf("red") >= 0){
    hasError = true;
  }
//  Serial.println(httpBody);
  resetFlags(); 
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

void lighting() {
  if(hasError){
    if (millis() -  lastBlinkTime > ngBlinkInterval) {
      flowLed();
      lastBlinkTime = millis();
    }
  }
  else if(statusUnknown){
    if(!digitalRead(3)){
      digitalWrite(3, HIGH);
    }
  }
  else
  {
    if(!digitalRead(2)){
      digitalWrite(2, HIGH);
    }
  }
}

void initializePinMode() {
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
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
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
  digitalWrite(8, LOW);
}


