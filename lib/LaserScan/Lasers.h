/* LaserScanner code
Copyright (c) 2015 Skyworks Aerial Systems. All rights reserved.
*/
#ifndef Lasers_h
#define Lasers_h

#include "HardwareSerial.h"
#include "Arduino.h"

class Lasers {
    public:
      Lasers();
      void begin(int);
      void scanVert(int);
      void scanHorz(int);
      void scan();
      void laserComs(char*, uint8_t*);

      void setBounds(int,int,int,int);
      void setYPos();
      void setXPos();
      void scale(int);
      void setScale();
      void setAngle(int);
      void invert(bool,bool);
      void rotate(int);

      int yPos,xPos, rotDir;
      bool invertH,invertW;
      int laserPin, fanPin;
    private:
      int value = 0,channel;
      int lBound,rBound,tBound,bBound;
      int sV,sH,rV,rH;
      int vdir,hdir,height,width;
};


#endif
