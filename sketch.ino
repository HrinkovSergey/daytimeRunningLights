#include <FastLED.h>

#define NUM_LEDS 21
#define RING_NUM_LEDS 16
#define DATA_PIN 5
#define RING_PIN 16
#define BRIGHTNESS  255
#define LED_BRIGHTNESS 255
#define RING_BRIGHTNESS 255
#define COLOR_FADE_SPEED 20
#define DEFAULT_COLOR CRGB::Blue
#define TURN_SIGNAL_COLOR CRGB::Red
#define RING_COLOR CRGB::Yellow
#define START_DXO_DELAY 30

CRGB leds[NUM_LEDS];
CRGB ring_leds[RING_NUM_LEDS];

boolean isSetWhite = true;
boolean buttonState = LOW;
boolean lastButtonState = LOW;
int mode = 0;

enum Modes {
  ORANGE,
  RED,
  BLUE,
  GREEN,
  SMOOTH_BLINK_RED,
  RAINBOW
};

// Variables to control non-blocking effects
unsigned long lastUpdate = 0;
unsigned long blinkInterval = 10;
unsigned long rainbowInterval = 20;
uint8_t hue = 0; // Used for the rainbow effect

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.addLeds<WS2812, RING_PIN, GRB>(ring_leds, RING_NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  pinMode(4, INPUT_PULLDOWN);
  pinMode(17, INPUT_PULLDOWN);
  startDXO();
  delay(1000);
}

void loop() {
  Check button for turn signal
  boolean button = digitalRead(4);
  if (button == 1) {
    turnSignal();
    isSetWhite = false;
  } else {
    if (!isSetWhite) {
      delay(500);
      setGraduallyColor(DEFAULT_COLOR, leds, NUM_LEDS, LED_BRIGHTNESS);
      isSetWhite = true;
    }
  }

  Check mode-switching button
  buttonState = digitalRead(17);
  if (buttonState == LOW && lastButtonState == HIGH) {
    mode = (mode + 1) % 6;  // Cycle through modes 0 to 5
    applyMode();
  }
  lastButtonState = buttonState;

}

void applyMode() {
  // Reset variables if necessary
  lastUpdate = millis();
  hue = 0;  // Reset hue for rainbow effect
  runCurrentModeEffect();
}

void runCurrentModeEffect() {
  Serial.println("Sensor Value: " + mode);
  Serial.println(mode);
  switch (mode) {
    case ORANGE:
      setGraduallyColor(CRGB::Orange, ring_leds, RING_NUM_LEDS, RING_BRIGHTNESS);
      break;
    case RED:
      setGraduallyColor(CRGB::Red, ring_leds, RING_NUM_LEDS, RING_BRIGHTNESS);
      break;
    case BLUE:
      setGraduallyColor(CRGB::Blue, ring_leds, RING_NUM_LEDS, RING_BRIGHTNESS);
      break;
    case GREEN:
      setGraduallyColor(CRGB::Green, ring_leds, RING_NUM_LEDS, RING_BRIGHTNESS);
      break;
    case SMOOTH_BLINK_RED:
      smoothBlinkNonBlocking(CRGB::Red);
      break;
    case RAINBOW:
      rainbowEffectNonBlocking();
      break;
  }
}

// -------------------------------------------------------

void startDXO() {
  for (int i = NUM_LEDS; i >= 0; i--) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = DEFAULT_COLOR;
    FastLED.show();
    delay(START_DXO_DELAY);
  }

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
    FastLED.show();
    delay(START_DXO_DELAY);
  }

  for (int i = NUM_LEDS; i >= 0; i--) {
    leds[i] = DEFAULT_COLOR;
    FastLED.show();
    delay(START_DXO_DELAY);
  }
  for (int i = NUM_LEDS; i >= 0; i--) {
    leds[i] = CRGB::Black;
    FastLED.show();
    delay(START_DXO_DELAY);
  }
  setGraduallyColor(DEFAULT_COLOR, leds, NUM_LEDS, LED_BRIGHTNESS);
  setGraduallyColor(RING_COLOR, ring_leds, RING_NUM_LEDS, RING_BRIGHTNESS);
}

void smoothBlinkNonBlocking(CRGB color) {
  unsigned long currentMillis = millis();
  if (currentMillis - lastUpdate >= blinkInterval) {
    static int brightness = 0;
    static int fadeDirection = 1;  // 1 for increasing brightness, -1 for decreasing

    setColor(color, ring_leds, RING_NUM_LEDS, brightness);
    FastLED.show();

    brightness += fadeDirection * 4;
    if (brightness >= 255 || brightness <= 0) {
      fadeDirection = -fadeDirection;  // Reverse direction
    }

    lastUpdate = currentMillis;
  }
}

void rainbowEffectNonBlocking() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastUpdate >= rainbowInterval) {
    for (int i = 0; i < RING_NUM_LEDS; i++) {
      ring_leds[i] = CHSV(hue + (i * 10), 255, 255);
    }
    FastLED.show();
    hue++;
    lastUpdate = currentMillis;
  }
}

void setGraduallyColor(CRGB color, CRGB* ledArray, int numLeds, int brightness) {
  for (int b = 0; b <= brightness; b += 4) {
    setColor(color, ledArray, numLeds, b);
    FastLED.show();
    delay(10);
  }
}

void setColor(CRGB color, CRGB* ledArray, int numLeds, int brightness) {
  CRGB scaledColor = color;
  scaledColor.nscale8_video(brightness);

  for (int i = 0; i < numLeds; i++) {
    ledArray[i] = scaledColor;
  }
}

void turnSignal() {
  int i = 0;
  while (i < (NUM_LEDS + 5)) {
    if (i < NUM_LEDS) leds[i] = TURN_SIGNAL_COLOR;
    if (i - 1 >= 0) leds[i - 1] = TURN_SIGNAL_COLOR;
    if (i - 2 >= 0) leds[i - 2] = TURN_SIGNAL_COLOR;
    if (i - 3 >= 0) leds[i - 3] = TURN_SIGNAL_COLOR;
    if (i - 4 >= 0) leds[i - 4] = TURN_SIGNAL_COLOR;
    if (i - 5 >= 0) leds[i - 5] = CRGB::Black;
    FastLED.show();
    delay(20);
    i++;
  }
}