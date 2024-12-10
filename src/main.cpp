#include <Arduino.h>
#include <TM1637Display.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
     
// Define the connections pins
#define DIO D3    
#define CLK D4  //7seg pins

#define BTN D5  //button Pin
#define SPK D6  //speaker pin

#define interval 100    //timer update interval 
#define Delay 1000      //button press Delay

TM1637Display display(CLK, DIO); // Create an instance of the TM1637Display

LiquidCrystal_I2C lcd(0x27, 16, 2); //init lcd display kind 16rows 2lanes 

Timer timer;

uint8_t dot = 0b01000000; // :
uint8_t letter_h[]{     
SEG_F | SEG_G | SEG_E | SEG_C | SEG_DP,  // h    https://github.com/tarantula3/TM1637-Digit-Display/blob/main/Datasheet.pdf
};

long int counter = 0;
struct Time //Time tuple to accses time out of timer
    {
      int seconds;
      int minutes;
      int hours;
    };



class Timer{
  protected:
    int seconds = 0;
    int minutes = 0;
    int hours = 0;
    bool pause = true;  //standart true to wait for start

  public:
  Timer(int seconds, int minutes, int hours):       //init variables
   seconds(seconds), minutes(minutes), hours(hours){};

  Timer() = default;


  Time getTime()const{return{seconds,minutes,hours};} //Time Getter for Time struct

  void showTime(){
    if (hours > 0){
      display.showNumberDecEx(hours, dot, false, 1, 0);   //hours left, only 1 num
      display.setSegments(letter_h, 1, 1);                //letter h behind + dots
      display.showNumberDecEx(minutes, dot, true, 2, 2);  //minutes
    }
    else if (minutes == 0 || seconds > 0){               //update secs while first minute and aslong minutes need no update
        display.showNumberDecEx(seconds, dot, true, 2, 2); // show only current secs
    }
    else if(seconds == 0){    //only update every minute
      display.showNumberDecEx(minutes, dot, true, 2, 0);   //minuten auf den ersten beiden
      display.showNumberDecEx(seconds, dot, true,  2, 2);   //sekunden auf den letzten beiden
    }
  }
  
  void reset(){   //reset Tiemr
    this->seconds = 0;
    this->minutes = 0;
    this->hours = 0;
    display.clear();
  }

  void update(){    //!when called, it updates 1sec of timer
    if (!pause){    //if active pause update of secs gets skipped
      if (++seconds == 60){ // nach 60sek +1min 60sek reset
            seconds = 0;
            
            if (++minutes == 60){ //after 1h switch to h display
              hours++;
              minutes = 0;
              display.clear();
            }
          }
    }
    
  }

  void togglePause(){
    pause = !pause; //toggle pause
  }  

};


class Speaker{
  protected:

  public:
  void playTone(){
  }
  
};


class Display{
  protected:
    Timer* timer;

  public:
    Display();

  void startup(){
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0,0);
    lcd.print("Test");
    delay(1000);
    timer.togglePause();


  }
};



void setup() {
  display.setBrightness(3);  // Set the display brightness (0-7)
  display.clear();
  pinMode(BTN, INPUT_PULLUP);


  Display dp;  //init Diaply class
  dp.startup();       //run LCD start sequence
};


void loop() { 

  static long lastPress = 0;  
  bool pause_timer = false;   //reset every loop

  int buttonState = digitalRead(BTN);
  if (buttonState == HIGH){pause_timer=true;} //check for button press and cahnge pause

  if (pause_timer && (millis() - lastPress) >= Delay){  //if button pressed(pause timer true) and button wasnt pressed in last Delay secs
      timer.togglePause();
      lastPress = millis();   //update last press time
    }

  
  if (millis() - counter >= interval){  //runs every interval ms
    counter += interval;
    
    timer.showTime();  //show time in lcd
    timer.update();   //add 1sec to timer

    Time currentTime = timer.getTime();
};
};