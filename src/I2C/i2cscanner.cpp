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

  pmLogging.LogLn("");
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
      pmLogging.Log("<I2C PORT=Wire");
      pmLogging.Log(String(selectedWirePort));
      pmLogging.LogLn(">");
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
      pmLogging.Log("<delay=");
      pmLogging.LogLn(delayFlag ? "5>" : "0>");
      break;

    case 'e':
      // eeprom test TODO
      break;

    case 'h':
      header = !header;
      pmLogging.Log("<header=");
      pmLogging.LogLn(header ? "yes>" : "no>");
      break;
    case 'p':
      printAll = !printAll;
      pmLogging.Log("<print=");
      pmLogging.LogLn(printAll ? "all>" : "found>");
      break;
    case 'i':
      disableIRQ = !disableIRQ;
      pmLogging.Log("<irq=");
      pmLogging.LogLn(disableIRQ ? "diabled>" : "enabled>");
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
  pmLogging.Log("<address Range = ");
  pmLogging.Log(String(addressStart));
  pmLogging.Log("..");
  pmLogging.Log(String(addressEnd));
  pmLogging.LogLn(">");

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
  pmLogging.Log("<speeds =");
  for (int i = 0; i < speeds; i++)
  {
    pmLogging.Log(" ");
    pmLogging.Log(String(speed[i]));
  }
  pmLogging.LogLn(" >");
}


char I2CScanner::getCommand()
{
  char c = '\0';
  if (Serial.available())
  {
    c = Serial.read();
  }

  #ifndef PMCOMMONNOWEBSERIAL
  if(c == '\0')
    c = pmCommonLib.WebSerial.GetInput();
  #endif
  
  return c;
}


void I2CScanner::displayHelp()
{
  pmLogging.Log("\nArduino MultiSpeed I2C Scanner - ");
  pmLogging.LogLn(version);
  pmLogging.LogLn();
  pmLogging.Log("I2C ports: ");
  pmLogging.Log(String(wirePortCount));
  pmLogging.Log("  Current: Wire");
  pmLogging.LogLn(String(selectedWirePort));
  pmLogging.LogLn("\t@ = toggle Wire - Wire1 .. Wire5 [e.g. TEENSY or Arduino Due]");

  pmLogging.LogLn("Scan mode:");
  pmLogging.LogLn("\ts = single scan");
  pmLogging.LogLn("\tc = continuous scan - 1 second delay");
  pmLogging.LogLn("\tq = quit continuous scan");
  pmLogging.LogLn("\td = toggle latency delay between successful tests. 0 - 5 ms");
  pmLogging.LogLn("\ti = toggle enable/disable interrupts");

  pmLogging.LogLn("Output:");

  pmLogging.LogLn("\tp = toggle printAll - printFound.");
  pmLogging.LogLn("\th = toggle header - noHeader.");
  pmLogging.LogLn("\ta = toggle address range, 0..127 - 8..119 (default)");

  pmLogging.LogLn("Speeds:");
  pmLogging.LogLn("\t0 = 100..800 KHz - step 100  (warning - can block!!)");
  pmLogging.LogLn("\t1 = 100 KHz");
  pmLogging.LogLn("\t2 = 200 KHz");
  pmLogging.LogLn("\t4 = 400 KHz");
  pmLogging.LogLn("\t9 = 50..400 KHz - step 50     < DEFAULT >");
  pmLogging.LogLn();
  pmLogging.LogLn("\t!! HIGH SPEEDS - WARNING - can block - not applicable for UNO");
  pmLogging.LogLn("\t8 =  800 KHz");
  pmLogging.LogLn("\tM = 1000 KHz");
  pmLogging.LogLn("\tN = 3400 KHz");
  pmLogging.LogLn("\tO = 5000 KHz");
  pmLogging.LogLn("\tP = 100 400 1000 3400 5000 KHz (standards)");
  pmLogging.LogLn("\n\tr = reset to startup defaults.");
  pmLogging.LogLn("\t? = help - this page");
  pmLogging.LogLn();
}

void I2CScanner::I2Cscan()
{
  startScan = millis();
  uint8_t count = 0;

  if (disableIRQ) noInterrupts();

  if (header)
  {
    pmLogging.Log("TIME\tDEC\tHEX\t");
    for (uint8_t s = 0; s < speeds; s++)
    {
      pmLogging.Log("\t");
      pmLogging.Log(String(speed[s]));
    }
    pmLogging.LogLn("\t[KHz]");
    for (uint8_t s = 0; s < speeds + 5; s++)
    {
      pmLogging.Log("--------");
    }
    pmLogging.LogLn();
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
      pmLogging.Log(String(millis()));
      pmLogging.Log("\t");
      pmLogging.Log(String(address));
      pmLogging.Log("\t0x");
      if (address < 0x10) pmLogging.Log("0");

      char hexadecimalnum [3];
      sprintf(hexadecimalnum, "%X", address);

      pmLogging.Log(hexadecimalnum);
      pmLogging.Log("\t");

      for (uint8_t s = 0; s < speeds ; s++)
      {
        pmLogging.Log("\t");
        pmLogging.Log(found[s] ? "V" : ".");
      }
      pmLogging.LogLn();
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
    pmLogging.LogLn();
    pmLogging.Log(String(count));
    pmLogging.Log(" devices found in ");
    pmLogging.Log(String(stopScan - startScan));
    pmLogging.LogLn(" milliseconds.");
  }

  interrupts();
}


// -- END OF FILE --
