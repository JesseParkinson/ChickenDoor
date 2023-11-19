// @file WebServer.ino
// @brief Example implementation using the ESP8266 WebServer.
//
// See also README.md for instructions and hints.


#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <TimeLib.h>
//#include <time.h>
#include <SunRise.h>
#include <NTPClient.h>
#include "secrets.h"  // add WLAN Credentials in here.

#include <FS.h>        // File System for Web Server Files
#include <LittleFS.h>  // This file system is used.

bool systemAuto;
#include <WiFiUdp.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

time_t timeOffset = -5 * 60 * 60;
time_t closeOffset = 2 * 60 * 60;
time_t t = now();

//double transit, sunrise, sunset;
SunRise sr;
// Location
double latitude = 39.73;
double longitude = -82.23;
//time_t utcOffset = -4 * 60 * 60;  // US Eastern Standard time (EST)
time_t sunRiseTime;
time_t sunSetTime;
const __FlashStringHelper *whichPage;
bool sunHasRise;
bool sunHasSet;
time_t epochTime;

// mark parameters not used in example
#define UNUSED __attribute__((unused))

// TRACE output simplified, can be deactivated here
#define TRACE(...) Serial.printf(__VA_ARGS__)

// name of the server. You reach it using http://webserver
#define HOSTNAME "ChickenDoor"

// local time zone definition (Berlin)
#define TIMEZONE "CET-1CEST,M3.5.0,M10.5.0/3"

// need a WebServer for http access on port 80.
ESP8266WebServer server(80);

// The text of builtin files are in this header file
#include "builtinfiles.h"


// ===== Simple functions used to answer simple GET requests =====

// This function is called when the WebServer was requested without giving a filename.
// This will redirect to the file index.htm when it is existing otherwise to the built-in $upload.htm page
void handleRedirect() {
  TRACE("Redirect...");
  String url = "/index.html";

  if (!LittleFS.exists(url)) { url = "/$update.htm"; }

  server.sendHeader("Location", url, true);
  server.send(302);
}  // handleRedirect()


// This function is called when the WebServer was requested to list all existing files in the filesystem.
// a JSON array with file information is returned.
void handleListFiles() {
  Dir dir = LittleFS.openDir("/");
  String result;

  result += "[\n";
  while (dir.next()) {
    if (result.length() > 4) { result += ","; }
    result += "  {";
    result += " \"name\": \"" + dir.fileName() + "\", ";
    result += " \"size\": " + String(dir.fileSize()) + ", ";
    result += " \"time\": " + String(dir.fileTime());
    result += " }\n";
    // jc.addProperty("size", dir.fileSize());
  }  // while
  result += "]";
  server.sendHeader("Cache-Control", "no-cache");
  server.send(200, "text/javascript; charset=utf-8", result);
}  // handleListFiles()

void calcTimes() {
  timeClient.update();
  epochTime = timeClient.getEpochTime();
  setTime(epochTime);
  t = now();
  sr.calculate(latitude, longitude, t);
  sunRiseTime = sr.riseTime;
  sunSetTime = sr.setTime;
  sunHasRise = sr.hasRise;
  sunHasSet = sr.hasSet;
}


// This will monitor the serial communication and change the necessary variables
void doorStatus() {
  calcTimes();
  //  Serial.println("Sunset: " + sunSetTime);
  if (systemAuto) {
    if (t < sunRiseTime && sunRiseTime < sunSetTime) {
      Serial.println("close");
      Serial.println("Sun has not risen");
      delay(1000);
    } else if (t > sunSetTime + closeOffset) {
      Serial.println("close");
      Serial.println("Sun has set");
      delay(1000);
    } else if (t < sunRiseTime && t < sunSetTime + closeOffset) {
      Serial.println("open");
      Serial.println("Sun has risen and not set");
      delay(1000);
    }
  } else if (!systemAuto) {
  }

  String data = Serial.readStringUntil('\n');
  //Serial.println(data);
  if (data == "Open" && !systemAuto) {
    whichPage = FPSTR("manualOpen");
  } else if (data == "Closed" && systemAuto) {
    whichPage = FPSTR("autoClosed");
  } else if (data == "Open" && systemAuto) {
    whichPage = FPSTR("autoOpen");
  } else if (data == "Closed" && !systemAuto) {
    whichPage = FPSTR("manualClosed");
  }
}


// This function is called when the sysInfo service was requested.
void handleSysInfo() {
  String result;
  char str[32];
  FSInfo fs_info;
  LittleFS.info(fs_info);
  char timeString[std::size("yyyy-mm-ddThh:mm:ssZ")];
  sunSetTime = sunRiseTime + timeOffset;
  strftime(std::data(timeString), std::size(timeString),"%FT%TZ", gmtime(&sunSetTime));
  result += "{\n";
  result += "  \"flashSize\": " + String(ESP.getFlashChipSize()) + ",\n";
  result += "  \"freeHeap\": " + String(ESP.getFreeHeap()) + ",\n";
  result += "  \"fsTotalBytes\": " + String(fs_info.totalBytes) + ",\n";
  result += "  \"fsUsedBytes\": " + String(fs_info.usedBytes) + ",\n";
  result += "  \"Time\": " + String(t) + ",\n";
  result += "  \"Sunset\": " + String(timeString) + ",\n";
  result += "}";

  server.sendHeader("Cache-Control", "no-cache");
  server.send(200, "text/javascript; charset=utf-8", result);
}  // handleSysInfo()


// ===== Request Handler class used to answer more complex requests =====

// The FileServerHandler is registered to the web server to support DELETE and UPLOAD of files into the filesystem.
class FileServerHandler : public RequestHandler {
public:
  // @brief Construct a new File Server Handler object
  // @param fs The file system to be used.
  // @param path Path to the root folder in the file system that is used for serving static data down and upload.
  // @param cache_header Cache Header to be used in replies.
  FileServerHandler() {
    TRACE("FileServerHandler is registered\n");
  }


  // @brief check incoming request. Can handle POST for uploads and DELETE.
  // @param requestMethod method of the http request line.
  // @param requestUri request ressource from the http request line.
  // @return true when method can be handled.
  bool canHandle(HTTPMethod requestMethod, const String UNUSED &_uri) override {
    return ((requestMethod == HTTP_POST) || (requestMethod == HTTP_DELETE));
  }  // canHandle()


  bool canUpload(const String &uri) override {
    // only allow upload on root fs level.
    return (uri == "/");
  }  // canUpload()


  bool handle(ESP8266WebServer &server, HTTPMethod requestMethod, const String &requestUri) override {
    // ensure that filename starts with '/'
    String fName = requestUri;
    if (!fName.startsWith("/")) { fName = "/" + fName; }

    if (requestMethod == HTTP_POST) {
      // all done in upload. no other forms.

    } else if (requestMethod == HTTP_DELETE) {
      if (LittleFS.exists(fName)) { LittleFS.remove(fName); }
    }  // if

    server.send(200);  // all done.
    return (true);
  }  // handle()


  // uploading process
  void upload(ESP8266WebServer UNUSED &server, const String UNUSED &_requestUri, HTTPUpload &upload) override {
    // ensure that filename starts with '/'
    String fName = upload.filename;
    if (!fName.startsWith("/")) { fName = "/" + fName; }

    if (upload.status == UPLOAD_FILE_START) {
      // Open the file
      if (LittleFS.exists(fName)) { LittleFS.remove(fName); }  // if
      _fsUploadFile = LittleFS.open(fName, "w");

    } else if (upload.status == UPLOAD_FILE_WRITE) {
      // Write received bytes
      if (_fsUploadFile) { _fsUploadFile.write(upload.buf, upload.currentSize); }

    } else if (upload.status == UPLOAD_FILE_END) {
      // Close the file
      if (_fsUploadFile) { _fsUploadFile.close(); }
    }  // if
  }    // upload()

protected:
  File _fsUploadFile;
};


// Setup everything to make the webserver work.
void setup(void) {
  delay(3000);  // wait for serial monitor to start completely.
  // Use Serial port for some trace information from the example
  Serial.begin(9600);
  timeClient.begin();
  timeClient.setTimeOffset(-4);
  calcTimes();
  epochTime = timeClient.getEpochTime();
  setTime(epochTime);
  //setSyncProvider(epochTime);
  Serial.setDebugOutput(false);
  // Find the last and next lunar set and rise.
  systemAuto = true;
  sr.calculate(latitude, longitude, t);

  // Returned values:
  bool sunVisible = sr.isVisible;
  bool sunHasRise = sr.hasRise;
  bool sunHasSet = sr.hasSet;

  // Additional returned values requiring conversion from UTC to local time zone
  // on the Arduino.
  //time_t sunQueryTime = sr.queryTime - utcOffset;
  sunRiseTime = sr.riseTime;
  sunSetTime = sr.setTime;


  TRACE("Starting WebServer example...\n");

  TRACE("Mounting the filesystem...\n");
  if (!LittleFS.begin()) {
    TRACE("could not mount the filesystem...\n");
    delay(2000);
    ESP.restart();
  }

  // start WiFI
  WiFi.mode(WIFI_STA);
  if (strlen(ssid) == 0) {
    WiFi.begin();
  } else {
    WiFi.begin(ssid, passPhrase);
  }

  // allow to address the device by the given name e.g. http://webserver
  WiFi.setHostname(HOSTNAME);

  TRACE("Connect to WiFi...\n");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    TRACE(".");
  }
  TRACE("connected.\n");

  // Ask for the current time using NTP request builtin into ESP firmware.
  TRACE("Setup ntp...\n");
  configTime(TIMEZONE, "pool.ntp.org");

  TRACE("Register service handlers...\n");

  // serve a built-in htm page
  server.on("/$upload.htm", []() {
    server.send(200, "text/html", FPSTR(uploadContent));
  });

  server.on("/$door.htm", []() {
    // Serial.println(whichPage);
    calcTimes();

    server.send(200, "text/html", whichPage);
  });

  // register a redirect handler when only domain name is given.
  server.on("/", HTTP_GET, handleRedirect);

  // register some REST services
  server.on("/$list", HTTP_GET, handleListFiles);
  server.on("/$sysinfo", HTTP_GET, handleSysInfo);

  // UPLOAD and DELETE of files in the file system using a request handler.
  server.addHandler(new FileServerHandler());

  // enable CORS header in webserver results
  server.enableCORS(true);

  // enable ETAG header in webserver results from serveStatic handler
  server.enableETag(true);

  // serve all static files
  server.serveStatic("/", LittleFS, "/");

  // handle cases when file is not found
  server.onNotFound([]() {
    // standard not found in browser.
    server.send(404, "text/html", FPSTR(notFoundContent));
  });

  server.begin();
  TRACE("hostname=%s\n", WiFi.getHostname());
}  // setup


// run the server...
void loop(void) {
  calcTimes();
  doorStatus();
  server.handleClient();

}  // loop()

// end.
