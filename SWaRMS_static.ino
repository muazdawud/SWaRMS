

#include <Arduino.h>
#include "Stepper.h"
#include "HX711.h"


#define stepSpeed 10U
#define stepPerRevolution 2048U

#define CP_Distance 5U
#define IG_Distance 3U
#define EG_Distance 3U

#define loadCellSck 22U
#define loadCellData 23U
#define tareWeight 2280.f
#define averageRead 3
#define W_Threshold 5000

#define Fan_Pin 12U

#define RedLED 25
#define BlueLED 33
#define YellowLED 32

#define CP_Button 35
#define EG_Button 34
#define IG_Button 21


const uint8_t CP_Pin[] = { 15, 2, 4, 16 };
const uint8_t EG_Pin[] = { 17, 5, 18, 19 };
const uint8_t IG_PIN[] = { 13, 14, 27, 26 };


Stepper IG_Stepper(stepPerRevolution, IG_PIN[0], IG_PIN[1], IG_PIN[2], IG_PIN[3]);
Stepper EG_Stepper(stepPerRevolution, EG_Pin[0], EG_Pin[1], EG_Pin[2], EG_Pin[3]);
Stepper CP_Stepper(stepPerRevolution, CP_Pin[0], CP_Pin[1], CP_Pin[2], CP_Pin[3]);


HX711 scale;


volatile bool timer_flag = false;
volatile uint32_t interrupt_counter = 0;
volatile uint32_t startup_timer_sync = 0;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
hw_timer_t *timer = NULL;


uint8_t UNIVERSAL_LOCK = 0;
uint8_t LTS = 0;

volatile uint8_t oldState = 0;
volatile uint8_t startup_flag = 0;
uint8_t stepping = 0;

uint32_t weight = 0;
volatile uint32_t count = 0;
volatile uint8_t test_weight = 0;
uint8_t weight_redundant_check = 0;

volatile uint8_t CP_State = 0;
volatile uint8_t IG_State = 0;
volatile uint8_t EG_State = 0;

uint8_t CP_Position = 0;
uint8_t IG_Position = 0;
uint8_t EG_Position = 0;

volatile uint8_t CP_Debounce = 0;
volatile uint8_t IG_Debounce = 0;
volatile uint8_t EG_Debounce = 0;

volatile uint8_t fan_Flag = 0;
volatile uint32_t fan_counter = 0;


void initScale();
void testStepper();
void interrupt_routine();
uint32_t handlePollScale();
void CP_Step();
void EG_Step();
void IG_Step();


void IRAM_ATTR onTimer() {

  portENTER_CRITICAL_ISR(&timerMux);
  interrupt_counter++;
  portEXIT_CRITICAL_ISR(&timerMux);

  if ((interrupt_counter - startup_timer_sync) >= 5) {

    if (startup_flag) {
      digitalWrite(RedLED, (oldState ^= 1));
      digitalWrite(BlueLED, (oldState));
      digitalWrite(YellowLED, (oldState));
    } else {
      portENTER_CRITICAL_ISR(&timerMux);
      timer_flag = true;
      portEXIT_CRITICAL_ISR(&timerMux);
    }

    portENTER_CRITICAL_ISR(&timerMux);
    startup_timer_sync = interrupt_counter;
    portEXIT_CRITICAL_ISR(&timerMux);
  }

  portENTER_CRITICAL_ISR(&timerMux);
  if ((interrupt_counter - fan_counter) > 60) {
    fan_counter = interrupt_counter;
    fan_Flag ^= 1;
  }
  portEXIT_CRITICAL_ISR(&timerMux);
}


void setup() {

  pinMode(RedLED, OUTPUT);
  pinMode(BlueLED, OUTPUT);
  pinMode(YellowLED, OUTPUT);

  pinMode(CP_Button, INPUT);
  pinMode(EG_Button, INPUT);
  pinMode(IG_Button, INPUT);

  pinMode(Fan_Pin, OUTPUT);

  timer = timerBegin(10000);
  timerAttachInterrupt(timer, &onTimer);
  timerAlarm(timer, 100, true, 0);

  startup_flag = 1;

  testStepper();

  initScale();
}


void loop() {

  if (timer_flag) {

    portENTER_CRITICAL(&timerMux);
    timer_flag = false;
    portEXIT_CRITICAL(&timerMux);

    if (!startup_flag) {
      portENTER_CRITICAL(&timerMux);
      interrupt_routine();
      portEXIT_CRITICAL(&timerMux);
    }
  }

  if (CP_State && !EG_State) {

    CP_Step();
  } else if (IG_State && !UNIVERSAL_LOCK && !(CP_State) && !(EG_State)) {

    IG_Step();

    if (LTS) {
      UNIVERSAL_LOCK = 1;
      LTS = 0;
    }
  } else if (EG_State && !(CP_State) && !(IG_State)) {

    EG_Step();
  }

  if (test_weight && scale.is_ready()) {

    weight = handlePollScale();

    if (weight >= ((W_Threshold / 10) * 7)) {
      digitalWrite(YellowLED, HIGH);
    } else {
      digitalWrite(YellowLED, LOW);
    }
  }

  if (weight >= ((W_Threshold / 10) * 5)) {
    digitalWrite(Fan_Pin, fan_Flag);
  }
}


void initScale() {

  scale.begin(loadCellData, loadCellSck);
  scale.set_scale(tareWeight);
  scale.tare();

  weight = handlePollScale();

  if (weight >= ((W_Threshold / 10) * 7)) {
    digitalWrite(YellowLED, HIGH);
  } else {
    digitalWrite(YellowLED, LOW);
  }
}


void testStepper() {

  IG_Stepper.setSpeed(stepSpeed);
  EG_Stepper.setSpeed(stepSpeed);
  CP_Stepper.setSpeed(stepSpeed);

  IG_Stepper.step(stepPerRevolution);
  IG_Stepper.step(-1 * stepPerRevolution);

  EG_Stepper.step(stepPerRevolution);
  EG_Stepper.step(-1 * stepPerRevolution);

  CP_Stepper.step(stepPerRevolution);
  CP_Stepper.step(-1 * stepPerRevolution);

  startup_flag = 0;

  digitalWrite(RedLED, LOW);
  digitalWrite(BlueLED, HIGH);
  digitalWrite(YellowLED, LOW);
}


void interrupt_routine() {

  stepping = (CP_State) | (EG_State) | (IG_State);

  if (!digitalRead(CP_Button) && !(stepping)) {
    if (CP_Debounce && (!EG_Debounce && !IG_Debounce)) {
      CP_State = 1;
      CP_Debounce = 0;
    } else {
      CP_Debounce = 1;
    }

  } else {
    CP_Debounce = 0;
  }

  if (!digitalRead(IG_Button) && !(stepping)) {
    if (IG_Debounce && (!EG_Debounce && !CP_Debounce)) {
      IG_State = 1;
      IG_Debounce = 0;
    } else {
      IG_Debounce = 1;
    }

  } else {
    IG_Debounce = 0;
  }

  if (!digitalRead(EG_Button) && !(stepping)) {
    if (EG_Debounce && (!IG_Debounce && !CP_Debounce)) {
      EG_State = 1;
      EG_Debounce = 0;
    } else {
      EG_Debounce = 1;
    }

  } else {
    EG_Debounce = 0;
  }

  if ((interrupt_counter - count) >= 100) {
    test_weight = 1;
    count = interrupt_counter;
  }

  if (weight >= W_Threshold) {
    if (++weight_redundant_check > 3) {
      if (!(IG_Position) && !(UNIVERSAL_LOCK)) {
        if (CP_Position) {

          CP_State = 1;
        }

        LTS = 1;
        IG_State = 1;
      }

      weight_redundant_check = 0;
    }
  } else {
    LTS = 0;
    UNIVERSAL_LOCK = 0;
    weight_redundant_check = 0;
  }

  stepping = 0;
}


uint32_t handlePollScale() {

  int32_t scaleAverage = 0;

  scaleAverage = scale.get_units();

  test_weight = 0;

  if (scaleAverage < 0) {
    return 0;
  }

  return scaleAverage;
}


void CP_Step() {

  oldState = 1;

  CP_State = 0;
  CP_Position ^= 1;

  int8_t steps = (CP_Position) ? -1 : 1;

  for (uint8_t i = 0; i < CP_Distance; i++) {
    for (uint16_t k = 0; k < stepPerRevolution; k++) {
      CP_Stepper.step(steps);
    }
    digitalWrite(BlueLED, (oldState ^= 1));
  }

  digitalWrite(BlueLED, HIGH);
  oldState = 0;
}


void IG_Step() {

  oldState = 1;

  IG_State = 0;
  IG_Position ^= 1;

  int8_t steps = (IG_Position) ? -1 : 1;

  for (uint8_t i = 0; i < IG_Distance; i++) {
    for (uint16_t k = 0; k < stepPerRevolution; k++) {
      IG_Stepper.step(steps);
    }
    digitalWrite(BlueLED, (oldState ^= 1));
  }

  digitalWrite(BlueLED, HIGH);
  oldState = 0;
}


void EG_Step() {

  oldState = 1;

  EG_State = 0;
  EG_Position ^= 1;

  int8_t steps = (EG_Position) ? -1 : 1;

  for (uint8_t i = 0; i < EG_Distance; i++) {
    for (uint16_t k = 0; k < stepPerRevolution; k++) {
      EG_Stepper.step(steps);
    }
    digitalWrite(BlueLED, (oldState ^= 1));
  }

  digitalWrite(BlueLED, HIGH);
  oldState = 0;
}