#include <TM1637Display.h>
#include "RotaryEncoder.h"

#include <Adafruit_NeoPixel.h>

// define the connections pins
#define CLK 7
#define DIO 8

#define PIN_WS2812B 0  // Arduino pin that connects to WS2812B
#define NUM_PIXELS 10   // The number of LEDs (pixels) on WS2812B

#define DELAY_INTERVAL 250  // 250ms pause between each pixel

Adafruit_NeoPixel WS2812B(NUM_PIXELS, PIN_WS2812B, NEO_GRB + NEO_KHZ800);

int Counter = 117, LastCount = 0;  //uneeded just for test
void RotaryChanged();              //we need to declare the func above so Rotary goes to the one below

int led_three = 2;
int current_state = 0;
int previous_state = 0;

RotaryEncoder Rotary(&RotaryChanged, 4, 3, 6);  // Pins 2 (DT), 3 (CLK), 4 (SW)



// create a display object of type TM1637Display
TM1637Display display = TM1637Display(CLK, DIO);

// an array that sets individual segments per digit to display the word "dOnE"
const uint8_t done[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,          // d
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // O
  SEG_C | SEG_E | SEG_G,                          // n
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G           // E
};

// degree celsius symbol
const uint8_t C[] = {
  SEG_A | SEG_D | SEG_E | SEG_F  // C
};

void RotaryChanged() {
  const unsigned int state = Rotary.GetState();

  if (state & DIR_CW)
    Counter++;

  if (state & DIR_CCW)
    Counter--;
}


void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  Rotary.setup();

  Serial.begin(9600);

  pinMode(led_three, OUTPUT);

  display.clear();
  display.setBrightness(5);  // set the brightness to 7 (0:dimmest, 7:brightest)

  Serial.println("Rotary Encoder Tests");

  WS2812B.begin();  // INITIALIZE WS2812B strip object (REQUIRED)
}

// the loop function runs over and over again forever
void loop() {

  if (Rotary.GetButtonDown()) {
    Serial.println("Yay button down!!!");
    if (current_state == 0) {
      digitalWrite(led_three, HIGH);
      current_state = 1;
      for (int pixel = 0; pixel < NUM_PIXELS; pixel++) {         // for each pixel
        WS2812B.setPixelColor(pixel, WS2812B.Color(0, 255, 0));  // it only takes effect if pixels.show() is called
        WS2812B.show();                                          // send the updated pixel colors to the WS2812B hardware.

        ;  // pause between each pixel
      }
    } else {
      digitalWrite(led_three, LOW);
      current_state = 0;
      WS2812B.clear();
      WS2812B.show();
    }
    delay(200);
  }

  if (LastCount != Counter) {
    Serial.println(Counter);
    LastCount = Counter;
    //display.clear();
    display.setSegments(C, 1, 0);
    display.showNumberDec(Counter, false, 3, 1);  // displayed _-9_
  }



  /*
  int i;
  for (i = 0; i < 10; i++) {
    display.showNumberDec(i);
    delay(500);
  display.clear();
  }
*/
}
