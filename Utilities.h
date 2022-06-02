#include <stdio.h>
#include <map>
using namespace std;

void hexDump(unsigned char *, uint16_t);
void Fletcher(uint8_t *, uint8_t);
void handleACK();
void handleMONGNSS();
void sendMONVER();
void sendNAVX5();
void sendCFGPRT();
void sendCFGDAT();
void sendCFGGNSS();
void sendMONGNSS();
void sendCFGNMEA();
void sendBDSON();
void sendCFGPRTx(uint8_t);
void sendCFGPRT0();
void sendCFGPRT1();
void sendCFGPRT2();
void sendCFGPRT3();
void sendCFGPRT4();
void sendCFGRATE();
void F0x060x00();
void F0x060x06();
void F0x060x08();
void F0x0a0x04();

void showHelp();

char buffer[256];
char B562[256];
char infoText[128];

std::map<string, void (*)()> commands;

#define GPSSup 1
#define GlonassSup 2
#define BeidouSup 4
#define GalileoSup 8

uint16_t readUint16(char *ptr) {
  return (ptr[0] | ptr[1] << 8);
}

uint32_t readUint32(char *ptr) {
  return (ptr[0] | ptr[1] << 8 | ptr[1] << 16 | ptr[1] << 24);
}

void initCommands() {
  commands["/MON-VER"] = &sendMONVER;
  commands["/CFG-NAVX5"] = &sendNAVX5;
  commands["/CFG-PRT"] = &sendCFGPRT;
  commands["/CFG-PRT 0"] = &sendCFGPRT0;
  commands["/CFG-PRT 1"] = &sendCFGPRT1;
  commands["/CFG-PRT 2"] = &sendCFGPRT2;
  commands["/CFG-PRT 3"] = &sendCFGPRT3;
  commands["/CFG-PRT 4"] = &sendCFGPRT4;
  commands["/CFG-DAT"] = &sendCFGDAT;
  commands["/MON-GNSS"] = &sendMONGNSS;
  commands["/CFG-GNSS"] = &sendCFGGNSS;
  commands["/CFG-NMEA"] = &sendCFGNMEA;
  commands["/BDS ON"] = &sendBDSON;
  commands["/CFG-RATE"] = &sendCFGRATE;

  commands["/?"] = &showHelp;
}

void hexDump(unsigned char *buf, uint16_t len) {
  char alphabet[17] = "0123456789abcdef";
  Serial.print(F("   +------------------------------------------------+ +----------------+\n"));
  Serial.print(F("   |.0 .1 .2 .3 .4 .5 .6 .7 .8 .9 .a .b .c .d .e .f | |      ASCII     |\n"));
  for (uint16_t i = 0; i < len; i += 16) {
    if (i % 128 == 0)
      Serial.print(F("   +------------------------------------------------+ +----------------+\n"));
    char s[] = "|                                                | |                |\n";
    uint8_t ix = 1, iy = 52;
    for (uint8_t j = 0; j < 16; j++) {
      if (i + j < len) {
        uint8_t c = buf[i + j];
        s[ix++] = alphabet[(c >> 4) & 0x0F];
        s[ix++] = alphabet[c & 0x0F];
        ix++;
        if (c > 31 && c < 128) s[iy++] = c;
        else s[iy++] = '.';
      }
    }
    uint8_t index = i / 16;
    if (i < 256) Serial.write(' ');
    Serial.print(index, HEX); Serial.write('.');
    Serial.print(s);
  }
  Serial.print(F("   +------------------------------------------------+ +----------------+\n"));
}

void Fletcher(uint8_t *mb, uint8_t len) {
  uint8_t i, CK_A, CK_B;
  CK_A = 0;
  CK_B = 0;
  for (i = 2; i < len; i++) {
    CK_A += mb[i];
    CK_B += CK_A;
  }
  mb[len] = CK_A;
  mb[len + 1] = CK_B;
}

void handleACK() {
  //  05 01 02 00 06 3e 4c 75
  uint8_t response = B562[1];
  char temp[48];
  Serial.println("   +------------------------------------------------+");
  if (response == 1) sprintf(temp, "ACK-ACK. Class: %#2X, ID: %#2X", B562[4], B562[5]);
  else sprintf(temp, "ACK-NACK. Class: 0x%2X, ID: 0x%#2X", B562[4], B562[5]);
  sprintf(infoText, "   |%32s                |", temp);
  Serial.println(infoText);
  Serial.println("   +------------------------------------------------+");
}

void handleMONGNSS() {
  // 0a 28 08 00 00 0f 03 03 03 00 00 00 52 16
  sprintf(infoText, "Version: %d", B562[4]);
  Serial.println(infoText);
  if (B562[5] & GPSSup) Serial.println(" * GPS supported");
  else Serial.println(" * GPS not supported");
  if (B562[5] & GlonassSup) Serial.println(" * GLONASS supported");
  else Serial.println(" * GLONASS not supported");
  if (B562[5] & BeidouSup) Serial.println(" * Beidou supported");
  else Serial.println(" * Beidou not supported");
  if (B562[5] & GalileoSup) Serial.println(" * Galileo supported");
  else Serial.println(" * Galileo not supported");
  if (B562[6] & GPSSup) Serial.println(" * GPS enabled by default");
  else Serial.println(" * GPS not enabled by default");
  if (B562[6] & GlonassSup) Serial.println(" * GLONASS enabled by default");
  else Serial.println(" * GLONASS not enabled by default");
  if (B562[6] & BeidouSup) Serial.println(" * Beidou enabled by default");
  else Serial.println(" * Beidou not enabled by default");
  if (B562[6] & GalileoSup) Serial.println(" * Galileo enabled by default");
  else Serial.println(" * Galileo not enabled by default");
  if (B562[7] & GPSSup) Serial.println(" * GPS enabled");
  else Serial.println(" * GPS not enabled");
  if (B562[7] & GlonassSup) Serial.println(" * GLONASS enabled");
  else Serial.println(" * GLONASS not enabled");
  if (B562[7] & BeidouSup) Serial.println(" * Beidou enabled");
  else Serial.println(" * Beidou not enabled");
  if (B562[7] & GalileoSup) Serial.println(" * Galileo enabled");
  else Serial.println(" * Galileo not enabled");
  sprintf(infoText, "Maximum number of concurrent major GNSS that can be supported by this receiver: %d", B562[8]);
  Serial.println(infoText);
}

void SendCmd0B(uint8_t myClass, uint8_t myID, char *cmd) {
  uint8_t mb[8];
  memset(mb, 0, 8);
  mb[0] = 0xB5; // Sync Char 1
  mb[1] = 0x62; // Sync Char 2
  mb[2] = myClass;
  mb[3] = myID;
  mb[4] = 0x00;
  mb[5] = 0x00; // Payload Length, Little Endian, 16-bit
  // hexDump(mb, 8);
  Fletcher(mb, 6);
  Serial.print("Sending ");
  Serial.print(cmd);
  Serial.println(" command:");
  hexDump(mb, 8);
  Serial1.write((char *)mb, 8);
  delay(100);
}

void sendMONVER() {
  SendCmd0B(MONVER[0], MONVER[1], (char*)"MON-VER");
}

void sendNAVX5() {
  SendCmd0B(NAVX5[0], NAVX5[1], (char*)"UBX-CFG-NAVX5");
}

void sendCFGPRT() {
  SendCmd0B(CFGPRT[0], CFGPRT[1], (char*)"CFG-PRT");
}

void sendCFGRATE() {
  SendCmd0B(CFGRATE[0], CFGRATE[1], (char*)"CFG-RATE");
}

void sendCFGPRTx(uint8_t port) {
  ONEBYTE[2] = CFGPRT[0];
  ONEBYTE[3] = CFGPRT[1];
  ONEBYTE[6] = port;
  Fletcher(ONEBYTE, 7);
  Serial.print("Sending CFG-PRT ");
  Serial.print(port);
  Serial.println(" command:");
  hexDump(ONEBYTE, 9);
  Serial1.write((char *)ONEBYTE, 9);
  delay(100);
}

void sendCFGPRT0() {
  sendCFGPRTx(0);
}

void sendCFGPRT1() {
  sendCFGPRTx(1);
}

void sendCFGPRT2() {
  sendCFGPRTx(2);
}

void sendCFGPRT3() {
  sendCFGPRTx(3);
}

void sendCFGPRT4() {
  sendCFGPRTx(4);
}

void sendCFGDAT() {
  SendCmd0B(CFGDAT[0], CFGDAT[1], (char*)"CFG-DAT");
}

void sendCFGNMEA() {
  SendCmd0B(CFGNMEA[0], CFGNMEA[1], (char*)"CFG-NMEA");
}

void sendCFGRST() {
  SendCmd0B(CFGRST[0], CFGRST[1], (char*)"CFG-RST");
}

void sendCFGGNSS() {
  Serial.println("Sending CFG-GNSS:");
  hexDump(CFGGNSS, 68);
  Serial1.write((char*)CFGGNSS, 68);
  delay(100);
}

void sendMONGNSS() {
  Serial.println("Sending MON-GNSS:");
  hexDump(MONGNSS, 16);
  Serial1.write((char*)MONGNSS, 16);
  delay(100);
}

void showHelp() {
  // Display all of the mapped functions
  Serial.println("Commands:");
  std::map<string, void (*)()>::iterator it;
  for (it = commands.begin(); it != commands.end(); ++it) {
    Serial.print(" > ");
    Serial.println(it->first.c_str());
  }
}

void enableConstellation(uint8_t constellation) {
  CFGGNSS[constellation] = 1;
  Fletcher(CFGGNSS, 68);
  Serial.println("Sending CFG-GNSS command:");
  hexDump(CFGGNSS, 68);
  Serial1.write((char *)CFGGNSS, 68);
  delay(100);
}

void sendBDSON() {
  enableConstellation(beidouOffset);
  delay(1000);
  MONGNSS[9] |= BeidouSup;
  sendMONGNSS();
  delay(100);
}

void F0x060x06() {
  Serial.println("CFG-DAT");
  uint16_t ln = readUint16(&B562[2]);
  Serial.println(" . Payload length: " + String(ln));
  if (ln != 52) {
    Serial.println(" . Wrong Payload length");
    return;
  }

  uint8_t ix = 4; // Offset
  uint16_t dn = readUint16(&B562[ix]);
  sprintf(infoText, " . Datum Number: %d\n", dn);
  Serial.print(infoText);
  sprintf(infoText, " . Datum name: %s\n", B562[ix + 2]);
  Serial.print(infoText);

  double R8 = (double)B562[ix + 8];
  sprintf(infoText, " . Semi-major Axis: %.2f\n", R8);
  Serial.print(infoText);

  R8 = (float)B562[ix + 16];
  sprintf(infoText, " . 1.0 / Flattening: %.2f\n", R8);
  Serial.print(infoText);

  float R4 = (float)B562[ix + 24];
  sprintf(infoText, " . dX: %.2f\n", R4);
  Serial.print(infoText);

  R4 = (float)B562[ix + 28];
  sprintf(infoText, " . dY: %.2f\n", R4);
  Serial.print(infoText);

  R4 = (float)B562[ix + 32];
  sprintf(infoText, " . dZ: %.2f\n", R4);
  Serial.print(infoText);

  R4 = (float)B562[ix + 36];
  sprintf(infoText, " . rotX: %.2f\n", R4);
  Serial.print(infoText);

  R4 = (float)B562[ix + 40];
  sprintf(infoText, " . rotY: %.2f\n", R4);
  Serial.print(infoText);

  R4 = (float)B562[ix + 44];
  sprintf(infoText, " . rotZ: %.2f\n", R4);
  Serial.print(infoText);
  R4 = (float)B562[ix + 44];
  sprintf(infoText, " . scale: %.2f\n", R4);
}

void F0x060x00() {
  Serial.println("CFG-PRT");
  uint16_t ln = readUint16(&B562[2]);
  Serial.println(" . Payload length: " + String(ln));
  if (ln != 20) {
    Serial.println(" . Wrong Payload length");
    return;
  }

  uint8_t ix = 4; // Offset
  uint8_t PortID = B562[ix];
  Serial.println(" . Port ID: " + String(PortID));
  // 1: reserved

  uint16_t bf16 = readUint16(&B562[ix + 2]); // 2: txReady
  // txReady
  if ((bf16 & 1) == 0) {
    // bit 1: en[able]
    Serial.println(" . TX disabled");
  } else {
    Serial.println(" . TX enabled");
    uint16_t z = bf16 & 2;
    if (z == 0) {
      Serial.println(" . Polarity: High-active");
    } else {
      Serial.println(" . Polarity: Low-active");
    }
    z = (bf16 & 2) >> 0b11111;
    Serial.println(" . PIN: " + String(z));
    z = (bf16 & 7) >> 0b111111111;
    Serial.println(" . Threshold: " + String(z) + " x 8 = " + String(z * 8));
    if (PortID == 1 || PortID == 2) {
      uint32_t bf32 = readUint32(&B562[ix + 4]); // 4: mode
      z = (bf32 & 6) >> 0b11;
      Serial.println(" . Char Len: " + String(z + 3));
      z = (bf32 & 9) >> 0b111;
      if (z == 0) {
        Serial.println(" . Even Parity");
      } else if (z == 1) {
        Serial.println(" . Odd Parity");
      } else if (z & 0b110 == 6) {
        Serial.println(" . No Parity");
      } else if (z & 0b010 == 2) {
        Serial.println(" . [Parity] Reserved");
      } else {
        Serial.println(" . [Parity] This should not happen");
      }
      z = (bf32 & 12) >> 0b11 * 5 + 10;
      sprintf(infoText, " . Stop bits: %3.1f", (z / 10));
      Serial.println(infoText);
      bf32 = readUint32(&B562[ix + 8]); // 8: baudRate
      Serial.println(" . Bauds/sec: " + String(bf32));
    }
    bf16 = readUint16(&B562[ix + 12]); // 12: inProtoMask
    Serial.println(" . Input protocols:");
    if ((bf16 & 1) == 1) Serial.println("  -> UBlox");
    if ((bf16 & 2) == 2) Serial.println("  -> NMEA");
    if ((bf16 & 4) == 4) Serial.println("  -> RTCM");
    bf16 = readUint16(&B562[ix + 14]); // 14: outProtoMask
    Serial.println(" . Output protocols:");
    if ((bf16 & 1) == 1) Serial.println("  -> UBlox");
    if ((bf16 & 2) == 2) Serial.println("  -> NMEA");
    if (PortID == 1 || PortID == 2) {
      bf16 = readUint16(&B562[ix + 16]); // 16: flags
      if ((bf16 & 2) == 2) Serial.println(" . Extended Tx Timeout");
    }
  }
}

void F0x060x08() {
  Serial.println("CFG-RATE");
  uint16_t ln = readUint16(&B562[2]);
  Serial.println(" . Payload length: " + String(ln));
  if (ln != 6) {
    Serial.println(" . Wrong Payload length");
    return;
  }
  uint8_t ix = 4; // Offset
  uint16_t z = readUint16(&B562[ix]); // 0: measRate
  sprintf(infoText, " . Measurement rate: %d ms", z);
  Serial.println(infoText);

  z = readUint16(&B562[ix + 2]); // 2: navRate
  if (z < 2) Serial.println(" . Navigation rate: " + String(z) + " cycle");
  else Serial.println(" . Navigation rate: " + String(z) + " cycles");

  z = readUint16(&B562[ix + 4]); // 4: timeRef
  if (z == 0) Serial.println(" . Reference time: UTC time");
  else Serial.println(" . Reference time: GPS time");
}

void F0x0a0x04() {
  Serial.println("MON-VER");
  uint16_t ln = readUint16(&B562[2]);
  Serial.println(" . Payload length: " + String(ln));
  uint8_t headerLength = 40;
  uint8_t nb = ln / headerLength;
  uint16_t ln0 = headerLength + nb * 30;
  Serial.println(" . numBlocks: " + String(nb));
  if (ln != ln0) {
    Serial.println(" . Wrong Payload length");
    return;
  }
  uint8_t ix = 4; // Offset
  memset(infoText, 0, 40);
  memcpy(infoText, B562 + ix, 30);
  Serial.print("swVersion: ");
  Serial.println(infoText);
  ix += 30;
  memset(infoText, 0, 40);
  memcpy(infoText, B562 + ix, 10);
  Serial.print("hwVersion: ");
  Serial.println(infoText);
  ix += 10;
  for (uint8_t i = 0; i < nb; i++) {
    memset(infoText, 0, 40);
    memcpy(infoText, B562 + ix, 30);
    Serial.print("extension ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(infoText);
    ix += 30;
  }
}

void F0x060x17() {
  Serial.print("CFG-NMEA");

  uint16_t ln = readUint16(&B562[2]);
  Serial.println(" . Payload length: " + String(ln));
  if (ln != 4) {
    Serial.println(" . Wrong Payload length");
    hexDump((unsigned char*)B562, ln + 2);
    return;
  }
  uint8_t ix = 4; // Offset
  uint8_t bf8 = B562[ix]; // 0: filter
  uint8_t z = bf8 & 0b000001;
  if (z == 0) {
    Serial.print(" . Position filtering enabled");
  } else {
    Serial.print(" . Position filtering disabled");
  }

  z = bf8 & 0b000010;
  if (z == 0) {
    Serial.print(" . Masked position filtering enabled");
  } else {
    Serial.print(" . Masked position filtering disabled");
  }

  z = bf8 & 0b000100;
  if (z == 0) {
    Serial.print(" . Time filtering enabled");
  } else {
    Serial.print(" . Time filtering disabled");
  }

  z = bf8 & 0b001000;
  if (z == 0) {
    Serial.print(" . Date filtering enabled");
  } else {
    Serial.print(" . Date filtering disabled");
  }

  z = bf8 & 0b010000;
  if (z == 0) {
    Serial.print(" . SBAS filtering enabled");
  } else {
    Serial.print(" . SBAS filtering disabled");
  }

  z = bf8 & 0b100000;
  if (z == 0) {
    Serial.print(" . Track filtering enabled");
  } else {
    Serial.print(" . Track filtering disabled");
  }

  z = B562[ix + 1]; // 1: version
  Serial.println(" . Version: " + String(z, HEX));

  z = B562[ix + 2]; // 1: numSV
  Serial.println(" . Maximum number of SV: " + String(z, HEX));

  bf8 = B562[ix + 3]; // 0: flags
  z = bf8 & 0b01;
  if (z == 1) {
    Serial.print(" . Compat mode enabled");
  } else {
    Serial.print(" . Compat mode disabled");
  }

}
