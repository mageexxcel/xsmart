#include "xconfig.h"
#include "HardwareSerial.h"
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

XConfig::XConfig(char *filename)
{
  configfile = filename;
}

void XConfig::initConfig(void)
{
  if (!SPIFFS.begin(true))
  {
    P("SPIFFS Mount Failed");
  }
  else
  {
    P("xconfig init");
  }
}
void XConfig::testConfig(void)
{
  addWifiSSID("manish", "java@123");
  getWifiSSID();
  deleteWifiSSID("manish");
  getWifiSSID();
}

String XConfig::loadConfigFile(void)
{
  P("load config file");
  P(configfile);
  File file = SPIFFS.open(configfile);
  if (!file || file.isDirectory())
  {
    P("- failed to open file for reading");
  }
  String data = "";
  while (file.available())
  {
    char ch = (char)file.read();
    data += String(ch);
  }
  return data;
}

void XConfig::saveConfigFile(const char *message)
{
  P("saving config file ");
  Serial.print(message);
  File file = SPIFFS.open(configfile, FILE_WRITE);
  if (!file)
  {
    P("- failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    P("- file written");
  }
  else
  {
    P("- frite failed");
  }
}

void XConfig::deleteWifiSSID(String ssid)
{
  P("deleteWifiSSID");
  String file = loadConfigFile();
  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(file);
  JsonArray &array1 = root["networks"].as<JsonArray>();

  StaticJsonBuffer<500> jsonBuffer2;
  JsonArray &networks = jsonBuffer2.createArray();

  for (int i = 0; i < array1.size(); i++)
  {
    JsonObject &obj = array1[i].as<JsonObject>();
    P(obj["ssid"].as<char *>());
    if (ssid != obj["ssid"].as<char *>())
    {
      StaticJsonBuffer<100> jsonBuffer1;
      JsonObject &object1 = jsonBuffer1.createObject();
      object1["ssid"] = obj["ssid"].as<char *>();
      object1["password"] = obj["password"].as<char *>();
      networks.add(object1);
    }
  }

  root.set("networks", networks);
  root.printTo(Serial);
  file = "";
  root.printTo(file);
  saveConfigFile(file.c_str());
}
JsonArray &XConfig::getWifiSSID(void)
{
  P("getWifiSSID");
  String file = loadConfigFile();
  P("reading ssid");
  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(file);
  JsonArray &array1 = root["networks"].as<JsonArray>();
  for (int i = 0; i < array1.size(); i++)
  {
    JsonObject &obj = array1[i].as<JsonObject>();
    P(obj["ssid"].as<char *>());
  }
  return array1;
}
void XConfig::addWifiSSID(String ssid, String password)
{
  P("addWifiSSID");
  String file = loadConfigFile();
  P(file);
  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(file);

  JsonVariant networks = root["networks"];

  if (!networks)
  {
    P("no networks in config file");
    JsonArray &array1 = root.createNestedArray("networks");
    StaticJsonBuffer<200> jsonBuffer1;
    JsonObject &object1 = jsonBuffer1.createObject();
    object1["ssid"] = ssid;
    object1["password"] = password;
    array1.add(object1);
  }
  else
  {
    JsonArray &array1 = root["networks"];
    StaticJsonBuffer<100> jsonBuffer1;
    JsonObject &object1 = jsonBuffer1.createObject();
    object1["ssid"] = ssid;
    object1["password"] = password;
    array1.add(object1);
  }
  root.printTo(Serial);
  file = "";
  root.printTo(file);
  saveConfigFile(file.c_str());
}

void XConfig::setNickName(String deviceName)
{
  P("setNickName");
  String file = loadConfigFile();
  P(file);
  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(file);

  root["nickname"] = deviceName;

  root.printTo(Serial);
  file = "";
  root.printTo(file);
  saveConfigFile(file.c_str());
}
String XConfig::getNickName()
{
  P("getNickName");
  String file = loadConfigFile();
  P(file);
  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(file);
  return root.get<String>("nickname");
}
JsonArray& XConfig::getPinConfig()
{
  P("getPinConfig");
  String file = loadConfigFile();
  P(file);
  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(file);

  if(root["pins"]){
    return root.get<JsonVariant>("pins").as<JsonArray&>();
  }else{
    StaticJsonBuffer<200> jsonBuffer;
    return jsonBuffer.createArray();
  }
  
}
void XConfig::setPinConfig(JsonArray& pinsConfig)
{
  P("setPinConfig");
  String file = loadConfigFile();
  P(file);
  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(file);

  JsonArray& pins = root.createNestedArray("pins");

  for(int i = 0;i<pinsConfig.size(); i++){
    pins.add(pinsConfig[i].as<JsonObject>());
  }

  root.printTo(Serial);
  file = "";
  root.printTo(file);
  saveConfigFile(file.c_str());
}

void XConfig::P(String msg)
{
    Serial.println(msg);
}