#include <Encoder.h>
#include "IC9700.h"
#include "IC7300.h"
#include "EasyNextionLibrary.h"

/* trigger0: 9700 Power
 * trigger1: 9700 VFO Mult
 * trigger17: 9700 Get Freq
 * trigger33: 9700 USB
 * trigger34: 9700 CW
 * trigger36: 9700 FM
 * 
 * trigger2: 7300 Power
 * trigger3: 7300 VFO Mult
 * trigger49: 7300 Get Freq
 * trigger46: 7300 USB
 * trigger48: 7300 FM
 * trigger47: 7300 CW
 * 
 * trigger4: 9700 Active
 * trigger5: 7300 Active
 * 
 * 63488=red
 * 33808=inactive color
 * 65535=active color
 * 6339=off color
 */

#define trx1TimerInterval 25
#define trx2TimerInterval 25
#define colorRed 63488
#define colorWhite 65535
#define colorBlack 6339
#define colorGrey 33808

#define volumePin9700 A0
#define rfgPin9700 A1
#define volumePin7300 A2
#define rfgPin7300 A3

#define trxVFOPinA 18
#define trxVFOPinB 19

IC9700 IC9700(19200, 1);
IC7300 IC7300(19200, 2);

int currentSMeterColor9700 = 0;
int currentSMeterColor7300 = 0;

int trx1CommandCounter = 3;
int trx2CommandCounter = 3;

unsigned long trx1TimerStart = millis();
unsigned long trx2TimerStart = millis();

byte temp[6];
byte tempCount = 0;

boolean IC9700Active = false;
boolean IC7300Active = false;
  
Encoder trxVFO(trxVFOPinA, trxVFOPinB);
EasyNex CIVDisplay(Serial3);

void setup()
{
  Serial.begin(11520);

  //Set Nextion display baud
  Serial3.begin(115200);
  Serial3.print("bauds=57600");
  Serial3.write(0xff);
  Serial3.write(0xff);
  Serial3.write(0xff);

  Serial3.end();
  //---------------------------------
  
  CIVDisplay.begin(57600);
}

void trigger0() //9700 Power button
{
  if(IC9700.currentFrequency != -1) //On turning off
  {
    IC9700.powerOff();
    delay(50);
    if(IC7300.currentFrequency != -1)
    {
      CIVDisplay.writeNum("trx2VFO.pco", colorWhite);
      IC7300Active = true;
    }
    IC9700Active = false;
    trx1CommandCounter = 3;
    IC9700.currentFrequency = -1;
    IC9700.currentMode = "";
    CIVDisplay.writeStr("btnTRX1Mode.txt", IC9700.currentMode);
    CIVDisplay.writeNum("trx1VFO.pco", colorBlack);
  }
  else //Off turning on
  {
    IC9700.powerOn();
    delay(50);
    if(IC7300.currentFrequency != -1)
    {
      CIVDisplay.writeNum("trx2VFO.pco", colorGrey);
      CIVDisplay.writeNum("trx1VFO.pco", colorWhite);
      IC7300Active = false;
    }
    IC9700Active = true;
    trx1CommandCounter = 3;
    IC9700.currentFrequency = -1;
    IC9700.currentMode = "";
  }
}

void trigger1() //9700 VFO Multi
{
  if(IC9700.currentFrequency != -1)
  {
    if(IC9700.vfoMult == 1){IC9700.vfoMult = 10;CIVDisplay.writeNum("trx1X10.val", 1);CIVDisplay.writeNum("trx1X100.val", 0);CIVDisplay.writeNum("trx1X1000.val", 0);}else if(IC9700.vfoMult == 10){IC9700.vfoMult = 100;CIVDisplay.writeNum("trx1X10.val", 0);CIVDisplay.writeNum("trx1X100.val", 1);CIVDisplay.writeNum("trx1X1000.val", 0);}else if(IC9700.vfoMult == 100){IC9700.vfoMult = 1000;CIVDisplay.writeNum("trx1X10.val", 0);CIVDisplay.writeNum("trx1X100.val", 0);CIVDisplay.writeNum("trx1X1000.val", 1);}else if(IC9700.vfoMult == 1000){IC9700.vfoMult = 1;CIVDisplay.writeNum("trx1X10.val", 0);CIVDisplay.writeNum("trx1X100.val", 0);CIVDisplay.writeNum("trx1X1000.val", 0);}
    IC9700Active = true;
    IC7300Active = false;

    CIVDisplay.writeNum("trx1VFO.pco", colorWhite);
    if(IC7300.currentFrequency != -1)
    {
      CIVDisplay.writeNum("trx2VFO.pco", colorGrey);
    }
  }
}

void trigger2() //7300 Power
{
  if(IC7300.currentFrequency != -1) //On turning Off
  {
    IC7300.powerOff();
    delay(50);
    if(IC9700.currentFrequency != -1)
    {
      CIVDisplay.writeNum("trx1VFO.pco", colorWhite);
      IC9700Active = true;
    }
    IC7300Active = false;
    trx2CommandCounter = 3;
    IC7300.currentFrequency = -1;
    IC7300.currentMode = "";
    
    CIVDisplay.writeStr("btnTRX2Mode.txt", IC7300.currentMode);
    CIVDisplay.writeNum("trx2VFO.pco", colorBlack);
  }
  else //Off turning on
  {
    IC7300.powerOn();
    delay(50);
    if(IC9700.currentFrequency != -1)
    {
      CIVDisplay.writeNum("trx2VFO.pco", colorWhite);
      CIVDisplay.writeNum("trx1VFO.pco", colorGrey);
      IC9700Active = false;
    }
    IC7300Active = true;
    trx2CommandCounter = 3;
    IC7300.currentFrequency = -1;
    IC7300.currentMode = "";
  }
}

void trigger3() //7300 VFO Mult
{
  if(IC7300.currentFrequency != -1)
  {
    if(IC7300.vfoMult == 1){IC7300.vfoMult = 10;CIVDisplay.writeNum("trx2X10.val", 1);CIVDisplay.writeNum("trx2X100.val", 0);CIVDisplay.writeNum("trx2X1000.val", 0);}else if(IC7300.vfoMult == 10){IC7300.vfoMult = 100;CIVDisplay.writeNum("trx2X10.val", 0);CIVDisplay.writeNum("trx2X100.val", 1);CIVDisplay.writeNum("trx2X1000.val", 0);}else if(IC7300.vfoMult == 100){IC7300.vfoMult = 1000;CIVDisplay.writeNum("trx2X10.val", 0);CIVDisplay.writeNum("trx2X100.val", 0);CIVDisplay.writeNum("trx2X1000.val", 1);}else if(IC7300.vfoMult == 1000){IC7300.vfoMult = 1;CIVDisplay.writeNum("trx2X10.val", 0);CIVDisplay.writeNum("trx2X100.val", 0);CIVDisplay.writeNum("trx2X1000.val", 0);}
  
    IC9700Active = false;
    IC7300Active = true;

    CIVDisplay.writeNum("trx2VFO.pco", colorWhite);
  
    if(IC9700.currentFrequency != -1)
    {
      CIVDisplay.writeNum("trx1VFO.pco", colorGrey);
    }
  }
}

void trigger4() //9700 Active
{
  if(IC9700.currentFrequency != -1)
  {
    IC9700Active = true;
    IC7300Active = false;

    CIVDisplay.writeNum("trx1VFO.pco", colorWhite);
    
    if(IC7300.currentFrequency != -1)
    {
      CIVDisplay.writeNum("trx2VFO.pco", colorGrey);
    }
    else
    {
      CIVDisplay.writeNum("trx2VFO.pco", colorBlack);
    }
  }
}

void trigger5() //7300 Active
{
  if(IC7300.currentFrequency != -1)
  {
    IC9700Active = false;
    IC7300Active = true;

    CIVDisplay.writeNum("trx2VFO.pco", colorWhite);
    if(IC9700.currentFrequency != -1)
    {
      CIVDisplay.writeNum("trx1VFO.pco", colorGrey);
    }
    else
    {
      CIVDisplay.writeNum("tr1VFO.pco", colorBlack);
    }
  }
}

void trigger17(){IC9700.currentFrequency = CIVDisplay.readNumber("trx1VFO.val");CIVDisplay.writeStr("btnTRX1Mode.txt", IC9700.currentMode);}
void trigger33(){IC9700.setMode(1);CIVDisplay.writeStr("btnTRX1Mode.txt", "USB");} //USB
void trigger34(){IC9700.setMode(3);CIVDisplay.writeStr("btnTRX1Mode.txt", "CW");} //CW
void trigger36(){IC9700.setMode(5);CIVDisplay.writeStr("btnTRX1Mode.txt", "FM");} //FM

void trigger49(){IC7300.currentFrequency = CIVDisplay.readNumber("trx2VFO.val");CIVDisplay.writeStr("btnTRX2Mode.txt", IC7300.currentMode);}
void trigger46(){IC7300.setMode(1);CIVDisplay.writeStr("btnTRX2Mode.txt", "USB");} //USB
void trigger47(){IC7300.setMode(3);CIVDisplay.writeStr("btnTRX2Mode.txt", "CW");} //CW
void trigger48(){IC7300.setMode(5);CIVDisplay.writeStr("btnTRX2Mode.txt", "FM");} //FM

void loop()
{
  CIVDisplay.NextionListen();

  while(Serial1.available()){IC9700.rcvdData(Serial1.read());}
  while(Serial2.available()){IC7300.rcvdData(Serial2.read());}


  if(millis() - trx1TimerStart >= trx1TimerInterval){switch(trx1CommandCounter){case 1:IC9700.getSMeter();break;
    case 2:
    
      if(IC9700Active)
      {
        IC9700.currentFrequency += trxVFO.read() * IC9700.vfoMult;
        trxVFO.write(0);
      }
      IC9700.currentVolume = analogRead(volumePin9700)/4.011;
      IC9700.currentRFG = analogRead(rfgPin9700)/1.95;
      IC9700.currentSquelch = map(IC9700.currentRFG,269,524,0,255);
      
      if(IC9700.currentSquelch <= 0)
      {
        IC9700.currentSquelch = 0;
      }
      
      if(IC9700.currentRFG >= 255)
      {
        IC9700.currentRFG = 255;
      }
      
      trx1CommandCounter = 0;
      break;
    case 3:IC9700.getFreq();if(IC9700.currentFrequency != -1){CIVDisplay.writeNum("trx1VFO.val", IC9700.currentFrequency);trx1CommandCounter = 4;CIVDisplay.writeNum("trx1VFO.pco", colorWhite);}trx1CommandCounter--;break;case 4:IC9700.getMode();if(IC9700.currentMode != ""){CIVDisplay.writeStr("btnTRX1Mode.txt", IC9700.currentMode);trx1CommandCounter = 0;}trx1CommandCounter--;break;}trx1CommandCounter++;trx1TimerStart = millis();} 
  
  if(millis() - trx2TimerStart >= trx2TimerInterval){switch(trx2CommandCounter){case 1:IC7300.getSMeter();break;
    case 2:
    
      if(IC7300Active)
      {
        IC7300.currentFrequency += trxVFO.read() * IC7300.vfoMult;
        trxVFO.write(0);
      }
      IC7300.currentVolume = analogRead(volumePin7300)/4.011;
      IC7300.currentRFG = analogRead(rfgPin7300)/4.011;
      
      trx2CommandCounter = 0;
      break;
    case 3:IC7300.getFreq();if(IC7300.currentFrequency != -1){CIVDisplay.writeNum("trx2VFO.val", IC7300.currentFrequency);trx2CommandCounter = 4;CIVDisplay.writeNum("trx2VFO.pco", colorWhite);}trx2CommandCounter--;break;case 4:IC7300.getMode();if(IC7300.currentMode != ""){CIVDisplay.writeStr("btnTRX2Mode.txt", IC7300.currentMode);trx2CommandCounter = 0;}trx2CommandCounter--;break;}trx2CommandCounter++;trx2TimerStart = millis();} 
  
  
  if(IC9700.oldFrequency != IC9700.currentFrequency && IC9700.currentFrequency != -1)
  {
      CIVDisplay.writeNum("trx1VFO.val", IC9700.currentFrequency);
      IC9700.setFreq(IC9700.currentFrequency);
      IC9700.oldFrequency = IC9700.currentFrequency;
  }
  
  if(IC7300.oldFrequency != IC7300.currentFrequency && IC7300.currentFrequency != -1)
  {
    CIVDisplay.writeNum("trx2VFO.val", IC7300.currentFrequency);
    IC7300.setFreq(IC7300.currentFrequency);
    IC7300.oldFrequency = IC7300.currentFrequency;
  }

  if(IC9700.currentRFG != IC9700.oldRFG)
  {    
    IC9700.setRFG(IC9700.currentRFG);
    IC9700.oldRFG = IC9700.currentRFG;
  }

  if(IC9700.currentSquelch != IC9700.oldSquelch)
  {
    IC9700.setSquelch(IC9700.currentSquelch);
    IC9700.oldSquelch = IC9700.currentSquelch;
  }
  
  if(IC9700.currentVolume != IC9700.oldVolume){IC9700.setVolume(IC9700.currentVolume);IC9700.oldVolume = IC9700.currentVolume;}
  if(IC7300.currentVolume != IC7300.oldVolume){IC7300.setVolume(IC7300.currentVolume);IC7300.oldVolume = IC7300.currentVolume;}
  if(IC9700.oldSMeter != IC9700.currentSMeter){if((IC9700.currentSMeter < 121) && (currentSMeterColor9700 != colorWhite)){CIVDisplay.writeNum("trx1Bar.pco", colorWhite);currentSMeterColor9700 = colorWhite;}else if((IC9700.currentSMeter >= 121) && (currentSMeterColor9700 != colorRed)){CIVDisplay.writeNum("trx1Bar.pco", colorRed);currentSMeterColor9700 = colorRed;}CIVDisplay.writeNum("trx1Bar.val", (IC9700.currentSMeter/2.41));IC9700.oldSMeter = IC9700.currentSMeter;}
  if(IC7300.oldSMeter != IC7300.currentSMeter){if((IC7300.currentSMeter < 121) && (currentSMeterColor7300 != colorWhite)){CIVDisplay.writeNum("trx2Bar.pco", colorWhite);currentSMeterColor7300 = colorWhite;}else if((IC7300.currentSMeter >= 121) && (currentSMeterColor7300 != colorRed)){CIVDisplay.writeNum("trx2Bar.pco", colorRed);currentSMeterColor7300 = colorRed;}CIVDisplay.writeNum("trx2Bar.val", (IC7300.currentSMeter/2.41));IC7300.oldSMeter = IC7300.currentSMeter;}

}
