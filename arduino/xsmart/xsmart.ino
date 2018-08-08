#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebSocketClient.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <FS.h>
#include <SPIFFS.h>

#define WIFI_AP_MODE  1  //acts as access point
#define WIFI_CONNECT_MODE 2 //acts a normal wifi module

#define ESP_getChipId()   ((uint32_t)ESP.getEfuseMac())

int current_wifi_status = WIFI_AP_MODE;
int previous_wifi_status = current_wifi_status; //this used to detect change


char path[] = "/";
char host[] = "5.9.144.226";

WebSocketClient webSocketClient; // Use WiFiClient class to create TCP connections
WiFiClient client;   //this client is used to make tcp connection
WiFiMulti wifiMulti; // connecting to multiple wifi networks
String webID = "LOLIN32-LITE-code-v.0.0.1"; //this should be some random no, we assigned to each device. ;


//this pins for lolin32 large device
//const int PINS[] = {15, 2, 18, 4, 16, 17, 5}; // these are pins from nodemcu we are using

//this pint for lolin32 mini
const int PINS[] = {13, 15, 2, 4, 18, 23, 5}; // these are pins from nodemcu we are using

const byte interruptPin = 19;


int PINS_STATUS[] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW}; //default status of all pins
const int PIN_SIZE = 7;

int delay_connect_wifi = 5000;  //this is delay after wifi connection, this is a variable because if wifi doesn't connect we try connection again after delay++ so its dynamic
const int max_delay_connect_wifi = delay_connect_wifi * 3;  //this is the max time we try to connect.

int ping_packet_count = 0;  //ping packet is also variable only after 10 times do we second another package
const int ping_packet_reset = 10;
long challange = 0;

int ok_ping_not_recieved_count = 0; //this is to check if we get back OK response of our ping, if not we do socket connection again.
const int ok_ping_not_recieved_count_max = 20;

volatile byte interruptCounter = 0;
unsigned long interruptMills = 0;
unsigned long interruptMillsMax = 2000;

char *esp_ap_password = "123456789";
int store_wifi_api_connect_result = -1;
char *store_wifi_api_ssid = "";
char *store_wifi_api_password = "";
int store_wifi_api_connect = 0;

IPAddress ip(192, 168, 1, 99); // where xx is the desired IP Address
IPAddress gateway(192, 168, 1, 254); // set gateway to match your wifi network
IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your wifi network

WebServer server(80);

int AP_STARTED = 0; //this is mainly used to set when AP mode is started because in loop, we cannot start ap again and again

void stopAP() {
  WiFi.softAPdisconnect();
  delay(100);
  server.close();
  AP_STARTED = 0;
}
void startWifiAP() {
  if (AP_STARTED == 0) {
    WiFi.mode(WIFI_AP_STA);
    AP_STARTED = 1;


    //    WiFi.config(ip, gateway, subnet);
    String ap_ssid = "xSmart-" + String(ESP_getChipId());
    Serial.println(ap_ssid);
    Serial.println("Configuring  access point for wifi network ...");
    char ap_ssid_array[ap_ssid.length() + 1];
    ap_ssid.toCharArray(ap_ssid_array, ap_ssid.length() + 1);
    WiFi.softAP(ap_ssid_array, esp_ap_password);
    //    WiFi.begin(); // this is to test, because in between when we do this wifi gets disconnected once
    WiFi.waitForConnectResult();
    IPAddress accessIP = WiFi.softAPIP();
    Serial.print("ESP AccessPoint IP address: ");
    Serial.println(accessIP);
    if (MDNS.begin("xsmart")) {
      Serial.println("MDNS responder started");
    }


    server.on("/", HTTP_GET, []() {
      Serial.println("ping");
      server.send(200, "application/json", "{\"ping\":\"" + webID + "\"}");
    });

    server.on("/wifi", HTTP_GET, []() {


      Serial.println("scan start");

      // WiFi.scanNetworks will return the number of networks found
      int n = WiFi.scanNetworks();
      Serial.println("scan done");
      String json = "[";
      if (n == 0) {
        Serial.println("no networks found");
      } else {
        Serial.print(n);
        Serial.println(" networks found");



        for (int i = 0; i < n; ++i) {
          String auth = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "OPEN" : "AUTH";
          json +=
            "{\"SSID\":\"" + WiFi.SSID(i) + "\",\"RSSI\":" + WiFi.RSSI(i) + "\,\"ENC\":\"" + auth + "\"}";
          // Print SSID and RSSI for each network found
          Serial.print(i + 1);
          Serial.print(": ");
          Serial.print(WiFi.SSID(i));
          Serial.print(" (");
          Serial.print(WiFi.RSSI(i));
          Serial.print(")");
          Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
          if (i != (n - 1)) {
            json += ",";
          }
          delay(10);
        }
      }
      json += "]";
      server.send(200, "application/json", json);
    });

    server.on("/wifisave", HTTP_GET, []() {
      store_wifi_api_ssid = "";
      store_wifi_api_connect_result = -1;
      if (server.args() == 0)
        return server.send(500, "text/plain", "BAD ARGS");

      String ssid = server.arg("SSID");
      String password = server.arg("password");
      String path = server.arg(0);
      Serial.println(path);
      Serial.println("wifi save called");



      char ssid_array[ssid.length() + 1];
      char passsword_array[password.length() + 1];
      ssid.toCharArray(ssid_array, ssid.length() + 1);
      password.toCharArray(passsword_array, password.length() + 1);
      store_wifi_api_ssid = ssid_array;
      store_wifi_api_password = passsword_array;
      store_wifi_api_connect = 1;

      //check if wifi password valid
      server.send(200, "application/json", "{\"ssid\":\"" + ssid + "\",\"password\":\"" + password + "\"}");


    });
    server.on("/wifiresult", HTTP_GET, []() {
      if (store_wifi_api_connect_result == WL_CONNECTED) {
        server.send(200, "application/json", "{\"ssid\":\"" + String(store_wifi_api_ssid) + "\",\"status\":\"connected\"}");
      } else {
        server.send(200, "application/json", "{\"ssid\":\"" + String(store_wifi_api_ssid) + "\",\"password\":\"not_connected\"}");
      }
      store_wifi_api_ssid = "";
      store_wifi_api_connect_result = -1;
    });
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("HTTP server started");
    MDNS.addService("http", "tcp", 80);
  } else {
    Serial.print(WiFi.localIP());
    Serial.println("webserver..");
    while (AP_STARTED == 1) {
      //      Serial.print(".");
      server.handleClient();
      yield();

      if (store_wifi_api_connect == 1) {
        store_wifi_api_connect = 0;
//        stopAP();
        WiFi.mode(WIFI_STA);
        delay(100);
        Serial.print(store_wifi_api_ssid);
        Serial.print(store_wifi_api_password);
        WiFi.begin(store_wifi_api_ssid, store_wifi_api_password);
        WiFi.waitForConnectResult();
        store_wifi_api_connect_result = WiFi.status();
        if (store_wifi_api_connect_result == WL_CONNECTED) {
          Serial.println("connected");
        } else {
          Serial.println("non connected");
        }
      }
    }
    Serial.println("ap while loop stopped");
  }

}

void stopWifi() {
  WiFi.disconnect();
  //  WiFi.config(IPAddress(0, 0, 0, 0), IPAddress(0,0,0,0), IPAddress(0,0,0,0));
}
void connectWifi() {
  WiFi.mode(WIFI_STA);
  delay(100);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to wifi");

  //  WiFi.begin(ssid, password);

  wifiMulti.addAP("MANISH", "9717071555");
  wifiMulti.addAP("Binatone 1006", "0123456789");
  wifiMulti.addAP("Etech4", "building@123");
  wifiMulti.addAP("Redmi amanish", "java@123");

  int tries = 0;
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    //to handle interrupt here, because it gets stuck in loop for a long time
    if (current_wifi_status != WIFI_CONNECT_MODE) {
      Serial.print("stopped connecting to wifi due to interrupt");
      break;
    }
    tries++;
    if (tries > 100) {
      Serial.print("some issue with wifi");
      break;
    }
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    delay_connect_wifi = 5000;
  }
  delay(delay_connect_wifi);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void forcePingPacket() {
  ping_packet_count = 0;
  pingPacket();
}
void pingPacket() {
  if (ping_packet_count == 0) {
    String data = "";
    String pin_data = "";
    String pin_status = "";
    for (int i = 0; i < PIN_SIZE; i++) {
      if (i == PIN_SIZE - 1) {
        pin_data += String(PINS[i]);
      } else {
        pin_data += String(PINS[i]) + ",";
      }
    }
    for (int i = 0; i < PIN_SIZE; i++) {
      if (i == PIN_SIZE - 1) {
        pin_status += String(PINS_STATUS[i]);
      } else {
        pin_status += String(PINS_STATUS[i]) + ",";
      }
    }
    randomSeed(analogRead(0));
    challange = random(1, 1000);
    String input =
      "{\"WEBID\":\"" + webID + "\",\"PINS\":[" + pin_data + "]\,\"PINS_STATUS\":[" + pin_status + "]\,\"challange\":\"" + challange + "\"}";
    Serial.println(input);
    webSocketClient.sendData(input);
    delay(10);
    ping_packet_count++;
  } else {
    ping_packet_count++;
    if (ping_packet_count > ping_packet_reset) {
      ping_packet_count = 0;
    }
  }
}
void connectSocket() {

  // Connect to the websocket server
  if (client.connect(host, 9030)) {
    Serial.println("Connected");
    // Handshake with the server
    webSocketClient.path = path;
    webSocketClient.host = host;
    if (webSocketClient.handshake(client)) {
      Serial.println("Handshake successful");
      pingPacket();
    } else {
      Serial.println("Handshake failed.");

    }
  } else {
    Serial.println("Connection failed.");

  }


}

void pinWrite(int pin_no, int pin_mode) {
  digitalWrite(pin_no, pin_mode);
  for (int i = 0; i < PIN_SIZE; i++) {
    if (PINS[i] == pin_no) {
      PINS_STATUS[i] = pin_mode;
    }
  }
}

void initIOPins() {
  for (int i = 0; i < PIN_SIZE; i++) {
    pinMode(PINS[i], OUTPUT);
  }
}

void highIOPins() {
  for (int i = 0; i < PIN_SIZE; i++) {
    pinWrite(PINS[i], HIGH);
  }
}
void lowIOPins() {
  for (int i = 0; i < PIN_SIZE; i++) {
    pinWrite(PINS[i], LOW);
  }
}
void playIOPins() {
  //left to right all leds play
  for (int i = 0; i < PIN_SIZE; i++) {
    lowIOPins();
    pinWrite(PINS[i], HIGH);
    delay(200);
  }
  //
  //  //right to left all leds play
  //
  for (int i = PIN_SIZE - 1; i >= 0; i--) {
    lowIOPins();
    pinWrite(PINS[i], HIGH);
    delay(200);
  }


  lowIOPins();
  delay(300);
  highIOPins();
  delay(500);
  lowIOPins();
  delay(300);
  highIOPins();
  delay(500);
  lowIOPins();

}


void handleInterrupt() {
  if (interruptCounter == 0) {
    interruptCounter = 1;
    interruptMills = millis();
    Serial.println("interrupt");
    Serial.println(interruptMills);
    Serial.println("***");
  } else {
    interruptCounter = 0;
    Serial.println("interrupt end");
    Serial.println(millis() - interruptMills);
    if (millis() - interruptMills > interruptMillsMax) {
      if (current_wifi_status == WIFI_CONNECT_MODE) {
        Serial.println("set ap mode");
        current_wifi_status = WIFI_AP_MODE;
      } else {
        current_wifi_status = WIFI_CONNECT_MODE;
        Serial.println("set wifi mode");
      }
    }

  }
}

void detectInterruptChange() {
  if (previous_wifi_status != current_wifi_status) {
    if (current_wifi_status == WIFI_AP_MODE) {
      Serial.println("detect ap mode");
      previous_wifi_status = WIFI_AP_MODE;
      stopWifi();
    } else {
      current_wifi_status = WIFI_CONNECT_MODE;
      previous_wifi_status = WIFI_CONNECT_MODE;
      Serial.println("detect connect mode");
      stopAP();
    }
  }
}

String readFile(fs::FS &fs, const char * path) {
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return "";
  }

  Serial.println("- read from file:");
  String data = "";
  while (file.available()) {
    Serial.write(file.read());
    data = data + file.read();
  }
  return data;
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- frite failed");
  }
}

void setup() {

  Serial.begin(115200);
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  delay(10);

  // We start by connecting to a WiFi network

  //need to look at interrupts. if we press push button for 5sec it will start wifi mode.
  //https://techtutorialsx.com/2016/12/11/esp8266-external-interrupts/
  //http://www.electronicwings.com/nodemcaAwgw s  AT                                     S4RRRRRRRRRRRRRRRRRR4AGu/nodemcu-gpio-interrupts-with-arduino-ide

  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, CHANGE);

  initIOPins();
  playIOPins();

}

void loop() {

  detectInterruptChange();

  if (current_wifi_status == WIFI_CONNECT_MODE) {
    String data;
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("wifi disconnected, connecting again.");

      if (delay_connect_wifi < max_delay_connect_wifi) {
        delay_connect_wifi += delay_connect_wifi;
      }
      Serial.print("some issue with wifi trying again in ");
      Serial.println(delay_connect_wifi);
      delay(delay_connect_wifi);
      connectWifi();

    } else {



      if (client.connected()) {

        //      Serial.println("websocket connected");
        //      Serial.println("my id" + webID);

        webSocketClient.getData(data);



        if (data.length() > 0) {
          DynamicJsonDocument doc;
          Serial.print("Received data: ");
          Serial.println(data);
          deserializeJson(doc, data);
          JsonObject obj = doc.as<JsonObject>();
          String type = obj[String("type")];
          int pin = obj[String("pin")];

          if (type == "HIGH") {
            Serial.println("setting hight");
            pinWrite(pin, HIGH);
            delay(10);
            forcePingPacket();
          } else if (type == "LOW") {
            Serial.println("setting low");
            pinWrite(pin, LOW);
            delay(10);
            forcePingPacket();
          } else if (type == "OK") {
            ok_ping_not_recieved_count = 0;
          }
          data = "";
        } else {
          pingPacket();
          ok_ping_not_recieved_count++;

          if (ok_ping_not_recieved_count > ok_ping_not_recieved_count_max) {
            Serial.println("websocket not responding.");
            ok_ping_not_recieved_count = 0;
            connectSocket();
          }
        }

      } else {
        Serial.println("websocket disconnected.");
        connectSocket();
      }
    }

    // wait to fully let the client disconnect
  } else {

    startWifiAP();
  }
  delay(1000);
}
