#include "Arduino.h"
#include "IC9700.h"

#define CIV_address 0xA2
#define controller_address 0xE0

#define ModeLSB   0
#define ModeUSB   1
#define ModeAM    2
#define ModeCW    3
#define ModeRTTY  4
#define ModeFM    5
#define ModeCWR   7
#define ModeRTTYR 8
#define ModeDV    17 

#define Filter1   1
#define Filter2   2
#define Filter3   3

byte CIV_On[] = {0x18, 0x01}; //tested - IC9700.powerOn()
byte CIV_Off[] = {0x18, 0x00}; //tested - IC9700.powerOff()
byte CIV_Vol[] = {0x14, 0x01};  //tested get and set - IC9700.getVolume() & IC9700.setVolume(0-255)
byte CIV_setFreq[] = {0x05};  //tested - IC9700.setFreq(xxxxxxxxxx) x in hz
byte CIV_getFreq[] = {0x03};  //tested - IC9700.getFreq()
byte CIV_getFreqTrans[] = {0x00}; //IC9700 frequency data 'transeive' mode
byte CIV_getMode[] = {0x04}; //Get operating Mode & filter
byte CIV_setMode[] = {0x06}; //FM, CW, USB set tested - Set operating Mode & filter 
byte CIV_Att[] = {0x11}; //get and set Attenuator 0000-0255
byte CIV_RFG[] = {0x14, 0x02}; //tested set - IC9700.setRFG(xxx) get and set RF Gain 0000-0255
byte CIV_Squelch[] = {0x14, 0x03}; //tested set - IC9700.setSquelch(xxx) get and set Squelch 000-255
byte CIV_NRLevel[] = {0x14, 0x06}; // get and set NR Level 0000-0255
byte CIV_PBT1[] = {0x14, 0x07}; // get and send PBT1 position 0000=CCW, 0128=center, 0255=CW
byte CIV_PBT2[] = {0x14, 0x08};  // get and send PBT2 position 0000=CCW, 0128=center, 0255=CW
byte CIV_RFPower[] = {0x14, 0x0A}; //get and send RF Power 0000-0255
byte CIV_NBLevel[] = {0x14, 0x12}; //get and send NB Level 0000-0255
byte CIV_SMeter[] = {0x15, 0x02}; //read S-Meter level 0000-0255
byte CIV_SWR[] = {0x15, 0x12}; // read SWR, 0000=1.0, 0048=1.5, 0080=2.0, 0120=3.0
byte CIV_NB[] = {0x16, 0x22}; // 00=off, 01=on
byte CIV_NR[] = {0x16, 0x40}; // 00=off, 01=on
byte CIV_TX[] = {0x1C, 0x00};  // 00=RX, 01=TX
byte CIV_RITFreq[] = {0x21, 0x00};  //Set RIT 
byte CIV_RIT[] = {0x21, 0x01}; //RIT 00=OFF, 01=ON

int tempRcvd[25];
int rcvdCounter = 0;

IC9700:: IC9700(int baudRate, int serialPort)
{
  switch (serialPort)
  {
    case 0:
      //Serial.begin(baudRate);
      break;
    case 1:
      Serial1.begin(baudRate);
      break;
    case 2:
      Serial2.begin(baudRate);
      break;
    case 3:
      Serial3.begin(baudRate);
      break;    
  }
  _baudRate = baudRate;
  _serialPort = serialPort;
}

void IC9700::powerOn(){sendCommand(CIV_On, sizeof(CIV_On), 0);}
void IC9700::powerOff(){sendCommand(CIV_Off, sizeof(CIV_Off), 0);}
void IC9700::getVolume(){sendCommand(CIV_Vol, sizeof(CIV_Vol), -1);}
void IC9700::getFreq(){sendCommand(CIV_getFreq, sizeof(CIV_getFreq), 0);}
void IC9700::setFreq(int tempFreq){sendCommand(CIV_setFreq, sizeof(CIV_setFreq), tempFreq);}
void IC9700::rcvdData(byte rcvdChar)
{
  //Serial.println(rcvdChar);
  tempRcvd[rcvdCounter] = rcvdChar;
  if(rcvdChar == 0xFD)
  {
    rcvdCounter = 0;
    parseReceived();
  }
  else
  {
    rcvdCounter++;
  }
}

void IC9700::getSMeter(){sendCommand(CIV_SMeter, sizeof(CIV_SMeter), -1);}
void IC9700::setVolume(int tempVolume){sendCommand(CIV_Vol, sizeof(CIV_Vol), tempVolume);}
void IC9700::setRFG(int tempRFG){sendCommand(CIV_RFG, sizeof(CIV_RFG), tempRFG);}
void IC9700::setSquelch(int tempSquelch){sendCommand(CIV_Squelch, sizeof(CIV_Squelch), tempSquelch);}

void IC9700::getMode(){sendCommand(CIV_getMode, sizeof(CIV_getMode),-1);}
void IC9700::setMode(byte mode){sendCommand(CIV_setMode, sizeof(CIV_setMode), mode);}

void IC9700::parseReceived()
{
  if(tempRcvd[2] == controller_address && tempRcvd[3] == CIV_address) // correct radio talking to controller
  { 
    if(tempRcvd[4] == CIV_Vol[0] && tempRcvd[5] == CIV_Vol[1]) //Decode received volume Data
    {
      currentVolume = ((BCDToDec(tempRcvd[6])*100) + BCDToDec(tempRcvd[7]));
    }
    else if(tempRcvd[4] == CIV_getFreq[0])
    {
      currentFrequency = BCDToDec(tempRcvd[5]) + (BCDToDec(tempRcvd[6])*100) + (BCDToDec(tempRcvd[7])*10000) + (BCDToDec(tempRcvd[8])*1000000) + (BCDToDec(tempRcvd[9])*100000000);
    }
    else if((tempRcvd[4] == CIV_SMeter[0]) && (tempRcvd[5] == CIV_SMeter[1]))
    {
      currentSMeter = ((BCDToDec(tempRcvd[6])*100) + BCDToDec(tempRcvd[7]));
    }
    else if(tempRcvd[4] == CIV_getMode[0])
    {
      currentMode = parseMode(BCDToDec(tempRcvd[5]));
    }
  }
}

void IC9700::sendCommand(byte *tempCommand, int tempLen, int tempValue)
{ 
  if(tempCommand[0] == CIV_On[0] && tempCommand[1] == CIV_On[1]){for(int i = 0; i < (_baudRate/1000)+2; i++){Serial1.write(0xFE);}} //Power on extra characters
  
  Serial1.write(0xFE);Serial1.write(0xFE);Serial1.write(CIV_address); Serial1.write(controller_address); //Preamble

  Serial1.write(tempCommand, tempLen); //send command

  if(tempCommand[0] == CIV_setFreq[0]){Serial1.write(decToBCD(extractLastTwoDec(tempValue)));Serial1.write(decToBCD(extractLastTwoDec(tempValue/100)));Serial1.write(decToBCD(extractLastTwoDec(tempValue/10000)));Serial1.write(decToBCD(extractLastTwoDec(tempValue/1000000)));Serial1.write(decToBCD(extractLastTwoDec(tempValue/100000000)));}
  else if((tempCommand[0] == CIV_Vol[0]) && (tempCommand[1] == CIV_Vol[1]) && tempValue > -1)
  {
    Serial1.write(decToBCD(tempValue / 100));
    Serial1.write(decToBCD(extractLastTwoDec(tempValue)));
  }
  else if((tempCommand[0] == CIV_RFG[0]) && (tempCommand[1] == CIV_RFG[1]) && tempValue > -1)
  {
    Serial1.write(decToBCD(tempValue / 100));
    Serial1.write(decToBCD(extractLastTwoDec(tempValue)));
  }
  else if((tempCommand[0] == CIV_Squelch[0]) && (tempCommand[1] == CIV_Squelch[1]) && tempValue > -1)
  {
    Serial1.write(decToBCD(tempValue / 100));
    Serial1.write(decToBCD(extractLastTwoDec(tempValue)));
  }
  else if(tempCommand[0] == CIV_setMode[0])
  {
    Serial1.write(decToBCD(tempValue));
  }
  
  Serial1.write(0xFD); //end
}

String IC9700::parseMode(byte tempVal){switch(tempVal){case ModeLSB:return "LSB";break;case ModeUSB:return "USB";break;case ModeAM:return "AM";break;case ModeCW:return "CW";break;case ModeRTTY:return "RTTY";break;case ModeFM:return "FM";break;case ModeCWR:return "CW-R";break;case ModeRTTYR:return "RTTY-R";break;case ModeDV:return "DV";break;}}
int IC9700::BCDToDec(byte tempByte){return ((tempByte/16*10) + (tempByte%16));} //decode received value
int IC9700::decToBCD(byte tempByte){return ((tempByte/10*16) + (tempByte%10));} //encode transmit value
int IC9700::extractLastTwoDec(int tempLong){return (((tempLong / 10U) % 10) * 10) + ((tempLong / 1U) % 10);}
