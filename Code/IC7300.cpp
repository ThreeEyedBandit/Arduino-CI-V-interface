#include "Arduino.h"
#include "IC7300.h"

#define CIV_7300_address 0x94
#define controller_address 0xE0

#define ModeLSB   0
#define ModeUSB   1
#define ModeAM    2
#define ModeCW    3
#define ModeRTTY  4
#define ModeFM    5
#define ModeCWR   7
#define ModeRTTYR 8

#define Filter1   1
#define Filter2   2
#define Filter3   3


byte CIV_7300_On[] = {0x18, 0x01}; //IC7300.powerOn()
byte CIV_7300_Off[] = {0x18, 0x00}; //IC7300.powerOff()
byte CIV_7300_Vol[] = {0x14, 0x01};  //IC7300.getVolume() & IC7300.setVolume(0-255)
byte CIV_7300_setFreq[] = {0x05};  //IC7300.setFreq(xxxxxxxxxx) x in hz
byte CIV_7300_getFreq[] = {0x03};  //IC7300.getFreq()
byte CIV_7300_getFreqTrans[] = {0x00}; //IC7300 frequency data 'transeive' mode
byte CIV_7300_getMode[] = {0x04}; //Get operating Mode & filter
byte CIV_7300_setMode[] = {0x06}; //Set operating Mode & filter 
//byte CIV_7300_Att[] = {0x11}; //get and set Attenuator 0000-0255
byte CIV_7300_RFGain[] = {0x14, 0x02}; //get and set RF Gain 0000-0255
byte CIV_7300_NRLevel[] = {0x14, 0x06}; // get and set NR Level 0000-0255
byte CIV_7300_PBT1[] = {0x14, 0x07}; // get and send PBT1 position 0000=CCW, 0128=center, 0255=CW
byte CIV_7300_PBT2[] = {0x14, 0x08};  // get and send PBT2 position 0000=CCW, 0128=center, 0255=CW
byte CIV_7300_RFPower[] = {0x14, 0x0A}; //get and send RF Power 0000-0255
byte CIV_7300_NBLevel[] = {0x14, 0x12}; //get and send NB Level 0000-0255
byte CIV_7300_SMeter[] = {0x15, 0x02}; //read S-Meter level 0000-0255
byte CIV_7300_SWR[] = {0x15, 0x12}; // read SWR, 0000=1.0, 0048=1.5, 0080=2.0, 0120=3.0
byte CIV_7300_NB[] = {0x16, 0x22}; // 00=off, 01=on
byte CIV_7300_NR[] = {0x16, 0x40}; // 00=off, 01=on
byte CIV_7300_TX[] = {0x1C, 0x00};  // 00=RX, 01=TX
byte CIV_7300_RITFreq[] = {0x21, 0x00};  //Set RIT 
byte CIV_7300_RIT[] = {0x21, 0x01}; //RIT 00=OFF, 01=ON

int temp7300Rcvd[25];
int rcvd7300Counter = 0;


IC7300:: IC7300(int baudRate, int serialPort)
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


void IC7300::powerOn(){sendCommand(CIV_7300_On, sizeof(CIV_7300_On), 0);}
void IC7300::powerOff(){sendCommand(CIV_7300_Off, sizeof(CIV_7300_Off), 0);}
void IC7300::getVolume(){sendCommand(CIV_7300_Vol, sizeof(CIV_7300_Vol), -1);}
void IC7300::getFreq(){sendCommand(CIV_7300_getFreq, sizeof(CIV_7300_getFreq), 0);}
void IC7300::setFreq(int tempFreq){sendCommand(CIV_7300_setFreq, sizeof(CIV_7300_setFreq), tempFreq);}
void IC7300::rcvdData(byte rcvdChar)
{
  //Serial.println(rcvdChar);
  temp7300Rcvd[rcvd7300Counter] = rcvdChar;
  if(rcvdChar == 0xFD)
  {
    rcvd7300Counter = 0;
    parseReceived();
  }
  else
  {
    rcvd7300Counter++;
  }
}

void IC7300::getSMeter(){sendCommand(CIV_7300_SMeter, sizeof(CIV_7300_SMeter), -1);}
void IC7300::setVolume(int tempVolume){sendCommand(CIV_7300_Vol, sizeof(CIV_7300_Vol), tempVolume);}

void IC7300::getMode()
{
  sendCommand(CIV_7300_getMode, sizeof(CIV_7300_getMode),-1);
}

void IC7300::setMode(byte mode)
{
  sendCommand(CIV_7300_setMode, sizeof(CIV_7300_setMode), mode);
}

void IC7300::parseReceived()
{
  if(temp7300Rcvd[2] == controller_address && temp7300Rcvd[3] == CIV_7300_address) // correct radio talking to controller
  { 
    if(temp7300Rcvd[4] == CIV_7300_Vol[0] && temp7300Rcvd[5] == CIV_7300_Vol[1]) //Decode received volume Data
    {
      currentVolume = ((BCDToDec(temp7300Rcvd[6])*100) + BCDToDec(temp7300Rcvd[7]));
    }
    else if(temp7300Rcvd[4] == CIV_7300_getFreq[0])
    {
      currentFrequency = BCDToDec(temp7300Rcvd[5]) + (BCDToDec(temp7300Rcvd[6])*100) + (BCDToDec(temp7300Rcvd[7])*10000) + (BCDToDec(temp7300Rcvd[8])*1000000) + (BCDToDec(temp7300Rcvd[9])*100000000);
    }
    else if((temp7300Rcvd[4] == CIV_7300_SMeter[0]) && (temp7300Rcvd[5] == CIV_7300_SMeter[1]))
    {
      currentSMeter = ((BCDToDec(temp7300Rcvd[6])*100) + BCDToDec(temp7300Rcvd[7]));
    }
    else if(temp7300Rcvd[4] == CIV_7300_getMode[0])
    {
      currentMode = parseMode(BCDToDec(temp7300Rcvd[5]));
    }
  }
}

void IC7300::sendCommand(byte *tempCommand, int tempLen, int tempValue)
{ 
  if(tempCommand[0] == CIV_7300_On[0] && tempCommand[1] == CIV_7300_On[1]){for(int i = 0; i < (_baudRate/1000)+2; i++){Serial2.write(0xFE);}} //Power on extra characters
  
  Serial2.write(0xFE);Serial2.write(0xFE);Serial2.write(CIV_7300_address); Serial2.write(controller_address); //Preamble

  Serial2.write(tempCommand, tempLen); //send command

  if(tempCommand[0] == CIV_7300_setFreq[0]){Serial2.write(decToBCD(extractLastTwoDec(tempValue)));Serial2.write(decToBCD(extractLastTwoDec(tempValue/100)));Serial2.write(decToBCD(extractLastTwoDec(tempValue/10000)));Serial2.write(decToBCD(extractLastTwoDec(tempValue/1000000)));Serial2.write(decToBCD(extractLastTwoDec(tempValue/100000000)));}
  else if((tempCommand[0] == CIV_7300_Vol[0]) && (tempCommand[1] == CIV_7300_Vol[1]) && tempValue > -1)
  {
    Serial2.write(decToBCD(tempValue / 100));
    Serial2.write(decToBCD(extractLastTwoDec(tempValue)));
  }
  else if(tempCommand[0] == CIV_7300_setMode[0])
  {
    Serial2.write(decToBCD(tempValue));
  }
  
  Serial2.write(0xFD); //end
}

String IC7300::parseMode(byte tempVal)
{
  switch(tempVal)
  {
    case ModeLSB:
      return "LSB";
      break;

    case ModeUSB:
      return "USB";
      break;

    case ModeAM:
      return "AM";
      break;

    case ModeCW:
      return "CW";
      break;

    case ModeRTTY:
      return "RTTY";
      break;

    case ModeFM:
      return "FM";
      break;

    case ModeCWR:
      return "CW-R";
      break;

    case ModeRTTYR:
      return "RTTY-R";
      break;
  }
}

int IC7300::BCDToDec(byte tempByte){return ((tempByte/16*10) + (tempByte%16));} //decode received value
int IC7300::decToBCD(byte tempByte){return ((tempByte/10*16) + (tempByte%10));} //encode transmit value
int IC7300::extractLastTwoDec(int tempLong){return (((tempLong / 10U) % 10) * 10) + ((tempLong / 1U) % 10);}
