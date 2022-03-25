#ifndef IC7300_h
#define IC7300_h

#include "Arduino.h"

class IC7300
{
  public:
    IC7300(int baudRate, int serialPort);

    void powerOn();
    void powerOff();
    void getVolume();
    void setVolume(int tempVolume);
    void getFreq();
    void setFreq(int tempFreq);

    void getMode();
    void setMode(byte mode);
  
    void getSMeter();
    
    void rcvdData(byte rcvdChar);

    int currentVolume = -1;
    int oldVolume = -1;

    int currentFrequency = -1;
    int oldFrequency = -1;
    int vfoMult = 1;    

    int currentRFG = -1;
    int oldRFG = -1;

    int currentSMeter;
    int oldSMeter;

    String currentMode = "";
    
  private:

  
    void sendCommand(byte *tempCommand, int tempLen, int tempValue);
    void parseReceived();

    int BCDToDec(byte tempByte);
    int decToBCD(byte tempByte);
    int extractLastTwoDec(int tempLong);
    int _serialPort;
    int _baudRate;

    String parseMode(byte tempVal);
    
}
;

#endif
