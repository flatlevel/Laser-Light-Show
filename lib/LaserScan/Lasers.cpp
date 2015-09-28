/* LaserScanner code
Copyright (c) 2015 Skyworks Aerial Systems. All rights reserved.
*/

#include "Arduino.h"
#include "Lasers.h"
#include "DmxSimple.h"
#include "HardwareSerial.h"

Lasers::Lasers(){}

void Lasers::begin(int startShape){
  // Serial.begin(9600);
  // Serial.println();
  // Serial.println("Syntax:");
  // Serial.println(" 123c : use DMX channel 123");
  // Serial.println(" 45w  : set current channel to value 45");
  DmxSimple.usePin(laserPin);
  DmxSimple.write(1,0); //Turn on the laser

  DmxSimple.write(2,startShape); //Starting shape

  lBound=2; rBound=127; tBound=127; bBound=2;
  yPos = 60; invertH=false;
  xPos = 60; invertW=false;
  sV=0,sH=0,rV=50,rH=50;
  vdir=1,hdir=1,rotDir=0;

  Serial.println("Dmx ready");
}

void Lasers::setXPos(){DmxSimple.write(5,xPos);}
void Lasers::setYPos(){DmxSimple.write(4,yPos);}

void Lasers::scanVert(int scanV){
  switch (scanV) {
    case 1: //Scan mode 1: downscan
      vdir=1;
      if(yPos<tBound){ //Vertical Scan
        yPos=yPos+1;
        setYPos();
      }
      else {yPos=bBound;}
      break;
    case 2: //Scan mode 2: upscan
      vdir=0;
      if(yPos>bBound){ //Vertical Scan
        yPos=yPos-1;
        setYPos();
      }
      else {yPos=tBound;}
       break;
    case 3: //Scan mode 3: up and down
      if(yPos<tBound && vdir==1){ //Vertical Scan
        yPos=yPos+1;
        setYPos();
      }
      else if(yPos==tBound && vdir==1){vdir=0;}
      else if(yPos>bBound && vdir==0){ //Vertical Scan
        yPos=yPos-1;
        setYPos();
      }
      else if(yPos==bBound && vdir==0){vdir=1;}
      break;
    default:
      break;
  }
}

void Lasers::scanHorz(int scanH){
  switch (scanH) {
    case 1: //Scan mode 1: leftscan
      hdir=1;
      if(xPos<rBound){ //Vertical Scan
        xPos=xPos+1;
        setXPos();
      }
      else {xPos=lBound;}
      break;
    case 2: //Scan mode 2: right scan
      hdir=0;
      if(xPos>lBound){ //Vertical Scan
        xPos=xPos-1;
        setXPos();
      }
      else {xPos=rBound;}
       break;
    case 3: //Scan mode 3: up and down
      if(xPos<rBound && hdir==1){ //Vertical Scan
        xPos=xPos+1;
        setXPos();
      }
      else if(xPos==rBound && hdir==1){hdir=0;}
      else if(xPos>lBound && hdir==0){ //Vertical Scan
        xPos=xPos-1;
        setXPos();
      }
      else if(xPos==lBound && hdir==0){hdir=1;}
      break;
    default:
      break;
  }
}

void Lasers::scan() {
  scanVert(sV);
  delay(rV);
  scanHorz(sH);
  delay(rH);
}

void Lasers::invert(bool h, bool w){
  invertH=h; invertW=w;
  setScale();
}

void Lasers::setBounds(int rB, int lB, int tB, int bB){
  rBound=rB;
  lBound=lB;
  tBound=tB;
  bBound=bB;
}

void Lasers::scale(int size){
  if(size==0){
    DmxSimple.write(9,0);
  }
  else{
    int s=map(size,1,100,11,115);
    DmxSimple.write(9,s);
  }
}
void Lasers::setScale(){
  if (height==0){DmxSimple.write(7,height);}
  else{
    if(invertH==true){
      int h=map(height,1,100,76,150);
      DmxSimple.write(7,h);
    }
    else {
      int h=map(height,1,100,74,2);
      DmxSimple.write(7,h);
    }
  }
  if (width==0){DmxSimple.write(6,width);}
  else{
    if(invertW==true){
      int w=map(width,1,100,76,150);
      DmxSimple.write(6,w);
    }
    else{
      int w=map(width,1,100,74,2);
      DmxSimple.write(6,w);
    }
  }
}

void Lasers::setAngle(int theta){ //Angle of rotation in degrees
  int a=map(theta,0,360,0,180);
  DmxSimple.write(8,a);
}

void Lasers::rotate(int speed){ //
  if(speed==0){DmxSimple.write(8,speed);}
  else{
    if(rotDir==0){
      int s=map(speed,0,100,181,224);
      DmxSimple.write(8,s);
    }
    else if(rotDir==1){
      int s=map(speed,0,100,224,255);
      DmxSimple.write(8,s);
    }
  }
}

void Lasers::laserComs(char* cmd, uint8_t* pos){
  int c;
  // Serial.println("Updating Lasers");
  while (*pos < 64) { // Only completes this loop if exhausted serial buffer.
  // if(Serial.available()){
    // c = Serial.read();
    c = cmd[*pos];
    (*pos)++;
    if ((c>='0') && (c<='9')) {
      value = 10*value + c - '0';
      // Serial.write(c);
      }
    else {
      switch (c){
        case 'c':
          channel = value;
          break;
        case 'w':
          DmxSimple.write(channel, value);
          break;
        case 'v': //Scan mode: 0 no scan, 1 downscan, 2 upscan, 3 up and down
          sV = value;
          break;
        case 'h': //Scan mode: 0 no scan, 1 leftscan, 2 rightscan, 3 left and right
          sH = value;
          break;
        case 'y': //Vertical scan rate in ms
          rV = value;
          break;
        case 'x': //Horiz scan rate in ms
          rH = value;
          break;
        case 't': //Top bound
          tBound = value;
          break;
        case 'b': //Bottom bound
          bBound = value;
          break;
        case 'r': //Right bound
          rBound = value;
          break;
        case 'l': //left bound
          lBound = value;
          break;
        case 'a': //Angle from 0-360
          setAngle(value);
          break;
        case 'p': //Rotation Direction; 0=CW; 1=CCw;
          rotDir=value;
          break;
        case 'o': //Rotation speed
          rotate(value);
          break;
        case 'j': //Vertical scale
          height = value; setScale();
          break;
        case 'i': //Horizontal scale
          width = value; setScale();
          break;
        case 's': //Total scale
          //width = value; height = value;
          scale(value);
          break;
        case 'z': //Invert X and Y
          switch(value){
            case 3:
              invert(true,true);
            case 1:
              invert(false,true);
              break;
            case 2:
              invert(true,false);
              break;
            default:
              invert(false,false);
              break;
          }
          break;
        default:
          // Serial.write("Invalid value");
          break;
      }
      // Serial.print("Laser cmd: ");
      // Serial.println(c);
      // Serial.print("Value: ");
      // Serial.println(value);
      value = 0;
      return;
    }
  }
  // Serial.println("Done");
}
