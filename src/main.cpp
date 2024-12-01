#include <Arduino.h>
#include <TM1637Display.h>

// Define the connections pins
#define CLK D1
#define DIO D2
#define interval 100
TM1637Display display(CLK, DIO); // Create an instance of the TM1637Display

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
    bool pause = false;

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

  void Pause(){
    pause = !pause; //toggle pause
  }  

};


void setup() {
  display.setBrightness(3);  // Set the display brightness (0-7)
  display.clear();
  pinMode(D3, INPUT);
};


void loop() { 

  bool pause_timer = false;   //reset every loop
  static long lastPress = 0;  
  static int Delay = 1000;    //ms
  static Timer timer;         //init a timer

  
  int buttonState = digitalRead(D3);
  if (buttonState == LOW){pause_timer=true;} //check for button press and cahnge pause

  
  if (millis() - counter >= interval){  //runs every interval ms or pause is activated
    counter += interval;
    
    timer.showTime();  //show time in lcd
    timer.update();   //add 1sec to timer

    Time currentTime = timer.getTime();

    if (pause_timer && (millis() - lastPress) >= Delay){  //if button pressed(pause timer true) and button wasnt pressed in last Delay secs
      timer.Pause();
      lastPress = millis();   //update last press time
    }

};
}