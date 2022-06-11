//---------------------------------------------------------------------
String getContentType(String filename) {
  if (server.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}
//----------------------------------------------------------------------
bool handleFileRead(String path) {

  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void setColorFromWeb() {
  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == "c") {
      uint32_t tmp = (uint32_t) strtol((const char *)&server.arg(i), NULL, 16);
      if (tmp >= 0x000000 && tmp <= 0xFFFFFF) {
        fill_solid(leds, NUM_LEDS, tmp);
        FastLED.show();
        adjustLEDs();
        server.send(200, "text/plain", "OK");
      }
    }
  }
}

void setBrightnessFromWeb() {
  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == "b") {
      uint8_t tmp = (uint8_t) strtol((const char *)&server.arg(i), NULL, 16);
      if (FastLED.getBrightness() > tmp) {
        fadeUpToBrightnessIn(tmp, 5000);
        FastLED.show();
        server.send(200, "text/plain", "FUCK YOU");
      }
      else {
        fadeUpToBrightnessIn(tmp, 5000);
        FastLED.show();
        server.send(200, "text/plain", "FUCK YOU");
      }
    }
}
}

void adjustTimeFromWeb() {
  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == "s") { // reset to NZST
      uint16_t tmp = (uint16_t) strtol((const char *)&server.arg(i), NULL, 16);
      configTime(3600 * tmp, 0, "time.nist.gov", "www.pool.ntp.org/zone/nz"); // GMT +12 Hrs
      adjustLEDs();
      server.send(200, "text/plain", (String)tmp);
      printf("Time changed to GMT+", tmp); 
    }
  }
}

void resetESP() {
  
  ESP.reset();
  server.send(200, "Rebooting...");
}
