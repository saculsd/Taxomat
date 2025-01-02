#include <Arduino.h>
#include <TM1637Display.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <string>
     
// Define the connections pins
#define DIO D3    
#define CLK D4  //7seg pins

#define BTN D5  //button Pin
#define SPK D6  //speaker pin
#define PIN_INPUT 0

#define interval 1000    //timer update interval 
#define Delay 1000      //button press Delay

TM1637Display display(CLK, DIO); // Create an instance of the TM1637Display

LiquidCrystal_I2C lcd(0x27, 16, 2); //init lcd display kind 16rows 2lanes 


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


  void startup(){

    for (int i=1; i<11; i++){
    if (i == 10){
      display.showNumberDecEx(0000, dot, true);
      break;
    }
    display.showNumberDecEx(i*1111, dot);
    delay(200);
    }

  }


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

  void playTone(int sound_height, int length){
    tone(SPK, sound_height);
    delay(length);
    noTone(SPK);
  }
  
};


class Display{
  protected:
    Timer* timer;
    Speaker speaker;

  public:
    Display() = default;
    Display(Timer* timer):timer(timer){
      if (this->timer == nullptr) {
        // Optional: Fehlerbehandlung, falls kein Timer übergeben wurde
        lcd.init();
        lcd.backlight();
        lcd.print("ERROR NULLPNTR");
        delay(2000);
      }
    }

  void startup(){
    std::string greeting = "te";
    int length = greeting.length();
    
    lcd.init();
    lcd.backlight();

    speaker.playTone(1000, 1000); //play 1Khz sound for 1s
    
    for(int i=0; i<length ; i++){          //!startup procedure to display greeting
      lcd.setCursor(( 7+length/2 )-i, 0); //to center always (7is middle)  
      
      for(int x=0; x<i+1; x++){
      lcd.print(greeting[x]);   //buchstaben hintereinander ausgeben
      }
      
      delay(300); //300ms between every letter
    }
    
    delay(2000);
    lcd.clear();
    lcd.print("7seg selftest...");
  }

  void showText(char* topText, int topRow, char* bottomText = "empty", int bottomRow = 0){
    lcd.setCursor(topRow, 0);
    lcd.printstr(topText);
    
    if (bottomText != "empty"){
      lcd.setCursor(bottomRow, 1);
      lcd.printstr(bottomText);
    }

    
  }

  void timerStart(){
    lcd.clear();
    lcd.print("Timer gestartet");
    //timer->reset();
    //timer->togglePause();

  }

};


class Menu{
private:
  Display display;

  bool short_press = false;
  bool long_press = false;
  bool during_press = false;

  char* topText;
  char* bottomText;
  int active_page;
  long lastPress = 0;  
  

public:

  Menu(){
    //init Menu on LCD
    this -> page1();
  }

void page1(){
  topText = "Taxomat starten?";
  bottomText = "<1>"; //12

  display.showText(topText, 0, bottomText, 13);
  active_page = 1;
}

void page2(){
  topText = " Helligkeit"; //1
  bottomText = " einstellen? <2>"; //0

  display.showText(topText, 1, bottomText, 0);
  active_page = 2;
}

void page3(){
  topText = " Waehrung"; //1
  bottomText = " einstellen? <3>"; //0

  display.showText(topText, 1, bottomText, 0);
  active_page = 3;
}

void nextPage(){
  lcd.clear();
  
  switch (active_page)
  {
  case 1:
    page2();
    break;
  
  case 2:
    page3();
    break;
  
  case 3:
    page1();
    break;
  }
}

void checkButton(bool press_detected){
  

  if (long_press && !during_press){


    if (millis() - lastPress >= Delay){
      long_press = false;
      lastPress = 0;
      return;
    }
    
    return;
  }


  if (press_detected && (millis() - lastPress) >= Delay){  //if button pressed(pause timer true) and button wasnt pressed in last Delay secs
    
    if (during_press){
      //long press detected
      if (millis() - lastPress >= 2000){ //dafür sorgen das nur einmal nach 2000ms ausgeführt wird
        Speaker().playTone(800, 300);

        switch (active_page)
        {
        case 1:
          display.timerStart(); 
          break;
        
        default:
          break;
        
        }


        during_press = false;
        lastPress = millis();
        
        return;
      }

      long_press = true;
      return;
    }
    
    
    
    during_press = true; //toggle during press from false to true and back
    
    lastPress = millis();
  }

  else if (!press_detected && during_press && !long_press){
    //short press detected
    this -> nextPage();

    during_press = false;
    lastPress = 0;

  }

}

};






Timer timer;    //Init Timer Class
Display dp(&timer); //init Diaply class timer pointer übergeben

void setup() { 
  pinMode(BTN, INPUT_PULLUP);
  pinMode(SPK, OUTPUT);

  display.setBrightness(3);  // Set the display brightness (0-7)
  display.clear();

  

  dp.startup();       //run LCD start sequence
  timer.startup();

};


void loop() { 
  static Menu menu; //init menu
  
  bool press_detected = false;   //reset every loop

  int buttonState = digitalRead(BTN);
  if (buttonState == HIGH){press_detected=true;} //check for button press and cahnge pause

  menu.checkButton(press_detected);
  
  if (millis() - counter >= interval){  //runs every interval ms
    counter += interval;
    
    timer.showTime();  //show time in 7seg display
    timer.update();   //add 1sec to timer

    Time currentTime = timer.getTime();
};
};


