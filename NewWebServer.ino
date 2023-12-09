#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <Hash.h>
#include <LittleFS.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include <SunRise.h>
#include <ESPAsyncWebServer.h>
#include <WiFiUdp.h>

AsyncWebServer server(80);

// REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ssid = "HDSWireless";
const char* password = "ximhte11y";
bool systemAuto = true;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -18000, 60000);

time_t timeOffset = -5 * 60 * 60;
time_t closeOffset = 2 * 60 * 60;
time_t t = now();
int nowInt;
int openInt;
int closeInt;
//double transit, sunrise, sunset;
SunRise sr;
// Location
double latitude = 39.73;
double longitude = -82.23;
//time_t utcOffset = -4 * 60 * 60;  // US Eastern Standard time (EST)
time_t sunRiseTime;
time_t sunSetTime;
bool sunHasRise;
bool sunHasSet;
time_t epochTime;
String SunRiseString;
String SunSetString;
char* openTime;
char* closeTime;
String openTimeStr;
String closeTimeStr;
char str[32];
char timeString[std::size("yyyy-mm-ddThh:mm:ssZ")];


const char* PARAM_STRING = "inputString";
const char* oPARAM_INT = "openTime";
const char* cPARAM_INT = "closeTime";
const char* PARAM_FLOAT = "inputFloat";

// HTML web page to handle input fields (inputString, inputInt, inputFloat)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Chicken Door Webserver</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <script>
    function submitMessage() {
      alert("Saved value to the Chicken Door");
      setTimeout(function(){ document.location.reload(false); }, 500);   
    }
  </script></head><body>

  <form action="/get" target="hidden-form">

    DoorMode (current value %DoorMode%):</br> <input type="radio" id="manual" name="inputString" value="manual">
    <label for="manual">Manual</label>
    </br>
    <input type="radio" name="inputString" value="auto"><label for="auto">Auto</label></br>
    <input type="submit" value="Submit" onclick="submitMessage()">
  </form><br>
    <div>Today's Sunrise is at %SunRiseTime%</div>
  <form action="/get" target="hidden-form">
    Open Time (current value %openTime%): 
       <input name="openTime" type="time" value="%openTime%">
    <input type="submit" value="Submit" onclick="submitMessage()">
  </form><br>
    <div>Today's Sunset is at %SunSetTime%</div>
  <form action="/get" target="hidden-form">
    Close Time (current value %closeTime%): 
      <input name="closeTime" type="time" value="%closeTime%">
    <input type="submit" value="Submit" onclick="submitMessage()">
  </form>
  <iframe style="display:none" name="hidden-form"></iframe>


</body></html>)rawliteral";

void notFound(AsyncWebServerRequest* request) {
  request->send(404, "text/plain", "Not found");
}

void calcTimes() {
  timeClient.update();
  openTimeStr = readFile(LittleFS, "/openTime.txt");
  closeTimeStr = readFile(LittleFS, "/closeTime.txt");
  nowInt = secondsFromHoursMinutes(timeClient.getFormattedTime());
  openInt = secondsFromHoursMinutes(openTimeStr);
  closeInt = secondsFromHoursMinutes(closeTimeStr);
  // epochTime = timeClient.getEpochTime();
  // setTime(epochTime);
  t = now();
  sr.calculate(latitude, longitude, t);
  sunRiseTime = sr.riseTime;
  sunSetTime = sr.setTime;
  sunHasRise = sr.hasRise;
  sunHasSet = sr.hasSet;
  char str[32];
  char timeString[std::size("yyyy-mm-ddThh:mm:ssZ")];
  sunSetTime = sunSetTime + timeOffset;
  sunRiseTime = sunRiseTime + timeOffset;
  strftime(std::data(timeString), std::size(timeString), "%T", gmtime(&sunSetTime));
  SunSetString = String(timeString);
  strftime(std::data(timeString), std::size(timeString), "%T", gmtime(&sunRiseTime));
  SunRiseString = String(timeString);
  
}


// This will monitor the serial communication and change the necessary variables
void doorStatus() {
  calcTimes();
  //  Serial.println("Sunset: " + sunSetTime);
  if (systemAuto) {
    if (nowInt < openInt-1) {
      Serial.println("close");
      Serial.println("Sun has not risen");
      delay(1000);
    } else if (nowInt > closeInt-1) {
      Serial.println("close");
      Serial.println("Sun has set");
      delay(1000);
    } else if (nowInt < closeInt-1 && nowInt > openInt-1) {
      Serial.println("open");
      Serial.println("Sun has risen and not set");
      delay(1000);
    }
  } else if (!systemAuto) {
  }

  String data = Serial.readStringUntil('\n');
  //Serial.println(data);
  if (data == "Open" && !systemAuto) {
    //whichPage = FPSTR("manualOpen");
  } else if (data == "Closed" && systemAuto) {
    // whichPage = FPSTR("autoClosed");
  } else if (data == "Open" && systemAuto) {
    // whichPage = FPSTR("autoOpen");
  } else if (data == "Closed" && !systemAuto) {
    // whichPage = FPSTR("manualClosed");
  }
}

String readFile(fs::FS& fs, const char* path) {
  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    return String();
  }
  String fileContent;
  while (file.available()) {
    fileContent += String((char)file.read());
  }
  file.close();
  return fileContent;
}

void writeFile(fs::FS& fs, const char* path, const char* message) {
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}

// Replaces placeholder with stored values
String processor(const String& var) {
  //Serial.println(var);
  if (var == "DoorMode") {
    return readFile(LittleFS, "/DoorMode.txt");
  } else if (var == "openTime") {
    return readFile(LittleFS, "/openTime.txt");
  } else if (var == "closeTime") {
    return readFile(LittleFS, "/closeTime.txt");
  } else if (var == "SunSetTime") {
    return SunSetString;
  } else if (var == "SunRiseTime") {

    return SunRiseString;
  }
  return String();
}

int32_t secondsFromHoursMinutes(String from) {
  return (from.substring(0, 2).toInt() * 3600 + from.substring(3, 5).toInt() * 60);
}

void setup() {
  Serial.begin(9600);
  timeClient.begin();
  //timeClient.setTimeOffset(-4);
  calcTimes();

  Serial.setDebugOutput(true);

  sr.calculate(latitude, longitude, t);

  sunRiseTime = sr.riseTime;
  sunSetTime = sr.setTime;

  //configTime(TIMEZONE, "pool.ntp.org");

  if (!LittleFS.begin()) {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  // Send a GET request to <ESP_IP>/get?inputString=<inputMessage>
  server.on("/get", HTTP_GET, [](AsyncWebServerRequest* request) {
    String inputMessage;
    String inputoMessage;
    String inputcMessage;
    // GET inputString value on <ESP_IP>/get?inputString=<inputMessage>
    if (request->hasParam(PARAM_STRING)) {
      inputMessage = request->getParam(PARAM_STRING)->value();
      writeFile(LittleFS, "/DoorMode.txt", inputMessage.c_str());
    }
    // GET inputInt value on <ESP_IP>/get?inputInt=<inputMessage>
    else if (request->hasParam(oPARAM_INT)) {
      inputoMessage = request->getParam(oPARAM_INT)->value();
      writeFile(LittleFS, "/openTime.txt", inputoMessage.c_str());
    } else if (request->hasParam(cPARAM_INT)) {
      inputcMessage = request->getParam(cPARAM_INT)->value();
      writeFile(LittleFS, "/closeTime.txt", inputcMessage.c_str());
    } else {
      inputMessage = "No message sent";
    }
    request->send(200, "text/text", inputMessage);
  });
  server.onNotFound(notFound);
  server.begin();
}

void loop() {
  // To access your stored values on inputString, inputInt, inputFloat
  String yourInputString = readFile(LittleFS, "/inputString.txt");

  calcTimes();
  doorStatus();

  int yourOpenTime = readFile(LittleFS, "/openTime.txt").toInt();

  int yourCloseTime = readFile(LittleFS, "/closeTime.txt").toInt();

  Serial.print("nowInt = ");
  Serial.println(nowInt);
  Serial.print("openInt = ");
  Serial.println(openInt);
  Serial.print("closeInt = ");
  Serial.println(closeInt);

  delay(5000);
}