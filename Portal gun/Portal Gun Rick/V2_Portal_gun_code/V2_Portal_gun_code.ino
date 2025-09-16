#include <Arduino.h>
#include <TM1637Display.h>
#include <ezButton.h>
#include <Adafruit_NeoPixel.h>

// ==== TM1637 DISPLAY PINS ====
#define CLK 9
#define DIO 10
TM1637Display display(CLK, DIO);

// ==== ROTARY ENCODER PINS ====
#define CLK_PIN 6
#define DT_PIN 7
#define SW_PIN 8

#define DIRECTION_CW 0
#define DIRECTION_CCW 1

int counter = 137;
int direction = DIRECTION_CW;
int CLK_state;
int prev_CLK_state;

ezButton button(SW_PIN);

// ==== LED STRIP ====
#define PIN_WS2812B 2
#define NUM_PIXELS 10
Adafruit_NeoPixel ws2812b(NUM_PIXELS, PIN_WS2812B, NEO_GRB + NEO_KHZ800);

int ledBrightness = 255;  // Start brightness
int targetBrightness = 255;
unsigned long pressStart = 0;
bool isLongPressing = false;

// Fading control
unsigned long lastFadeUpdate = 0;
const int fadeInterval = 10;  // ms between brightness steps


unsigned long lastButtonTrigger = 0;  // timestamp of last valid button action
const unsigned long buttonCooldown = 1500;  // 1000 ms cooldown

// Custom segments for 'C'
const uint8_t CHAR_C = SEG_A | SEG_F | SEG_E | SEG_D;

// ===== FUNCTIONS =====
void showCounterWithC(int value) {
  uint8_t data[4];
  data[0] = CHAR_C;
  int absVal = abs(value);
  data[3] = display.encodeDigit(absVal % 10);
  data[2] = display.encodeDigit((absVal / 10) % 10);
  data[1] = display.encodeDigit((absVal / 100) % 10);
  display.setSegments(data);
}

void setAllGreen(int brightness) {
  int g = constrain(brightness, 0, 255);
  for (int i = 0; i < NUM_PIXELS; i++) {
    ws2812b.setPixelColor(i, ws2812b.Color(0, g, 0));
  }
  ws2812b.show();
}

// Non-blocking gradual fade for LEDs and display
void handleFade() {
  if (millis() - lastFadeUpdate >= fadeInterval) {
    lastFadeUpdate = millis();

    // Fade LED brightness
    if (ledBrightness < targetBrightness) {
      ledBrightness += 4;
      if (ledBrightness > 255) ledBrightness = 255;
      setAllGreen(ledBrightness);
    } else if (ledBrightness > targetBrightness) {
      ledBrightness -= 4;
      if (ledBrightness < 0) ledBrightness = 0;
      setAllGreen(ledBrightness);
    }

    // Map ledBrightness (0-255) to display brightness (0-7)
    int mappedDisplayBrightness = map(ledBrightness, 0, 255, 0, 7);
    display.setBrightness(mappedDisplayBrightness);

    // Turn off display if brightness is zero, else show number
    if (ledBrightness == 0) {
      display.clear();
    } else {
      showCounterWithC(counter);
    }
  }
}

// ===== SETUP =====
void setup() {
  Serial.begin(9600);
  display.setBrightness(0x0f);
  showCounterWithC(counter);

  pinMode(CLK_PIN, INPUT);
  pinMode(DT_PIN, INPUT);
  button.setDebounceTime(50);

  prev_CLK_state = digitalRead(CLK_PIN);

  ws2812b.begin();
  setAllGreen(ledBrightness);
}

// ===== LOOP =====
void loop() {
  button.loop();

  // ==== ROTARY ENCODER ====
  CLK_state = digitalRead(CLK_PIN);
  if (CLK_state != prev_CLK_state && CLK_state == HIGH) {
    if (digitalRead(DT_PIN) == HIGH) {
      counter++;
      direction = DIRECTION_CW;
    } else {
      counter--;
      direction = DIRECTION_CCW;
    }
    showCounterWithC(counter);
  }
  prev_CLK_state = CLK_state;

  // ==== BUTTON HANDLING ====
  if (button.isPressed()) {
    pressStart = millis();
    isLongPressing = true;
  }

  if (button.isReleased()) {
    unsigned long currentTime = millis();
    unsigned long pressDuration = currentTime - pressStart;

    // Only allow trigger if cooldown has passed
    if (pressDuration >= 300 && (currentTime - lastButtonTrigger >= buttonCooldown)) {
        if (ledBrightness < 50 ) {
            targetBrightness = 255;  // fade up to max
        } else if(ledBrightness > 200) {
            targetBrightness = 0;    // fade down to 0
        }

        lastButtonTrigger = currentTime;  // update last trigger time
    }
}


  // While holding long press → ramp toward max
  if (isLongPressing && button.getState() == LOW) {
    if (ledBrightness < 255) {
      targetBrightness = 255;
    }
  }

  // Handle fade non-blocking
  handleFade();
}
