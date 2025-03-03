void setup_wifi() {

  WiFiManager wifiManager;

  WiFi.hostname("terrarium-host");   // Set hostname before connecting to WiFi
  WiFi.mode(WIFI_STA);

  //reset settings - for testing
//  wifiManager.resetSettings();
//  Serial.setDebugOutput(true);
  delay(3000);
  Serial.println(F("Connecting to last known network"));
  wifiManager.setAPCallback(configModeCallback);
  unsigned long wifiTimeout = millis() + 10000;  //**** WiFi timeout
  while (WiFi.status() != WL_CONNECTED && millis() < wifiTimeout) {
    // Do something here (LED is already blinking through heartbeat ticker)
    yield();
  }

  if (!wifiManager.autoConnect("AutoconnectAP", "password")) {
    Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.restart();
    delay(5000);
  }
//  if (WiFi.status() != WL_CONNECTED ) {
//    Serial.println(F("Connection Failed!"));
//    // If we haven't just come out of Deep Sleep, start AP
//    /* You don't want to start AP if the device came out of deep sleep
//       because it may be an intermittent WiFi issue. Best in that case is to reset
//       or go back to sleep for a while and try again. */
//    if (ESP.getResetInfoPtr()->reason != REASON_DEEP_SLEEP_AWAKE) {
//      Serial.println(F("Starting AP..."));
//      wifiManager.setConfigPortalTimeout(180);
//      if (!wifiManager.startConfigPortal("WIFI-Setup", "esp82666")) {
//        Serial.println(F("Configuration timeout."));
//      }
//    }
//
//    // Execution will reach here only if device came out of deep sleep
//    // and unable to connect, or if configuration portal timed out
//    delay(3000);
//    /* Select here whether to sleep or to reset, if came out
//        of deep sleep and unable to connect to network */
//    //ESP.deepSleep(SLEEP_TIME * 1000, WAKE_RF_DEFAULT); // microseconds
//    ESP.reset();
//    delay(5000);
//  }

  // Connection success
  Serial.print(F("IP address: "));
  Serial.println(WiFi.localIP());
  Serial.println(F("Ready\r\n"));
}


void configModeCallback(WiFiManager *myWiFiManager) {
  // Configuration mode entered, change LED behaviour
  heartbeat_Tick.detach();
  heartbeat_Tick.attach(0.1, heartbeat);

  Serial.println(F("Entered config mode"));
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
}


void setup_OTA() {
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
   ArduinoOTA.setHostname("myesp8266");

//  ArduinoOTA.setPassword((const char *)"password");

  ArduinoOTA.onStart([]() {
    Serial.print("\nOTA Update [");
    heartbeat_Tick.detach();
    heartbeat_Tick.attach(0.1, heartbeat);
    OTAProgressIndicator = 10; // first indication at 10%
  });
  ArduinoOTA.onEnd([]() {
    Serial.println(F("]\nDONE\nRestarting now!"));
    delay(1000);
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    //Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    if ( (progress / (total / 100)) >= OTAProgressIndicator) {
      Serial.print("=");
      OTAProgressIndicator += 10;
    }
  });
  ArduinoOTA.onError([](ota_error_t error) {      
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
    delay(1000);
    ESP.reset();
    delay(1000);
  });
  ArduinoOTA.begin();
}

void heartbeat() {
  static bool ledState = LOW;
  digitalWrite(LEDPIN, ledState);
  ledState = !ledState;
}

void setup_animation() {
  static uint8_t thisLED = 0;
  static uint8_t thisColor = 0;
  CRGB colors[] = {CRGB::Salmon, CRGB::Violet, CRGB::YellowGreen};

  leds[thisLED] = colors[thisColor];
  FastLED.show();

  if (++thisLED >= NUM_LEDS) {
    thisLED = 0;
    if (++thisColor >= 3) {
      thisColor = 0;
    }
  }
}

void setup_server() {
  //SERVER INIT
  server.on("/color", setColorFromWeb);
  
  server.on("/brightness", setBrightnessFromWeb);

  server.on("/time", adjustTimeFromWeb);

  server.on("/reset", resetESP);


  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });
  server.begin();
  MDNS.addService("http", "tcp", 80);
}
