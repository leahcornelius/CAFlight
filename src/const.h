#ifndef CONFIG_H
#define CONFIG_H
// WiFi AP config
const char *ssid = "CTech Launch Control";
const char *password = "there is no spoon";
// Pinout
const int relayPin = 14;
const int armedLedPin = 27;
const int launchingLedPin = 26;
const int buzzerPin = 12;
const int armSwitchPin = 25;
const int launchButtonPin = 33;
// 7 seg config
const byte numDigits = 4;
const byte digitPins[] = {15, 2, 4, 16};
const byte segmentPins[] = {13, 23, 22, 21, 19, 18, 5};
const bool resistorsOnSegments = false;   // 'false' means resistors are on digit pins
const byte hardwareConfig = COMMON_ANODE; // See README.md for options
const bool updateWithDelays = false;      // Default 'false' is Recommended
const bool leadingZeros = true;           // Use 'true' if you'd like to keep the leading zeros
const bool disableDecPoint = true;        // Use 'true' if your decimal point doesn't exist or isn't connected

// Misc config
const int buzzerDuration = 300;
#endif