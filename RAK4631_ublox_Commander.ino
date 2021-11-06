/*
  line #71 compiler.libraries.ldflags= -lsupc++ -lm -lc -lstdc++
*/

#undef max
#undef min
#include <string>
#include <vector>

using namespace std;
unsigned char MONGNSS[16] = {
  0xB5, 0x62, 0x0A, 0x28, 0x08, 0x00,
  0x01, 0x0F, 0x03, 0x05, 0x03, 0x00, 0x00, 0x00,
  0x57, 0x34
};

unsigned char CFGGNSS[68] = {
  0xB5, 0x62, 0x06, 0x3E, 0x3C, 0x00,
  0x00, 0x00, 0x20, 0x07,  // msgVer. numTrkChHw. numTrkChUse. numConfigBlocks
  0x00, 0x08, 0x10, 0x00, 0x01, 0x00, 0x01, 0x01, // 0: GPS
  0x01, 0x01, 0x03, 0x00, 0x01, 0x00, 0x01, 0x01, // 1: SBAS
  0x02, 0x04, 0x08, 0x00, 0x00, 0x00, 0x01, 0x01, // 2: Galileo
  0x03, 0x08, 0x10, 0x00, 0x01, 0x00, 0x01, 0x01, // 3: Beidou
  0x04, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x01, // 4: IMES
  0x05, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x01, // 5: QZSS
  0x06, 0x08, 0x0E, 0x00, 0x00, 0x00, 0x01, 0x01, // 6: GLONASS
  0x2E, 0x95 // Checksum
};

#define gpsOffset 17
#define sbasOffset 25
#define galileoOffset 33
#define beidouOffset 41
#define imesOffset 49
#define qzssOffset 57
#define glonassOffset 65

unsigned char MONVER[2] = {0x0A, 0x04};
unsigned char NAVX5[2] = {0x06, 0x23};
unsigned char CFGPRT[2] = {0x06, 0x00};
unsigned char CFGDAT[2] = {0x06, 0x06};
unsigned char CFGNMEA[2] = {0x06, 0x17};
unsigned char CFGRST[2] = {0x06, 0x04};

template class basic_string<char>; // https://github.com/esp8266/Arduino/issues/1136
// Required or the code won't compile!
namespace std _GLIBCXX_VISIBILITY(default) {
_GLIBCXX_BEGIN_NAMESPACE_VERSION
void __throw_length_error(char const*) {}
void __throw_bad_alloc() {}
void __throw_out_of_range(char const*) {}
void __throw_logic_error(char const*) {}
void __throw_out_of_range_fmt(char const*, ...) {}
}

#include <Arduino.h>
#include "/Users/dda/Library/Arduino15/packages/rakwireless/hardware/nrf52/1.0.1/libraries/Adafruit_TinyUSB_Arduino/src/Adafruit_TinyUSB.h"
#include "Utilities.h"

uint8_t ix = 0;
vector<string> userStrings;
char UTC[7] = {0};
uint8_t SIV = 0;

float latitude, longitude;

float parseDegrees(const char *term) {
  float value = (float)(atof(term) / 100.0);
  uint16_t left = (uint16_t)value;
  value = (value - left) * 1.66666666666666;
  value += left;
  return value;
}

vector<string> parseNMEA(string nmea) {
  vector<string>result;
  if (nmea.at(0) != '$') {
    Serial.println("Not an NMEA sentence!");
    return result;
  }
  size_t lastFound = 0;
  size_t found = nmea.find(",", lastFound);
  while (found < nmea.size() && found != string::npos) {
    string token = nmea.substr(lastFound, found - lastFound);
    result.push_back(token);
    lastFound = found + 1;
    found = nmea.find(",", lastFound);
  }
  string token = nmea.substr(lastFound, found - lastFound);
  result.push_back(token);
  lastFound = found + 1;
  found = nmea.find(",", lastFound);
  return result;
}

void parseGPRMC(vector<string>result) {
  if (result.at(1) != "") {
    sprintf(buffer, "[%s] %s:%s:%s UTC", result.at(0).c_str(), result.at(1).substr(0, 2).c_str(), result.at(1).substr(2, 2).c_str(), result.at(1).substr(4, 2).c_str());
    Serial.println(buffer);
  }
  // if (result.at(2) == "V") Serial.println("Invalid fix!");
  // else Serial.println("Valid fix!");
  if (result.at(3) != "") {
    float newLatitude, newLongitude;
    newLatitude = parseDegrees(result.at(3).c_str());
    newLongitude = parseDegrees(result.at(5).c_str());
    if (newLatitude != latitude || newLongitude != longitude) {
      latitude = newLatitude;
      longitude = newLongitude;
      sprintf(buffer, "[%s] Coordinates: %3.8f %c, %3.8f %c\n", result.at(0).c_str(), latitude, result.at(4).c_str()[0], longitude, result.at(6).c_str()[0]);
      Serial.print(buffer);
    }
  }
}

void parseGPGGA(vector<string>result) {
  if (result.at(1) != "") {
    sprintf(buffer, "[%s] %s:%s:%s UTC", result.at(0).c_str(), result.at(1).substr(0, 2).c_str(), result.at(1).substr(2, 2).c_str(), result.at(1).substr(4, 2).c_str());
    Serial.println(buffer);
  }
  //  if (result.at(6) == "0") Serial.println("Invalid fix!");
  //  else Serial.println("Valid fix!");
  if (result.at(2) != "") {
    latitude = parseDegrees(result.at(2).c_str());
    longitude = parseDegrees(result.at(4).c_str());
    sprintf(buffer, "[%s] Coordinates: %3.8f %c, %3.8f %c\n", result.at(0).c_str(), latitude, result.at(3).c_str()[0], longitude, result.at(5).c_str()[0]);
    Serial.print(buffer);
  }
}

void parseGPZDA(vector<string>result) {
  if (result.at(1) != "") {
    sprintf(buffer, "[%s] Time: %s:%s:%s UTC", result.at(0).c_str(), result.at(1).substr(0, 2).c_str(), result.at(1).substr(2, 2).c_str(), result.at(1).substr(4, 2).c_str());
    Serial.println(buffer);
  }
  if (result.at(2) != "") {
    sprintf(buffer, "[%s] Date: %s/%s/%s UTC", result.at(0).c_str(), result.at(4).c_str(), result.at(3).c_str(), result.at(2));
    Serial.println(buffer);
  }
}

void parseGPGLL(vector<string>result) {
  if (result.at(1) != "") {
    latitude = parseDegrees(result.at(1).c_str());
    longitude = parseDegrees(result.at(3).c_str());
    sprintf(buffer, "[%s] Coordinates: %3.8f %c, %3.8f %c\n", result.at(0).c_str(), latitude, result.at(2).c_str()[0], longitude, result.at(4).c_str()[0]);
    Serial.print(buffer);
  }
}

void parseGPGSV(vector<string>result) {
  if (result.at(1) != "") {
    uint8_t newSIV = atoi(result.at(3).c_str());
    if (SIV != newSIV) {
      sprintf(buffer, "[%s] Message %s / %s. SIV: %s\n", result.at(0).c_str(), result.at(2).c_str(), result.at(1).c_str(), result.at(3).c_str());
      Serial.print(buffer);
      SIV = newSIV;
    }
  }
}

void parseGPTXT(vector<string>result) {
  //$GPTXT, 01, 01, 02, ANTSTATUS = INIT
  if (result.at(1) != "") {
    sprintf(buffer, " . Message %s / %s. Severity: %s\n . Message text: %s\n",
            result.at(2).c_str(), result.at(1).c_str(), result.at(3).c_str(), result.at(4).c_str(), result.at(5).c_str());
    Serial.print(buffer);
  }
}

void parseGPVTG(vector<string>result) {
  Serial.println("Track Made Good and Ground Speed.");
  if (result.at(1) != "") {
    sprintf(buffer, " . True track made good %s [%s].\n", result.at(1).c_str(), result.at(2).c_str());
    Serial.print(buffer);
  }
  if (result.at(3) != "") {
    sprintf(buffer, " . Magnetic track made good %s [%s].\n", result.at(3).c_str(), result.at(4).c_str());
    Serial.print(buffer);
  }
  if (result.at(5) != "") {
    sprintf(buffer, " . Speed: %s %s.\n", result.at(5).c_str(), result.at(6).c_str());
    Serial.print(buffer);
  }
  if (result.at(7) != "") {
    sprintf(buffer, " . Speed: %s %s.\n", result.at(7).c_str(), result.at(8).c_str());
    Serial.print(buffer);
  }
}

void parseGPGSA(vector<string>result) {
  // $GPGSA,A,3,15,29,23,,,,,,,,,,12.56,11.96,3.81
  Serial.println("GPS DOP and active satellites");
  if (result.at(1) == "A") Serial.println(" . Mode: Automatic");
  else if (result.at(1) == "M") Serial.println(" . Mode: Manual");
  else Serial.println(" . Mode: ???");
  if (result.at(2) == "1") {
    Serial.println(" . Fix not available.");
    return;
  } else if (result.at(2) == "2") Serial.println(" . Fix: 2D");
  else if (result.at(2) == "3") Serial.println(" . Fix: 3D");
  else {
    Serial.println(" . Fix: ???");
    return;
  }
  Serial.print(" . PDOP: "); Serial.println(result.at(result.size() - 3).c_str());
  Serial.print(" . HDOP: "); Serial.println(result.at(result.size() - 2).c_str());
  Serial.print(" . VDOP: "); Serial.println(result.at(result.size() - 1).c_str());
}

void setup() {
  Serial.begin(115200);
  initCommands();
  time_t timeout = millis();
  while (!Serial) {
    if ((millis() - timeout) < 5000) {
      delay(100);
    } else {
      break;
    }
  }
  delay(2500);
  Serial.println("\nGPS Example (NMEA Parser)");
  Serial.println("Turning off port");
  pinMode(WB_IO2, OUTPUT); // SLOT_A SLOT_B
  digitalWrite(WB_IO2, 0);
  delay(100);
  Serial.println("Turning on port");
  digitalWrite(WB_IO2, 1);
  delay(100);
  Serial1.begin(9600);
  delay(100);
  Serial.println("Serial1 ready!");
  delay(1000);
  SendCmd0B(MONVER[0], MONVER[1], (char*)"MON-VER");
}

bool waitForDollar = true;

bool isItB562() {
  char c = Serial1.read();
  if (c == 0x62) {
    memset(B562, 0, 256);
    uint8_t posx = 0, i;
    B562[posx++] = Serial1.read(); // Class
    B562[posx++] = Serial1.read(); // ID
    B562[posx++] = Serial1.read(); // Length L
    B562[posx++] = Serial1.read(); // Length H
    uint16_t len = B562[posx - 2];
    sprintf(infoText, "B562 incoming: %2x %2x payload length: %d\n", B562[0], B562[1], len);
    if (len > 0)
      for (i = 0; i < len; i++) B562[posx++] = Serial1.read();
    B562[posx++] = Serial1.read(); // CK_A
    B562[posx++] = Serial1.read(); // CK_B
    Serial.println("B562:");
    hexDump((unsigned char*)B562, posx);
    if (B562[0] == 0x05) handleACK();
    if (B562[0] == MONGNSS[0] && B562[1] == MONGNSS[1]) handleMONGNSS();
    if (B562[0] == CFGDAT[0] && B562[1] == CFGDAT[1]) F0x060x06();
    return true;
  }
  return false;
}

void loop() {
  if (Serial.available()) {
    char incoming[256];
    uint8_t posx = 0;
    char c = Serial.read();
    while (Serial.available()) {
      if (c > 31) incoming[posx++] = c;
      else if (c == 13 || c == 10) {
        while (Serial.available()) c = Serial.read();
        break;
      }
      c = Serial.read();
    }
    incoming[posx] = 0;
    if (incoming[0] == '/') {
      // / command
      Serial.print("> ");
      Serial.println(incoming + 1);
      std::map<string, void (*)()>::iterator it;
      it = commands.find(string(incoming));
      if (it == commands.end()) {
        Serial.println("Not a known command!");
        showHelp();
      } else {
        it->second();
      }
    } else {
      Serial.println("Not a command!");
    }
  }

  if (Serial1.available()) {
    char c = Serial1.read();
    if (c == 0xB5) {
      isItB562();
      return;
    }
    if (waitForDollar && c == '$') {
      waitForDollar = false;
      buffer[0] = '$';
      ix = 1;
    } else if (waitForDollar == false) {
      if (c == 13) {
        buffer[ix] = 0;
        c = Serial1.read();
        delay(50);
        string nextLine = string(buffer);
        userStrings.push_back(nextLine.substr(0, nextLine.size() - 3));
        waitForDollar = true;
      } else if (c == 0xB5) {
        if (isItB562()) return;
      } else {
        buffer[ix++] = c;
      }
    }
  }
  if (userStrings.size() > 0) {
    string nextLine = userStrings[0];
    userStrings.erase(userStrings.begin());
    if (nextLine.substr(0, 1) != "$") {
      // Serial.print("Not an NMEA string!\n>> ");
      // Serial.println(nextLine.c_str());
      return;
    } else {
      vector<string>result = parseNMEA(nextLine);
      if (result.size() == 0) return;
      string verb = result.at(0);
      if (verb.substr(3, 3) == "RMC") {
        parseGPRMC(result);
      } else if (verb.substr(3, 3) == "GSV") {
        parseGPGSV(result);
      } else if (verb.substr(3, 3) == "GGA") {
        parseGPGGA(result);
      } else if (verb.substr(3, 3) == "GLL") {
        parseGPGLL(result);
      } else if (verb.substr(3, 3) == "GSA") {
        //parseGPGSA(result);
      } else if (verb.substr(3, 3) == "VTG") {
        //parseGPVTG(result);
      } else if (verb.substr(3, 3) == "ZDA") {
        parseGPZDA(result);
      } else if (verb.substr(3, 3) == "TXT") {
        parseGPTXT(result);
      } else {
        Serial.println(nextLine.c_str());
      }
    }
  }
}
