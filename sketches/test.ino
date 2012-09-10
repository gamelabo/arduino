#include <SPI.h>
#include <Ethernet.h>

byte mac[] = {};

EthernetClient client;


const char server[] = "192.168.0.2";

const char requestPage[] = "/test.txt";
const char errorCheckWord[] = "red";



unsigned long lastConnectionTime = 0;          // last time you connected to the server, in milliseconds
//boolean lastConnected = false;                 // state of the connection last time through the main loop
const unsigned long postingInterval = 10*1000;  // delay between updates, in milliseconds

boolean lastConnected = false;

boolean carriageReturn = false;
boolean lineBreak = false;
boolean inHttpBody = false;
String firstLine = "";
String httpBody = "";
boolean inFirstLine = true;

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
              client.stop();
              lastConnected = false;
              Serial.println("not 200");
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
      lastConnected = false;

      Serial.println();
      Serial.println(httpBody.length());
      Serial.println(httpBody);

      carriageReturn = false;
      lineBreak = false;
      inHttpBody = false;
      firstLine = "";
      httpBody = "";
      inFirstLine = true;

    }
  }

  if(!client.connected() && (millis() - lastConnectionTime > postingInterval)) {
    httpRequest();
  }

}

// this method makes a HTTP connection to the server:
void httpRequest() {
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
    client.stop();
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













