	#ifndef TM1637_h
#define TM1637_h
#include <inttypes.h>
#include <Arduino.h>
  //************definitions for TM1637*********************
#define ADDR_AUTO  0x40
#define ADDR_FIXED 0x44
  
#define STARTADDR  0xc0 
  /**** definitions for the clock point of the 4-Digit Display *******/
#define POINT_ON   1
#define POINT_OFF  0
  /**************definitions for brightness***********************/
#define  BRIGHT_DARKEST 0
#define  BRIGHT_TYPICAL 2
#define  BRIGHTEST      7
//--------------------------------------------------------//
//Special characters index of tube table
#define INDEX_NEGATIVE_SIGN 16
#define INDEX_BLANK     17
/************definitions for type of the 4-Digit Display*********************/
#define D4036B 0
#define D4056A 1

  class TM1637
  {
    public:
    uint8_t Cmd_SetData;
    uint8_t Cmd_SetAddr;
    uint8_t Cmd_DispCtrl;
    boolean _PointFlag;   //_PointFlag=1:the clock point on
    uint8_t _DispType;
    TM1637(uint8_t, uint8_t);
    void init(uint8_t = D4036B);
    void writeByte(int8_t wr_data);//write 8bit data to tm1637
    void start(void);//send start bits
    void stop(void); //send stop bits
    void display(int8_t DispData[]);
    void display(uint8_t BitAddr,int8_t DispData);
    void display(double Decimal);
    void display(int16_t Decimal);
    void clearDisplay(void);
    void set(uint8_t = BRIGHT_TYPICAL,uint8_t = 0x40,uint8_t = 0xc0);//To take effect the next time it displays.
    void point(boolean PointFlag);//whether to light the clock point ":".To take effect the next time it displays.
    void coding(int8_t DispData[]); 
    int8_t coding(int8_t DispData); 
    private:
    uint8_t Clkpin;
    uint8_t Datapin;
    uint8_t DecPoint;
    boolean BlankingFlag;
  };
#endif

