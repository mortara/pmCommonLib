#include "i2cscanner.hpp"
#include "../pmCommonLib.hpp"
#include <String.h>
//
//    FILE: MultiSpeedI2CScanner.ino
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.16
// PURPOSE: I2C scanner at different speeds
//    DATE: 2013-11-05
//     URL: https://github.com/RobTillaart/MultiSpeedI2CScanner
//     URL: http://forum.arduino.cc/index.php?topic=197360
//


///////////////////////////////////////////////////////////////////////////
//
// MAIN CODE
//
void I2CScanner::setup()
{
#if defined (ESP8266) || defined(ESP32)
  uint8_t sda = 21;  //  21
  uint8_t scl = 22;  //  22
  Wire.begin(sda, scl, (uint8_t)100000);  //  ESP32 - change config pins if needed.
#else
  Wire.begin();
#endif

#if defined WIRE_IMPLEMENT_WIRE1 || WIRE_INTERFACES_COUNT > 1
  Wire1.begin();
  wirePortCount++;
#endif
#if defined WIRE_IMPLEMENT_WIRE2 || WIRE_INTERFACES_COUNT > 2
  Wire2.begin();
  wirePortCount++;
#endif
#if defined WIRE_IMPLEMENT_WIRE3 || WIRE_INTERFACES_COUNT > 3
  Wire3.begin();
  wirePortCount++;
#endif
#if defined WIRE_IMPLEMENT_WIRE4 || WIRE_INTERFACES_COUNT > 4
  Wire4.begin();
  wirePortCount++;
#endif
#if defined WIRE_IMPLEMENT_WIRE5 || WIRE_INTERFACES_COUNT > 5
  Wire5.begin();
  wirePortCount++;
#endif

  wire = &Wire;

  pmCommonLib.WebSerial.println("");
  reset();
}


void I2CScanner::loop()
{
  yield();
  char command = getCommand();
  switch (command)
  {
    case '@':
      selectedWirePort = (selectedWirePort + 1) % wirePortCount;
      pmCommonLib.WebSerial.print("<I2C PORT=Wire");
      pmCommonLib.WebSerial.print(String(selectedWirePort));
      pmCommonLib.WebSerial.println(">");
      switch (selectedWirePort)
      {
        case 0:
          wire = &Wire;
          break;
#if defined WIRE_IMPLEMENT_WIRE1 || WIRE_INTERFACES_COUNT > 1
        case 1:
          wire = &Wire1;
          break;
#endif
#if defined WIRE_IMPLEMENT_WIRE2 || WIRE_INTERFACES_COUNT > 2
        case 2:
          wire = &Wire2;
          break;
#endif
#if defined WIRE_IMPLEMENT_WIRE3 || WIRE_INTERFACES_COUNT > 3
        case 3:
          wire = &Wire3;
          break;
#endif
#if defined WIRE_IMPLEMENT_WIRE4 || WIRE_INTERFACES_COUNT > 4
        case 4:
          wire = &Wire4;
          break;
#endif
#if defined WIRE_IMPLEMENT_WIRE5 || WIRE_INTERFACES_COUNT > 5
        case 5:
          wire = &Wire5;
          break;
#endif
      }
      break;

    case 's':
      state = ONCE;
      break;
    case 'c':
      state = CONT;
      break;
    case 'd':
      delayFlag = !delayFlag;
      pmCommonLib.WebSerial.print("<delay=");
      pmCommonLib.WebSerial.println(delayFlag ? "5>" : "0>");
      break;

    case 'e':
      // eeprom test TODO
      break;

    case 'h':
      header = !header;
      pmCommonLib.WebSerial.print("<header=");
      pmCommonLib.WebSerial.println(header ? "yes>" : "no>");
      break;
    case 'p':
      printAll = !printAll;
      pmCommonLib.WebSerial.print("<print=");
      pmCommonLib.WebSerial.println(printAll ? "all>" : "found>");
      break;
    case 'i':
      disableIRQ = !disableIRQ;
      pmCommonLib.WebSerial.print("<irq=");
      pmCommonLib.WebSerial.println(disableIRQ ? "diabled>" : "enabled>");
      break;

    case '0':
    case '1':
    case '2':
    case '4':
    case '8':
    case '9':
    case 'M':
    case 'N':
    case 'O':
    case 'P':
      setSpeed(command);
      break;

    case 'r':
      reset();
      break;

    case 'a':
      setAddress();
      break;

    case 'q':
      ESP.restart();
      break;
    case '?':
      state = HELP;
      break;
    default:
      break;
  }

  switch (state)
  {
    case ONCE:
      I2Cscan();
      state = HELP;
      break;
    case CONT:
      I2Cscan();
      delay(1000);
      break;
    case HELP:
      displayHelp();
      state = STOP;
      break;
    case STOP:
      break;
    default: // ignore all non commands
      break;
  }
}

//////////////////////////////////////////////////////////////////////

void I2CScanner::reset()
{
  setSpeed('9');
  selectedWirePort = 0;
  addressStart     = 8;
  addressEnd       = 119;
  
  delayFlag  = false;
  printAll   = true;
  header     = true;
  disableIRQ = false;

  state = STOP;

  displayHelp();
}


void I2CScanner::setAddress()
{
  if (addressStart == 0)
  {
    addressStart = 8;
    addressEnd = 119;
  }
  else
  {
    addressStart = 0;
    addressEnd = 127;
  }
  pmCommonLib.WebSerial.print("<address Range = ");
  pmCommonLib.WebSerial.print(String(addressStart));
  pmCommonLib.WebSerial.print("..");
  pmCommonLib.WebSerial.print(String(addressEnd));
  pmCommonLib.WebSerial.println(">");

}


void I2CScanner::setSpeed(char sp)
{
  switch (sp)
  {
    case '1':
      speed[0] = 100;
      speeds = 1;
      break;
    case '2':
      speed[0] = 200;
      speeds = 1;
      break;
    case '4':
      speed[0] = 400;
      speeds = 1;
      break;
    case '8':
      speed[0] = 800;
      speeds = 1;
      break;
    case '9':  // limited to 400 KHz
      speeds = 8;
      for (int i = 1; i <= speeds; i++) speed[i - 1] = i * 50;
      break;
    case '0':  // limited to 800 KHz
      speeds = 8;
      for (int i = 1; i <= speeds; i++) speed[i - 1] = i * 100;
      break;

    // new in 0.1.10 - experimental
    case 'M':
      speed[0] = 1000;
      speeds = 1;
      break;
    case 'N':
      speed[0] = 3400;
      speeds = 1;
      break;
    case 'O':
      speed[0] = 5000;
      speeds = 1;
      break;
    case 'P':
      speed[0] = 100;
      speed[1] = 400;
      speed[2] = 1000;
      speed[3] = 3400;
      speed[4] = 5000;
      speeds = 5;
      break;
  }
  pmCommonLib.WebSerial.print("<speeds =");
  for (int i = 0; i < speeds; i++)
  {
    pmCommonLib.WebSerial.print(" ");
    pmCommonLib.WebSerial.print(String(speed[i]));
  }
  pmCommonLib.WebSerial.println(" >");
}


char I2CScanner::getCommand()
{
  char c = '\0';
  if (Serial.available())
  {
    c = Serial.read();
  }

  if(c == '\0')
    c = pmCommonLib.WebSerial.GetInput();

  return c;
}


void I2CScanner::displayHelp()
{
  pmCommonLib.WebSerial.print("\nArduino MultiSpeed I2C Scanner - ");
  pmCommonLib.WebSerial.println(version);
  pmCommonLib.WebSerial.println();
  pmCommonLib.WebSerial.print("I2C ports: ");
  pmCommonLib.WebSerial.print(String(wirePortCount));
  pmCommonLib.WebSerial.print("  Current: Wire");
  pmCommonLib.WebSerial.println(String(selectedWirePort));
  pmCommonLib.WebSerial.println("\t@ = toggle Wire - Wire1 .. Wire5 [e.g. TEENSY or Arduino Due]");

  pmCommonLib.WebSerial.println("Scan mode:");
  pmCommonLib.WebSerial.println("\ts = single scan");
  pmCommonLib.WebSerial.println("\tc = continuous scan - 1 second delay");
  pmCommonLib.WebSerial.println("\tq = quit continuous scan");
  pmCommonLib.WebSerial.println("\td = toggle latency delay between successful tests. 0 - 5 ms");
  pmCommonLib.WebSerial.println("\ti = toggle enable/disable interrupts");

  pmCommonLib.WebSerial.println("Output:");

  pmCommonLib.WebSerial.println("\tp = toggle printAll - printFound.");
  pmCommonLib.WebSerial.println("\th = toggle header - noHeader.");
  pmCommonLib.WebSerial.println("\ta = toggle address range, 0..127 - 8..119 (default)");

  pmCommonLib.WebSerial.println("Speeds:");
  pmCommonLib.WebSerial.println("\t0 = 100..800 KHz - step 100  (warning - can block!!)");
  pmCommonLib.WebSerial.println("\t1 = 100 KHz");
  pmCommonLib.WebSerial.println("\t2 = 200 KHz");
  pmCommonLib.WebSerial.println("\t4 = 400 KHz");
  pmCommonLib.WebSerial.println("\t9 = 50..400 KHz - step 50     < DEFAULT >");
  pmCommonLib.WebSerial.println();
  pmCommonLib.WebSerial.println("\t!! HIGH SPEEDS - WARNING - can block - not applicable for UNO");
  pmCommonLib.WebSerial.println("\t8 =  800 KHz");
  pmCommonLib.WebSerial.println("\tM = 1000 KHz");
  pmCommonLib.WebSerial.println("\tN = 3400 KHz");
  pmCommonLib.WebSerial.println("\tO = 5000 KHz");
  pmCommonLib.WebSerial.println("\tP = 100 400 1000 3400 5000 KHz (standards)");
  pmCommonLib.WebSerial.println("\n\tr = reset to startup defaults.");
  pmCommonLib.WebSerial.println("\t? = help - this page");
  pmCommonLib.WebSerial.println();
}

void I2CScanner::I2Cscan()
{
  startScan = millis();
  uint8_t count = 0;

  if (disableIRQ) noInterrupts();

  if (header)
  {
    pmCommonLib.WebSerial.print("TIME\tDEC\tHEX\t");
    for (uint8_t s = 0; s < speeds; s++)
    {
      pmCommonLib.WebSerial.print("\t");
      pmCommonLib.WebSerial.print(String(speed[s]));
    }
    pmCommonLib.WebSerial.println("\t[KHz]");
    for (uint8_t s = 0; s < speeds + 5; s++)
    {
      pmCommonLib.WebSerial.print("--------");
    }
    pmCommonLib.WebSerial.println();
    delay(100);
  }

  for (uint8_t address = addressStart; address <= addressEnd; address++)
  {
    bool printLine = printAll;
    bool found[speeds];
    bool fnd = false;

    for (uint8_t s = 0; s < speeds ; s++)
    {
      yield();    // keep ESP happy

#if ARDUINO < 158 && defined (TWBR)
      uint16_t PREV_TWBR = TWBR;
      TWBR = (F_CPU / (speed[s] * 1000) - 16) / 2;
      if (TWBR < 2)
      {
        Serial.println("ERROR: not supported speed");
        TWBR = PREV_TWBR;
        return;
      }
#else
      wire->setClock(speed[s] * 1000UL);
#endif
      wire->beginTransmission (address);
      found[s] = (wire->endTransmission () == 0);
      fnd |= found[s];
      // give device 5 millis
      if (fnd && delayFlag) delay(RESTORE_LATENCY);
    }

    if (fnd) count++;
    printLine |= fnd;

    if (printLine)
    {
      pmCommonLib.WebSerial.print(String(millis()));
      pmCommonLib.WebSerial.print("\t");
      pmCommonLib.WebSerial.print(String(address));
      pmCommonLib.WebSerial.print("\t0x");
      if (address < 0x10) pmCommonLib.WebSerial.print("0");

      char hexadecimalnum [3];
      sprintf(hexadecimalnum, "%X", address);

      pmCommonLib.WebSerial.print(hexadecimalnum);
      pmCommonLib.WebSerial.print("\t");

      for (uint8_t s = 0; s < speeds ; s++)
      {
        pmCommonLib.WebSerial.print("\t");
        pmCommonLib.WebSerial.print(found[s] ? "V" : ".");
      }
      pmCommonLib.WebSerial.println();
    }
  }

/*
  //  FOOTER
  if (header)
  {
    for (uint8_t s = 0; s < speeds + 5; s++)
    {
      Serial.print(F("--------"));
    }
    Serial.println();

    Serial.print(F("TIME\tDEC\tHEX\t"));
    for (uint8_t s = 0; s < speeds; s++)
    {
      Serial.print(F("\t"));
      Serial.print(speed[s]);
    }
    Serial.println(F("\t[KHz]"));
  }
*/

  stopScan = millis();
  if (header)
  {
    pmCommonLib.WebSerial.println();
    pmCommonLib.WebSerial.print(String(count));
    pmCommonLib.WebSerial.print(" devices found in ");
    pmCommonLib.WebSerial.print(String(stopScan - startScan));
    pmCommonLib.WebSerial.println(" milliseconds.");
  }

  interrupts();
}


// -- END OF FILE --
