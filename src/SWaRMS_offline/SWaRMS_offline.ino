


// #include <Stepper.h>
// #include "HX711.h"


// #define stepSpeed 8U
// #define stepPerRevolution 2048U

// #define compactionPlateDistance
// #define intakeGateDistance
// #define extractionGateDistance

// #define loadCellSck 22U
// #define loadCellData 23U
// #define tareWeight  //Scale Callibration Weight
// #define averageRead 5

// #define Fan_Pin 12U

// #define RedLED 25
// #define BlueLED 33
// #define YellowLED 32

// #define CP_Button 35
// #define EG_Button 34
// #define IG_Button 21

// #define CP_MOTOR_ID 1
// #define EG_MOTOR_ID 2
// #define IG_MOTOR_ID 3


// const uint8_t CP_Pin[] = { 15, 2, 4, 16 };
// const uint8_t EG_Pin[] = { 17, 5, 18, 19 };
// const uint8_t IG_PIN[] = { 13, 14, 27, 26 };
// // const uint8_t LED_Pin[] = {25, 33, 32};
// // const uint8_t BUT_Pin[] = {35, 34, 21};


// Stepper intakeStepper(stepPerRevolution, IG_PIN[0], IG_PIN[1], IG_PIN[2], IG_PIN[3]);
// Stepper extractionStepper(stepPerRevolution, EG_Pin[0], EG_Pin[1], EG_Pin[2], EG_Pin[3]);
// Stepper compactionStepper(stepPerRevolution, CP_Pin[0], CP_Pin[1], CP_Pin[2], CP_Pin[3]);


// HX711 scale;


// uint32_t weight = 0;
// uint32_t timer1 = 0;
// uint8_t iteration = 0;
// uint8_t direction_flag = 0;
// uint8_t CP_Button_State = 0;
// uint8_t EG_Button_State = 0;
// uint8_t IG_Button_State = 0;


// void setup() {

//   pinMode(RedLED, OUTPUT);
//   pinMode(BlueLED, OUTPUT);
//   pinMode(YellowLED, OUTPUT);

//   pinMode(CP_Button, INPUT);
//   pinMode(EG_Button, INPUT);
//   pinMode(IG_Button, INPUT);

//   for (uint8_t num = 0; i < 5; i++) {

//     digitalWrite(RedLED, HIGH);
//     delay(500);
//     digitalWrite(RedLED, LOW);
//     delay(500);
//   }

//   intakeStepper.setSpeed(stepSpeed);
//   extractionStepper.setSpeed(stepSpeed);
//   compactionStepper.setSpeed(stepSpeed);

//   initScale();

//   digitalWrite(BlueLED, HIGH);
// }


// void loop() {

//   CP_State_Check();
//   EG_State_Check();
//   IG_State_Check();

  

// }


// void initScale() {

//   scale.begin(loadCellData, loadCellSck);
//   scale.set_scale(tareWeight);

//   weight = handlePollScale();
// }


// void handleMotorStep(uint8_t id, uint8_t direction) {

//   int16_t steps = stepPerRevolution;

//   if(direction){
//     steps = -1*stepPerRevolution;
//   }

//   if (id == CP_MOTOR_ID) {
//     intakeStepper.step(steps);
//   } else if (id == EG_MOTOR_ID) {
//     extractionStepper.step(steps);
//   } else if (id == IG_MOTOR_ID) {
//     compactionStepper.step(steps);
//   }
// }


// uint32_t handlePollScale() {

//   uint32_t scaleAverage = 0;
//   uint8_t count = 0;

//   scale.power_up();
//   while (count < 10) {

//     if (scale.is_ready()) {
//       scaleAverage = scale.get_units(averageRead);
//       count = 10;
//     }
//     count++;
//   }
//   scale.power_down();

//   return scaleAverage;
// }


// void CP_State_Check(){

//   if(digitalRead(CP_Button) && !(CP_Button_State)){
//     if(!EG_Button && !IG_Button){
//       CP_Button_State = 1;
//       iteration = 0;
//     }
//   }

//   if(CP_Button_State){
    
//     if(direction_flag){
//       handleMotorStep(CP_MOTOR_ID, direction_flag);
//       if(!(--iteration)){
//         CP_Button_State = 0;
//         direction_flag = 0;
//       }
//     }
//     else{
//       handleMotorSetup(CP_MOTOR_ID, direction_flag);
//       if(++iteration == compactionPlateDistance){
//         direction_flag = 1;
//       }
//     }
//   }

// }


// void EG_State_Check(){

//   if(digitalRead(EG_Button) && !(EG_Button_State)){
//     if(!IG_Button && !CP_Button_State){
//       EG_Button_State = 1;
//       iteration = 0;
//     }
//   }

//   if(EG_Button_State){
    
//     if(direction_flag){
//       handleMotorStep(EG_MOTOR_ID, direction_flag);
//       if(!(--iteration)){
//         EG_Button_State = 0;
//         direction_flag = 0;
//       }
//     }
//     else{
//       handleMotorSetup(EG_MOTOR_ID, direction_flag);
//       if(++iteration == compactionPlateDistance){
//         direction_flag = 1;
//       }
//     }
//   }

// }


// void IG_State_Check(){

//   if(digitalRead(IG_Button) && !(IG_Button_State)){
//     if(!EG_Button_State && !CP_Button_State){
//       IG_Button_State = 1;
//       iteration = 0;
//     }
//   }

//   if(IG_Button_State){
    
//     if(IG_direction_flag){
//       handleMotorStep(IG_MOTOR_ID, IG_direction_flag);
//       if(!(--iteration)){
//         IG_Button_State = 0;
//         IG_direction_flag = 0;
//       }
//     }
//     else{
//       handleMotorSetup(IG_MOTOR_ID, IG_direction_flag);
//       if(++iteration == compactionPlateDistance){
//         IG_direction_flag = 1;
//       }
//     }
//   }

// }