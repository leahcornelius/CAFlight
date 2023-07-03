/* SevSeg Counter Example

 Copyright 2020 Dean Reading

 This example demonstrates a very simple use of the SevSeg library with a 4
 digit display. It displays a counter that counts up, showing deci-seconds.
 */

#include "SevSeg.h"
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiAP.h>
#include "const.h"
#define LED_BUILTIN 2

SevSeg sevseg;        // Instantiate a seven segment controller object
WebServer server(80); // Controller web server

unsigned long countdown_length = 10000;
unsigned long countdown = countdown_length;

unsigned long lastTime = 0;
unsigned long currentTime = 0;
unsigned long lastStatusLedTime = 0;
unsigned long lastBuzzerTime = 0;

bool armed = false;
bool launchStarted = false;
bool statusLedState = false;
bool buzzerStatus = false;

String SendHTML(unsigned long countdownValue, unsigned long countdownLength, bool isArmed, bool isLaunchStarted)
{
  String htmlCode = "<!DOCTYPE html>\
  <html>\
  <head>\
  <meta charset=\"utf-8\">\
  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
  <title>CTech Launch Controller</title>\
  <style>\
    body {\
      font-family: Arial, Helvetica, sans-serif;\
      text-align: center;\
    }\
    .button {\
      background-color: #4CAF50;\
      border: none;\
      color: white;\
      padding: 15px 32px;\
      text-align: center;\
      text-decoration: none;\
      display: inline-block;\
      font-size: 16px;\
      margin: 4px 2px;\
      cursor: pointer;\
    }\
    .button2 {\
      background-color: #008CBA;\
    }\
    .button3 {\
      background-color: #f44336;\
    }\
    .button4 {\
      background-color: #e7e7e7; color: black;\
    }\
    </style>\
  </head>\
  <body>\
  <h1>CTech Launch Controller</h1>\
  <h2>T-minus: ";
  htmlCode += countdownValue / 1000;
  htmlCode += "s</h2>\
  <h2>Armed: ";
  htmlCode += isArmed ? "YES" : "NO";
  htmlCode += "</h2>\
  <h2>Launch started: ";
  htmlCode += isLaunchStarted ? "YES" : "NO";
  htmlCode += "</h2>";
  if (!isLaunchStarted)
  {
    htmlCode += "<form action=\"/countdown\">\
    <label for=\"countdown\">Countdown length (s):</label>\
    <input type=\"number\" id=\"countdown\" name=\"countdown\" value=\"";
    htmlCode += countdownLength / 1000;
    htmlCode += "\" min=\"1\" max=\"60\"><br><br>\
    <input type=\"submit\" class=\"button button2\" value=\"Set countdown\">\
    </form>";
  }
  htmlCode += "</body> </html>";
  return htmlCode;
}
void handle_root()
{
  server.send(200, "text/html", SendHTML(countdown, countdown_length, armed, launchStarted));
}

void handle_set_countdown()
{
  if (server.hasArg("countdown"))
  {
    countdown_length = server.arg("countdown").toInt() * 1000;
    countdown = countdown_length;
    server.send(200, "text/html", SendHTML(countdown, countdown_length, armed, launchStarted));
  }
  else
  {
    server.send(400, "text/plain", "400: Invalid Request");
  }
}

void begin_launch()
{
  digitalWrite(launchingLedPin, HIGH);
  launchStarted = true;
  countdown = countdown_length;
  lastTime = millis();
  lastBuzzerTime = millis();
  buzzerStatus = true;
  digitalWrite(buzzerPin, buzzerStatus);
}

void handle_not_found()
{
  server.send(404, "text/plain", "404: Not found");
}

void setup()
{
  Serial.begin(115200);
  Serial.println("");
  Serial.println(" - CTech Launch Controller - ");
  Serial.print("Setting pin modes... ");
  digitalWrite(relayPin, LOW);
  pinMode(relayPin, OUTPUT);
  pinMode(armedLedPin, OUTPUT);
  pinMode(launchingLedPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(armSwitchPin, INPUT_PULLUP);
  pinMode(launchButtonPin, INPUT_PULLUP);
  Serial.println("DONE");
  Serial.println("Starting WiFi AP:");
  Serial.print("- SSID: ");
  Serial.print(ssid);
  Serial.print(", Pass: ");
  Serial.println(password);
  if (!WiFi.softAP(ssid, password))
  {
    Serial.println("- AP_init: FAILED!");
    while (1)
    {
      statusLedState = !statusLedState;
      digitalWrite(LED_BUILTIN, statusLedState);
      delay(100);
    }
  }
  else
  {
    Serial.println("- AP init: OK");
    Serial.print("- IP: ");
    Serial.println(WiFi.softAPIP());
    server.on("/", handle_root);
    server.on("/countdown", handle_set_countdown);
    server.onNotFound(handle_not_found);
    server.begin();
    Serial.println("- Server: ONLINE");
  }

  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments,
               updateWithDelays, leadingZeros, disableDecPoint);
  sevseg.setBrightness(90);
  sevseg.setNumber(countdown / 10);
  lastTime = millis();
}

void launch()
{
  buzzerStatus = false;
  launchStarted = false;
  countdown = countdown_length;
  digitalWrite(launchingLedPin, LOW);
  digitalWrite(buzzerPin, HIGH);
  digitalWrite(relayPin, HIGH);
  delay(2000);
  digitalWrite(relayPin, LOW);
  digitalWrite(buzzerPin, LOW);
}

void loop()
{
  server.handleClient();
  armed = !digitalRead(armSwitchPin);
  digitalWrite(armedLedPin, armed);
  sevseg.setNumber(countdown / 10);
  if (armed)
  {
    if (launchStarted)
    {
      currentTime = millis();
      countdown -= currentTime - lastTime;
      Serial.println(countdown);
      if (buzzerStatus)
      {
        if (currentTime - lastBuzzerTime >= buzzerDuration)
        {
          buzzerStatus = false;
          digitalWrite(buzzerPin, LOW);
        }
      }

      if (countdown <= 0)
      {
        launch();
        sevseg.setChars("-GO-");
      }
      else if (countdown > 2000 && countdown % 1000 == 0)
      {
        buzzerStatus = true;
        digitalWrite(buzzerPin, HIGH);
        lastBuzzerTime = currentTime;
      }
      else if (countdown <= 2000 && countdown % 500 == 0)
      {
        buzzerStatus = true;
        digitalWrite(buzzerPin, HIGH);
        lastBuzzerTime = currentTime;
      }
      lastTime = currentTime;
    }
    else if (!digitalRead(launchButtonPin))
    {
      begin_launch();
    }
  }
  else
  {
    if (launchStarted)
    {
      sevseg.setChars("ABRT");
      countdown = countdown_length;
      launchStarted = false;
      digitalWrite(launchingLedPin, LOW);
    }
  }
  sevseg.refreshDisplay(); // Must run repeatedly
}

/// END ///