/*
 * common.c
 *
 *  Created on: 2015�~4��5��
 *      Author: Hung
 */
#include "tgmath.h"
#include "app.h"
#include "driving.h"
#include "pid.h"
#include "common.h"
#include "encoder.h"
#include "linesensor.h"
#include "motor.h"
#include "servo.h"
//#include "ps4_usbhost.h"
#include "ds4.h"
#include "analog.h"
#include "airboard.h"
#include "distancesensor.h"

const int reloadPos = 7;
const int numberOfStage = 8;
int debug_display[10]={};

#if IS_MOTOR_0_2016 && IS_MOTOR_1_2016
	const float fastHatMultiplier_krpm;
	const float slowHatMultiplier_krpm;
	float hatMultiplier_krpm;
#else
	const int fastHatMultiplier = 40;
	const int slowHatMultiplier = 5;
	int hatMultiplier = 40;
#endif

const int HatDeadzone = 10;
const int upLimit = 5300;// in terms of SetPoint (Motor Board Max Limit 53XX)
const int lowLimit = -5300;
const int acceleration_limit = 60;//60;

//for ps4 button debounce
int old_r1_state,old_l1_state,old_dpad_state;
int old_circle_state, old_square_state,old_cross_state,old_triangle_state;
//for ps4 dpad
int circleAlive = 0;
int dpadUPAlive = 0;
int dpadDOWNAlive = 0;

//===============================================
//Wizley Defined Variables
int targetPosition=0;

//Wizley Defined Variables
//==============================================

int NormalizeAngle(int t)
{
    while(t > 1800) t -= 3600;
    while(t <= -1800) t += 3600;
    return t;           // range from -1799 to +1800
}

float fNormalizeAngle(float t)
{
    while(t > PI) t -= (PI * 2.0);
    while(t <= -PI) t += (PI * 2.0);
    return t;           // range from -PI to PI
}

int AddDeadZone(int value, int tolerance)
{
    if(value > tolerance) value -= tolerance;
    else if(value < -tolerance) value += tolerance;
    else value = 0;
    return value;
}

int Euclidean(int x1, int y1, int x2, int y2){
  float dx = x1 - x2;
  float dy = y1 - y2;
  return (int)sqrt(dx*dx+dy*dy);
}

//rip
float constrain(float value, float positiveLimit, float negativeLimit) {
    if (positiveLimit>negativeLimit){
      if(value > positiveLimit)
          value = positiveLimit;
      if(value < negativeLimit)
          value = negativeLimit;
    }
    else{
      value = (positiveLimit+negativeLimit)/2.0;
    }
	return value;
}

float accelerationLimit_krpm(float speed, float oldSpeed,float upperLimit, float lowerLimit) {
	if(oldSpeed > 0){
		if(speed > oldSpeed + upperLimit) {//forward limit
			speed = oldSpeed + upperLimit;
		}
		if(speed < oldSpeed + -0.2) {
		speed = oldSpeed + -0.2;
		}
	}
	if(oldSpeed <= 0) {
		if(speed > oldSpeed + 0.2) {//backward limit stop stop
			speed = oldSpeed + 0.2;
		}
		if(speed < oldSpeed + lowerLimit) {
		speed = oldSpeed + lowerLimit;
		}
	}
	return speed;
}

int accelerationLimit(int speed, int oldSpeed,int accLimit, int decLimit ) {
	if(oldSpeed > 0){
		if(speed > oldSpeed + accLimit) {//forward limit
			speed = oldSpeed + accLimit;
		}
		if(speed < oldSpeed - decLimit){
			speed = oldSpeed - decLimit;//200 is for main robot;
		}
	}
	if(oldSpeed <= 0) {
		//backward limit stop de-acceleration can be faster than acceleration
		if(speed < oldSpeed - accLimit){
			speed = oldSpeed - accLimit;
		}
		if(speed > oldSpeed + decLimit) {
		speed = oldSpeed + decLimit;
		}
	}
	return speed;
}


//Line sensor
bool LSOnline_prev = false;
int LSCounter = 0;
int LSState = 0;
void Linesensor(void){
  int returnval=0;
  if (LineSensor2016[0].status == 1 && LSOnline_prev){
    if(LSCounter > 0){
      LSCounter--;
    }
    else if (LSCounter == 0){
      LSCounter--;
      returnval= 1;
    }
    else if (LSCounter == -1){
      returnval= 2;
    }
  }
  else if (LineSensor2016[0].status != 1 && !LSOnline_prev){
    if(LSCounter > 0){
      LSCounter--;
    }
    else if (LSCounter == 0){
      LSCounter--;
      returnval= -1;
    }
    else if (LSCounter == -1){
      returnval= -2;
    }
  }
  else if (LineSensor2016[0].status == 1 && !LSOnline_prev){
    LSCounter = 10;
  }
  else if (LineSensor2016[0].status != 1 && LSOnline_prev){
    LSCounter = 10;
  }

  LSOnline_prev = (LineSensor2016[0].status == 1);
  LSState = returnval;
}

//encoder usage
int polezone = 0;
long encoder_1, encoder_2;
long long LencoderSum = 0;
long long RencoderSum = 0;
long yEncoderOffset=0;
int32_t distance_1, distance_2;
int distanceSum;
float yDistance;
float something;
long xDistanceOffset = -500;//offset of the Origin to the Line
long long encoderSum = 0;
void UpdatePosition(void) {
    update_shooter_flags();
    //common code for motor, encoder and line sensor
    if(encoder1_2.Alive == 0) {
      encoder1_2.delta_count[0] = 0;
      encoder1_2.delta_count[1] = 0;
    }
    else{
      LencoderSum += (int16_t)encoder1_2.delta_count[0];
      RencoderSum += (int16_t)encoder1_2.delta_count[1];
    }
    if (abs(LencoderSum) > abs(RencoderSum) + 4000){
      encoderSum = 2 * LencoderSum;
    }
    else if (abs(RencoderSum) > abs(LencoderSum) + 4000) {
      encoderSum = 2 * RencoderSum;
    }
    else{
      encoderSum = LencoderSum+RencoderSum;
    }

    yDistance = (long long)(M[6].Board.EncoderCount-yEncoderOffset) *500 / 395473;

    debug_display[7] = LencoderSum;
    debug_display[8] = RencoderSum;

    int LSpos = 1911 - LineSensor2016[0].position; //line at right = -ve, line at left = +ve;
    Linesensor();
    switch(LSState){
      case 2: //Online
        break;

      case -2: //Offline
        break;

      case 1: //Rising Edge
        if(LSpos>0){
          polezone ++;
        }
        else{
          polezone --;
        }
        break;

      case -1: //Falling Edge
        break;

      default: //Unknown State
        break;

    }
}

DS4_status_t old_data2;
uint8_t PS4_ButtonPress(ButtonEnum b){
     if (b <= LEFT){
         switch (b) {
             case UP:
                 return (DS4.dpad_code == DPAD_LEFT_UP && old_data2.dpad_code == DPAD_OFF)
                     || (DS4.dpad_code == DPAD_UP && old_data2.dpad_code == DPAD_OFF)
                     || (DS4.dpad_code == DPAD_UP_RIGHT && old_data2.dpad_code == DPAD_OFF);
             case RIGHT:
                 return (DS4.dpad_code == DPAD_UP_RIGHT && old_data2.dpad_code == DPAD_OFF)
                     || (DS4.dpad_code == DPAD_RIGHT && old_data2.dpad_code == DPAD_OFF)
                     || (DS4.dpad_code == DPAD_RIGHT_DOWN && old_data2.dpad_code == DPAD_OFF);
             case DOWN:
                 return (DS4.dpad_code == DPAD_RIGHT_DOWN && old_data2.dpad_code == DPAD_OFF)
                     || (DS4.dpad_code == DPAD_DOWN && old_data2.dpad_code == DPAD_OFF)
                     || (DS4.dpad_code == DPAD_DOWN_LEFT && old_data2.dpad_code == DPAD_OFF);
             case LEFT:
                 return (DS4.dpad_code == DPAD_DOWN_LEFT && old_data2.dpad_code == DPAD_OFF)
                     || (DS4.dpad_code == DPAD_LEFT && old_data2.dpad_code == DPAD_OFF)
                     || (DS4.dpad_code == DPAD_LEFT_UP && old_data2.dpad_code == DPAD_OFF);
             default:
                 return 0;
         }
     }else{
       switch (b){
             case SHARE:
               return DS4.share & !(old_data2.share);
             case OPTIONS:
               return DS4.options & !(old_data2.options);
             case L3:
               return DS4.l3 & !(old_data2.l3);
             case R3:
               return DS4.r3 & !(old_data2.r3);
             case L2:
               return DS4.l2 & !(old_data2.l2);
             case R2:
               return DS4.r2 & !(old_data2.r2);
             case L1:
               return DS4.l1 & !(old_data2.l1);
             case R1:
               return DS4.r1 & !(old_data2.r1);
             case TRIANGLE:
               return DS4.triangle & !(old_data2.triangle);
             case CROSS:
               return DS4.cross & !(old_data2.cross);
             case SQUARE:
               return DS4.square & !(old_data2.square);
             case PS:
               return DS4.ps & !(old_data2.ps);
             case TOUCHPAD:
                return DS4.tpad_click & !(old_data2.tpad_click);
             case CIRCLE:
                return DS4.circle & !(old_data2.circle);
             default:
                return 0;
       }
     }
}

void savePS4Data(void){
  memcpy(&old_data2, &DS4, sizeof(old_data2));
}


int prev_target = 0;
int prev_error = 0;
long long i_term = 0;

int y_prev_target = 0;
int y_prev_error = 0;
float y_i_term = 0;
//int y_i_error_limit = 200;


bool XPID(int current, int target){
  if (target != prev_target){
      i_term=0;
      prev_error =0;
  }
  int32_t output = 0;
  int32_t curr_error = target-current;


  if (AddDeadZone(curr_error, 5)){
      output += (int) constrain(curr_error * 3.0, 32767,-32767); //p_term
      output += (int) constrain(i_term * 0, 32767, -32767); //i_term
      output += (int) constrain((curr_error-prev_error) * 0, 32767,-32767); //d_term
      output = (int) constrain(output, 32767,-32767);


      int max_speed = 5000;
      if(curr_error != prev_error){
        max_speed = acceleration_limit * (float)abs(curr_error) / abs(curr_error-prev_error);
        max_speed = (int) constrain(max_speed, 32767,-32767);
      }

      if (output>50)output = constrain(output,max_speed,300);
      else if (output<-50)output = constrain(output,-300,-max_speed);
      else if (output<0) output = ((curr_error/10)-1)*100;
      else if (output>0) output =  ((curr_error/10)+1)*100;

//	  float P_p = (float)(curr_error) * 5.0;
//	  i_term += (curr_error) * 0.5;
//	  i_term = constrain(i_term, 150, -150);
//	  float D_d = (float)(curr_error - prev_error) * 0;
//	  output = constrain(P_p + i_term + D_d,upLimit, lowLimit);


  }
  else{
    output = 0;
    i_term = 0;
    prev_error = 0;
  }



  M[0].SetPoint = output;
  M[1].SetPoint = output;


  prev_target = target;
  prev_error = curr_error;
  i_term+=curr_error;


  return (output==0);
}

int asdasd = 0;
//not finished
bool YPID(int32_t current, int target){
	int output = 0;
	if (target != prev_target){
		y_i_term=0;
		y_prev_error =0;
	}
	long curr_error = target-current;


	if (AddDeadZone(curr_error, 1)){
		float P = constrain(curr_error * 5.0, 300, -300);
		asdasd = P;
		y_i_term += 0.1 * curr_error;
		y_i_term = constrain(y_i_term, 40, -40);
		float D = 0 * (curr_error - y_prev_error);
		output = P + y_i_term + D;
		output = constrain(output, 300, -300);
	}
	else{
		output = 0;
		y_i_term = 0;
		y_prev_error = 0;
	}



  M[6].SetPoint = output;


  y_prev_target = target;
  y_prev_error = curr_error;

  return output==0;
}

//const int zeroPitch = 19382;//19642;
//const int maxPitch = 24727;//26.0
//const int minPitch = 18567;//-6.0
//#define DELTA_PITCH (maxPitch - minPitch)

int getPitch(void){
	int servoPitch = ((uint16_t)(M[5].Board.ADCValue)-ZERO_PITCH) / PITCH_RATIO;
	return servoPitch;
}

/*
 * Roll = 27601 when horizontal // 27599 machine 2
 * Roll = 29630 when rolled to left by 20.0deg. // 20.1 = 31566
 *
 * Roll = 27599 0 degree for machine 2
 * Roll = 31566 20.1 degree for machine 2
 * 30475 15.0
 */
//const int maxRoll = 31566;
int getRoll(void){
	//	int servoRoll = ((uint16_t)(M[4].Board.ADCValue)-zeroRoll) *(0 - 201) * 1.05 /(maxRoll - zeroRoll);
	int servoRoll = (ZERO_ROLL - (uint16_t)(M[4].Board.ADCValue)) / ROLL_RATIO;
	return servoRoll;
}

bool setPitchRoll(int pitch, int roll){
  //Move pitch roll
  pitch=constrain(pitch,300,0);
  roll=constrain(roll,350,-350);
  int pitchAngle = -AddDeadZone(pitch-getPitch(), 0);
  int rollAngle = -AddDeadZone(roll-getRoll(), 0);
  pitchAngle*=(abs(pitchAngle) > 100)? 900:80;
  rollAngle*=(abs(rollAngle) > 100)? 200:50;
  M[2].SetPoint = constrain(pitchAngle + rollAngle, 32367, -32367);
  M[3].SetPoint = constrain(pitchAngle - rollAngle, 32367, -32367);
        /*servoPitch= constrain((int)servoPitch + PITCH_MIN, PITCH_MAX, PITCH_MIN);
        if (servoPitch > Servo1.command[1] )Servo1.command[1]+=(servoPitch > Servo1.command[1] + 20)?3:1;
        else if (servoPitch < Servo1.command[1] )Servo1.command[1]-=(servoPitch < Servo1.command[1] - 20)?2:1;
        */
  return (AddDeadZone(pitchAngle+rollAngle, 80));
}

/*
 * 2 : upper valve R	0 is extended
 * 3 : lower valve R	0 is extended
 * 4 : upper valve L	0 is extended
 * 5 : lower valve L	0 is extended
 * 6 : from right to middle platform		0 is extended
 * 7 : form left to middle platform			0 is extended
 * 8 : hoist								1 is extended
 *
 * Servo1.command[0] and M[4].Board.ADCValue: roll
 * Servo1.command[1] and M[5].Board.ADCValue: pitch
 * Servo1.command[2] : rammer
 *
 * start cases
 * case 1: no disk on left, right, hoist and barrel
 * case 2: disk in barrel
 *
 * end cases
 * case 1: disk at left platform,
 * case 2: disk at right platform
 * case 3: no disk
 *
 * flags
 * lower hoist:		M3 adc(lower)
 * left to mid: 	M2 adc
 * right to mid: 	M6 adc
 */
const int platformNoMiddleYesAlive = 190;
const int raiseHoistAlive = 50;
const int ramAndShootAlive = 150;
const int lowerHoistAlive = 80;


int shooterNextState = PLATFORM_NO_MIDDLE_NO;
int leftPlatform_f = 0;
int leftMiddle_f = 0;
int rightPlatform_f = 0;
int rightMiddle_f = 0;
int lowerHoist_f = 0;
int platformHaveDisc_f = 0;
int retrieveHook_f = 0;
//int leftDisc = 9999;
//int rightDisc = 10000;
int shooterAlive = 0;
int loadError = 0;

void dec_shooter_alive(void) {
	if(shooterAlive > 0) {
		shooterAlive--;
	}
}
void update_shooter_flags(void) {
	lowerHoist_f = CSwitch;
	leftMiddle_f = LSwitch;
	rightMiddle_f = RSwitch;
}

void platform_no_middle_no(void) {
//	load left buffer and right buffer, this state should appear at the start of the competition only
	if(shooterAlive > 0) {
		if(shooterAlive == 100) {
			airSetState(&airBoard, 2, 1);
			airSetState(&airBoard, 4, 1);
		}
		else if(shooterAlive == 1) {
			airSetState(&airBoard, 2, 0);
			airSetState(&airBoard, 4, 0);
		}
		dec_shooter_alive();
	}
	else if(shooterAlive == 0) {
		shooterNextState = PLATFORM_NO_MIDDLE_YES;
		shooterAlive = platformNoMiddleYesAlive;
	}
}

void platform_yes_middle_yes(void) {
	shooterNextState = RAM_AND_SHOOT;
}

void platform_no_middle_yes(void) { // platform does not have disc, both buffer have loaded up disc
	if(shooterAlive >= 0) {
//		if(leftDisc <= rightDisc && rightDisc > 0) {			//if have more disc at left side
////			load_right();
//			if(shooterAlive == platformNoMiddleYesAlive) {
//				airSetState(&airBoard, 3, 1);	//open lower valve
//			}
//			if(shooterAlive == (platformNoMiddleYesAlive - 60)) {
//				airSetState(&airBoard, 3, 0);	//close lower valve
//				airSetState(&airBoard, 6, 1);	//push disc to middle
//			}
//			if(shooterAlive == (platformNoMiddleYesAlive - 80)) {
//				airSetState(&airBoard, 2, 1);	//open upper valve
//			}
//			if(rightMiddle_f > CLOSE) { //
//				airSetState(&airBoard, 2, 0);	//close upper valve
//				airSetState(&airBoard, 6, 0);	//retrieve piston
//				platformHaveDisc_f = TRUE;
//				rightDisc--;
//				if(shooterAlive > 5) {
//					shooterAlive = 5;
//				}
//			}
////			if(shooterAlive == 1){
////				rightdisc--;
////			}
//		}
//		else if (leftDisc > rightDisc && leftDisc > 0) {	//if have more disc on left side
//			//loadleft();
//			if(shooterAlive == platformNoMiddleYesAlive) {
//				airSetState(&airBoard, 5, 1);//open lower valve
//			}
//			if(shooterAlive == (platformNoMiddleYesAlive - 60)) {
//				airSetState(&airBoard, 5, 0); //close lower valve
//				airSetState(&airBoard, 7, 1); //push disc to middle
//			}
//			if(shooterAlive == (platformNoMiddleYesAlive - 80)) {
//				airSetState(&airBoard, 4, 1);  //open upper valve
//			}
//			//add  && retrieveHook_f to it later
//			if(LSwitch){	//limit switch is hitted
//				airSetState(&airBoard, 4, 0);	//close upper valve
//				airSetState(&airBoard, 7, 0);	//retract rightMiddle valve
//				platformHaveDisc_f = TRUE;
//				leftDisc--;
//				if(shooterAlive > 5) {
//					shooterAlive = 5;
//				}
//			}
////			if (shooterAlive == 1) {
////				leftDisc--;
////			}
//		}
		dec_shooter_alive();
	}
	if (shooterAlive == 0 && leftMiddle_f < OPEN && rightMiddle_f < OPEN && platformHaveDisc_f == TRUE) {
		shooterNextState = RAISE_HOIST;
		shooterAlive = raiseHoistAlive;
		airSetState(&airBoard, 2, 0);	//close upper valve
		airSetState(&airBoard, 6, 0);	//retrieve piston
		airSetState(&airBoard, 4, 0);	//close upper valve
		airSetState(&airBoard, 7, 0);	//retract rightMiddle valve
	}
	else if (shooterAlive == 0 && leftMiddle_f < OPEN && rightMiddle_f < OPEN && platformHaveDisc_f == FALSE) {
		airSetState(&airBoard, 4, 0);	//close upper valve
		airSetState(&airBoard, 7, 0);	//retract rightMiddle valve
		shooterAlive = platformNoMiddleYesAlive-1;
		//shooterNextState = LOADING_ERROR;
	}

}

void raise_hoist(void) {
	if(shooterAlive > 0) {
		if(shooterAlive == raiseHoistAlive) {
			airSetState(&airBoard, 8, 1);
		}
		/*
		 * roll
		 * ADC: 10962	Servo: 415
		 * ADC: 7603	Servo: 436
		 *
		 * pitch
		 * ADC: 36877	Servo 417
		 * ADC: 38792	Servo 431
		 */
		if(shooterAlive <= raiseHoistAlive - 10) {
			int servoRoll = abs((int)ADC_ROLL_MIN-(uint16_t)(M[4].Board.ADCValue))/140;//7730
			servoRoll = constrain((int)servoRoll + ROLL_DEFAULT, ROLL_MAX, ROLL_MIN);
			if (servoRoll > Servo1.command[0]) Servo1.command[0]+= (servoRoll > Servo1.command[0] + 20)?3:1;
			else if (servoRoll < Servo1.command[0]) Servo1.command[0]-= (servoRoll < Servo1.command[0] - 20)?2:1;
		}
        if(shooterAlive <= raiseHoistAlive - 10) {
            int servoPitch = abs((uint16_t)(M[5].Board.ADCValue)-ADC_PITCH_MIN)/150;//6625
            servoPitch= constrain((int)servoPitch + PITCH_MIN, PITCH_MAX, PITCH_MIN);
            if (servoPitch > Servo1.command[1] )Servo1.command[1]+=(servoPitch > Servo1.command[1] + 20)?3:1;
            else if (servoPitch < Servo1.command[1] )Servo1.command[1]-=(servoPitch < Servo1.command[1] - 20)?2:1;
        }
		dec_shooter_alive();
	}
	else if(shooterAlive == 0) {
	    int servoRoll = abs((int)ADC_ROLL_MIN-(uint16_t)(M[4].Board.ADCValue))/140;//7730
	    Servo1.command[0] = constrain((int)servoRoll + ROLL_DEFAULT, ROLL_MAX, ROLL_MIN);

	    int servoPitch = abs((uint16_t)(M[5].Board.ADCValue)-ADC_PITCH_MIN)/150;//6625
	    Servo1.command[1]= constrain((int)servoPitch + PITCH_MIN, PITCH_MAX, PITCH_MIN);


		shooterNextState = RAM_AND_SHOOT;
		shooterAlive = ramAndShootAlive;
	}
}
//ram is a term used when ppl push the ammunition into the gun barrel
void ram_and_shoot(void) {
	if(shooterAlive > 0) {
		if(Servo1.command[2] < 401) {
			Servo1.command[2] += 20;
		}
		else {
			if(shooterAlive % 1 == 0) {
				Servo1.command[2] += 2;
			}
		}
		dec_shooter_alive();
	}
	if(shooterAlive == 0 || Servo1.command[2] >= RAMMER_MAX) {
		shooterAlive = lowerHoistAlive;
		Servo1.command[2] = RAMMER_MIN;
		shooterNextState = LOWER_HOIST;
		platformHaveDisc_f = FALSE;
	}
}

void lower_hoist(void) {
	if(shooterAlive > 0) {
		if(shooterAlive == lowerHoistAlive) {
			airSetState(&airBoard, 8, 0);
		}
		if(shooterAlive == lowerHoistAlive-18) {
			Servo1.command[0] = ROLL_DEFAULT;
			Servo1.command[1] = PITCH_MIN;
		}
		dec_shooter_alive();
	}
	else if(shooterAlive == 0 || lowerHoist_f > CLOSE) {
		shooterNextState = PLATFORM_NO_MIDDLE_YES;
		shooterAlive = platformNoMiddleYesAlive;
	}
}

/////////////////////////////////////////////////////////////////////////
//////////////////////////////
bool middleDisc = false;
bool firstPush=true;
bool leftDisc = false;
bool rightDisc = false;
//////////////////////////////

int LMR = 0;
bool shooterError = false;


const unsigned int leftLoaderAliveDefalt = 140;
unsigned int leftLoaderAlive = 0;
void left_loader(bool start){

    if(start){
      if (!leftLoaderAlive){
        leftLoaderAlive=leftLoaderAliveDefalt;
      }
    }
    else if (leftLoaderAlive){
        if(leftLoaderAlive == leftLoaderAliveDefalt) {
            airSetState(&airBoard, 4, 0);//close upper valve
            airSetState(&airBoard, 5, 1);//open lower valve
        }
        else if(leftLoaderAlive > 60) {
            airSetState(&airBoard, 4, 0);
            airSetState(&airBoard, 5, !((leftLoaderAlive/40)%2)); //close lower valve
        }
        else if(leftLoaderAlive >  0) {
          airSetState(&airBoard, 4, !((leftLoaderAlive/40)%2));
          airSetState(&airBoard, 5, 0);
        }
    leftLoaderAlive--;
    }
    else{
          airSetState(&airBoard, 4, 0);
          airSetState(&airBoard, 5, 0);
    }
}

const unsigned int rightLoaderAliveDefalt = 140;
unsigned int rightLoaderAlive = 0;

void right_loader(bool start){
    if(start){
      if(!rightLoaderAlive){
        rightLoaderAlive=rightLoaderAliveDefalt;
      }
    }
    else if (rightLoaderAlive){
        if(rightLoaderAlive == rightLoaderAliveDefalt) {
            airSetState(&airBoard, 2, 0);   //close upper valve
            airSetState(&airBoard, 3, 1);   //open lower valve
        }
        else if(rightLoaderAlive > 60) {
            airSetState(&airBoard, 2, 0);
            airSetState(&airBoard, 3, !((rightLoaderAlive/40)%2)); //close lower valve
        }
        else if(rightLoaderAlive > 0) {
          airSetState(&airBoard, 2, !((rightLoaderAlive/40)%2));
          airSetState(&airBoard, 3, 0);
        }
    rightLoaderAlive--;
    }
    else{
      airSetState(&airBoard, 2, 0);
      airSetState(&airBoard, 3, 0);
    }

}

bool lastDiscIsLeft = true;
unsigned int pusherAlive = 0;
const unsigned int pusherAliveDefault = 70;
void pusher(bool start){
  if(start && !pusherAlive){
    pusherAlive=pusherAliveDefault;
  }
  else if(pusherAlive){
    if(pusherAlive == pusherAliveDefault){   //Select Disc and push to centre
        airSetState(&airBoard, 6, 0); //Retrieve right disc to middle
        airSetState(&airBoard, 7, 0);
        if (CSwitch||PS4_ButtonPress(TRIANGLE)){
          pusherAlive--;
        }
    }
    else{
      if(pusherAlive == pusherAliveDefault-1){
          if ((!rightLoaderAlive && leftLoaderAlive)||
              (!rightLoaderAlive && !leftLoaderAlive && lastDiscIsLeft)){
              airSetState(&airBoard, 6, 1); //Push right disc to middle
          }
          else if ((!leftLoaderAlive && rightLoaderAlive)||
              (!rightLoaderAlive && !leftLoaderAlive && !lastDiscIsLeft)){
              airSetState(&airBoard, 7, 1); //Push left disc to middle
          }
        }
        if(pusherAlive == pusherAliveDefault-55){ // Retrieve piston and check if centre have disc
            if (!LSwitch && !RSwitch && !(DS4.triangle)){
                airSetState(&airBoard, 6, 0); //Retrieve right disc piston
                airSetState(&airBoard, 7, 0); //Retrieve left disc piston
                pusherAlive=pusherAliveDefault+50;
            }
            else{
              airSetState(&airBoard, 6, 0); //Retrieve right disc piston
              airSetState(&airBoard, 7, 0); //Retrieve left disc to piston
            }
        }
        if(pusherAlive == 1){ //Raise Hoist & Load Other Disc
          switch(LMR){
            case -1:
              left_loader(true);
              lastDiscIsLeft=true;
              break;
            case 1:
              right_loader(true);
              lastDiscIsLeft=false;
              break;
            default:
                if(lastDiscIsLeft){
                  right_loader(true);
                  lastDiscIsLeft=false;
                }
                else{
                  left_loader(true);
                  lastDiscIsLeft=true;
                }
              break;
          }

                            /*
                             * roll
                             * ADC: 10962   Servo: 415
                             * ADC: 7603    Servo: 436
                             *
                             * pitch
                             * ADC: 36877   Servo 417
                             * ADC: 38792   Servo 431
                             */
        }
        pusherAlive--;
    }
  }
  else{
      airSetState(&airBoard, 6, 0); //Retrieve right disc to middle
      airSetState(&airBoard, 7, 0);
  }
}



const unsigned int shooterAliveDefault = 180;
long currentCount = 0;

//unsigned int shooterAlive = 500;
void shooter(bool start){
  if(start && !shooterAlive && !pusherAlive){
    shooterAlive=shooterAliveDefault;
    shooterError = false;
  }
  else if (shooterAlive>0) {
      if(shooterAlive == shooterAliveDefault){
        Servo1.command[0] = ROLL_DEFAULT;
        Servo1.command[1] = PITCH_MIN;
        Servo1.command[2] = RAMMER_MIN;
        airSetState(&airBoard, 8, 1);
        currentCount = 0;
      }
      if (shooterAlive < shooterAliveDefault-20 && shooterAlive > shooterAliveDefault-40){ // Central Platform follow Pitch and Roll
            int servoRoll = constrain(ROLL_DEFAULT + getRoll()*SERVO_STEP_ROLL, ROLL_MAX, ROLL_MIN);
            if (servoRoll > Servo1.command[0]) Servo1.command[0]+= (servoRoll > Servo1.command[0] + 20)?2:1;
            else if (servoRoll < Servo1.command[0]) Servo1.command[0]-= (servoRoll < Servo1.command[0] - 20)?2:1;
      }
      if(shooterAlive < shooterAliveDefault-20 && shooterAlive > shooterAliveDefault-40){
            int servoPitch = constrain(PITCH_MIN + getPitch()*SERVO_STEP_PITCH, PITCH_MAX, PITCH_MIN);
            if (servoPitch > Servo1.command[1] )Servo1.command[1]+=(servoPitch > Servo1.command[1] + 10)?6:2;
            else if (servoPitch < Servo1.command[1] )Servo1.command[1]-=(servoPitch < Servo1.command[1] - 20)?2:1;
      }
      if (shooterAlive == shooterAliveDefault-50){
                  Servo1.command[0]=constrain(ROLL_DEFAULT + getRoll()*SERVO_STEP_ROLL, ROLL_MAX, ROLL_MIN);
                  Servo1.command[1]=constrain(PITCH_MIN + getPitch()*SERVO_STEP_PITCH, PITCH_MAX, PITCH_MIN);
      }
      if (shooterAlive < shooterAliveDefault-70){
        if(Servo1.command[2]<RAMMER_MAX){
//          if(shooterAlive % 1 == 0 && Servo1.command[2] < 420) {
//              Servo1.command[2] += 2+((420-Servo1.command[2])/20);
//          }
//          else {
//            if(shooterAlive % 1 == 0) {
//              Servo1.command[2] += 1;
//            }
//          }
            Servo1.command[2] = Servo1.command[2] + ((RAMMER_MAX - (Servo1.command[2]))/8.0);
            Servo1.command[2] +=1;
        }

        if((M[4].SetPoint-M[4].Feedback)>(4+(M[4].SetPoint / 100))){
            currentCount+=M[4].SetPoint-M[4].Feedback;
        }
      }
      if(shooterAlive>shooterAliveDefault-100){
        airSetState(&airBoard, 8, 1);
      }
      if(shooterAlive == shooterAliveDefault-120){
        //Servo1.command[0] = ROLL_DEFAULT;
        //Servo1.command[1] = PITCH_MIN;
        airSetState(&airBoard, 8, 0);
        //pusher(true);
      }
      if(shooterAlive == shooterAliveDefault-140){
        Servo1.command[0] = ROLL_DEFAULT;
      }
      if(shooterAlive == shooterAliveDefault-170){
        Servo1.command[1] = PITCH_MIN;
      }
      if (currentCount>100){
        pusher(true);
      }
      if(shooterAlive == 1 && currentCount < 100){
        Servo1.command[2] = RAMMER_MIN;
        shooterAlive=-50;
        Servo1.command[0]=constrain(ROLL_DEFAULT + getRoll()*SERVO_STEP_ROLL, ROLL_MAX, ROLL_MIN);
        Servo1.command[1]=constrain(PITCH_MIN + getPitch()*SERVO_STEP_PITCH, PITCH_MAX, PITCH_MIN);
        airSetState(&airBoard, 8, 1);
      }
      debug_display[6] = currentCount;
  shooterAlive--;
  }
  else if(shooterAlive <0){
    if(shooterAlive == -1){
      shooterAlive = shooterAliveDefault-40;
    }
  shooterAlive++;
  }
  else{
    Servo1.command[0] = ROLL_DEFAULT;
    Servo1.command[1] = PITCH_MIN;
    Servo1.command[2] = RAMMER_MIN;
    airSetState(&airBoard, 8, 0); //lower platform
  }
}

bool shoot_left = true;
int loaderAlive = 0;
void reload_and_shoot(void) {
/*
	if(shooterNextState == PLATFORM_NO_MIDDLE_NO) {
		platform_no_middle_no();
	}
	if(shooterNextState == PLATFORM_YES_MIDDLE_YES) {
		platform_yes_middle_yes();
	}
	if(shooterNextState == PLATFORM_NO_MIDDLE_YES) {
		platform_no_middle_yes();
	}
	if(shooterNextState == RAISE_HOIST) {
		raise_hoist();
	}
	if(shooterNextState == RAM_AND_SHOOT) {
		ram_and_shoot();
	}
	if(shooterNextState == LOWER_HOIST) {
		lower_hoist();
	}
*/
  /////////////////////////////////////////////////////////////////////////


      shooter(true);
      shooter(false);
      pusher(false);
      left_loader(false);
      right_loader(false);

}

int xOldMotorSpeed = 0;
bool lastMovePitch = false;
int lastpr = 0;
int accuprchange = 0;
void runManual(void) {

        //X Motor Control
    int left_head_x = (int)((uint16_t)(DS4.hat_left_x) - 128);
    int left_head_y = (int)((uint16_t)(DS4.hat_left_y) - 128);
    int xMotorSpeed = 0;
    int yMotorSpeed = 0;
    if (abs(left_head_x) > abs(left_head_y)){
      // Move X Moter
        xMotorSpeed = -AddDeadZone((int)((uint16_t)(DS4.hat_left_x) - 128), 10 );
        xMotorSpeed = xMotorSpeed*5+((xMotorSpeed>0)? 4700:-4700)*(float)xMotorSpeed*xMotorSpeed/13942.0;
        xMotorSpeed*=2;
    }
    else if (abs(left_head_x) < abs(left_head_y)){
      //Move Y
        yMotorSpeed = -AddDeadZone((int)((uint16_t)(DS4.hat_left_y) - 128), 10) * 5;


    }
    M[0].SetPoint = xMotorSpeed;
    M[1].SetPoint = xMotorSpeed;
    if(M[6].Setting.Mode==motor_Pmode){
      M[6].SetPoint = M[6].Feedback+yMotorSpeed;
    }
    else if (M[6].Setting.Mode==motor_Vmode){
      M[6].SetPoint = yMotorSpeed;
    }
    //Move pitch roll
/*
    int pitchAngle = 0;
    int rollAngle = 0;
    if (abs(DS4.hat_right_x - 128) > abs(DS4.hat_right_y - 128)){
      rollAngle = -AddDeadZone((int)((uint16_t)(DS4.hat_right_x) - 128), HatDeadzone);
      if (rollAngle > 0){
        rollAngle = 100 + 2000*(float)abs(rollAngle)*abs(rollAngle)*abs(rollAngle)*abs(rollAngle)*rollAngle/34359738368.0;
      }
      else if (rollAngle < 0){
        rollAngle = -100 + 2000*(float)abs(rollAngle)*abs(rollAngle)*abs(rollAngle)*abs(rollAngle)*rollAngle/34359738368.0;
      }
    }
    else if (abs(DS4.hat_right_x - 128) < abs(DS4.hat_right_y - 128)){
      pitchAngle = -AddDeadZone((int)((uint16_t)(DS4.hat_right_y) - 128), HatDeadzone);
      if(pitchAngle > 0 ){
        pitchAngle = 100 + 2000*(float)abs(pitchAngle)*abs(pitchAngle)*abs(pitchAngle)*abs(pitchAngle)*pitchAngle/34359738368.0;
      }
      else if (pitchAngle < 0){
        pitchAngle = -100 + 2000*(float)abs(pitchAngle)*abs(pitchAngle)*abs(pitchAngle)*abs(pitchAngle)*pitchAngle/34359738368.0;
      }

    }
    //pitchAngle= pitchAngle*5 + 00*(float)abs(pitchAngle)*abs(pitchAngle)*pitchAngle/2097152.0;
    //rollAngle = rollAngle*5 + 00*(float)abs(rollAngle)*abs(rollAngle)*rollAngle/2097152.0;
    M[2].SetPoint = pitchAngle + rollAngle;
    M[3].SetPoint = pitchAngle - rollAngle;
*/

    int moveRoll = AddDeadZone((int)((uint16_t)(DS4.hat_right_x) - 128), HatDeadzone);
    int movePitch = AddDeadZone((int)((uint16_t)(DS4.hat_right_y) - 128), HatDeadzone);

    if (!moveRoll && !movePitch){
      M[2].SetPoint=0;
      M[3].SetPoint=0;
    }
    else if (abs(moveRoll)>abs(movePitch)){
      if (lastMovePitch){
        lastpr=getPitch();
        accuprchange=0;
        lastMovePitch = false;
      }
      else{
        accuprchange += moveRoll;
        setPitchRoll(lastpr,getRoll()+accuprchange/20);
        if(accuprchange/10){
          accuprchange=0;
        }
      }
    }
    else if(abs(movePitch)>abs(moveRoll)){
      if (!lastMovePitch){
        lastpr=getRoll();
        accuprchange=0;
        lastMovePitch = true;
      }
      else{
        accuprchange += movePitch;
        setPitchRoll(getPitch()+accuprchange/20,lastpr);
        if(accuprchange/10){
          accuprchange=0;
        }
      }
    }

    //Change Shooter Speed
    int shootSpeed = (M[4].SetPoint+M[5].SetPoint)/2;
    if(DS4.dpad_code == DPAD_UP) { //pressing UP
      //Increase Shooting Speed
        if(dpadUPAlive == 0) {
          shootSpeed += 5;
        }
        if(dpadUPAlive > 20 && dpadUPAlive % 8 == 0 && DS4.r1) {
          shootSpeed += 20;
        }
        dpadUPAlive++;
    }
    else {
        dpadUPAlive = 0;
    }

    if(DS4.dpad_code == DPAD_DOWN) {   //pressing down
      //Decrease Shooting Speed
        if(dpadDOWNAlive == 0) {
          shootSpeed -= 5;
        }
        if(dpadDOWNAlive > 20 && dpadDOWNAlive % 8 == 0 && DS4.r1) {
          shootSpeed -= 20;
        }
        dpadDOWNAlive++;
    }
    else {
        dpadDOWNAlive = 0;
    }

    if(DS4.r1){
      if(PS4_ButtonPress(LEFT)){
        LMR = constrain(--LMR, 1, -1);
      }
      if(PS4_ButtonPress(RIGHT)){
        LMR = constrain(++LMR, 1, -1);
      }
    }
    shootSpeed = constrain(shootSpeed, 3000, 200);
    M[4].SetPoint = shootSpeed;
    M[5].SetPoint = shootSpeed;

    //Move Shoot Speed
//    M[5].SetPoint = shootSpeed[currentPos];
//    M[4].SetPoint = shootSpeed[currentPos];
    //Y Motor Control
}

void runAuto(PositionStates *set, int pos) {
  if (pos == 0 || pos == 8){
    airSetState(&airBoard, 3, 0);
    airSetState(&airBoard, 5, 0);
    airSetState(&airBoard, 6, 0);
    airSetState(&airBoard, 7, 0);
    airSetState(&airBoard, 8, 0);
    Servo1.command[0] = (ROLL_MIN+ROLL_MAX)/2;
    Servo1.command[1] = PITCH_MIN;
    Servo1.command[2] = RAMMER_MIN;
    shooterAlive=0;
    pusherAlive = 0;
    airSetState(&airBoard, 2, 1);
    airSetState(&airBoard, 4, 1);
    setPitchRoll(set[pos].pitch, set[pos].roll);
    XPID(distanceSum, set[pos].x);
    YPID((int)yDistance, set[pos].y);
    left_loader(true);
    right_loader(true);
    pusher(true);
  }
  else{
      YPID((int)yDistance, set[pos].y);
      XPID(distanceSum, set[pos].x);
      if(!setPitchRoll(set[pos].pitch, set[pos].roll) && !AddDeadZone(distanceSum-set[pos].x, 10)){
          reload_and_shoot();
      }
  }
  M[4].SetPoint = set[pos].shootspd;
  M[5].SetPoint = set[pos].shootspd;
  return;
}

void castLimit(void){
  //XMovement Speed
  M[0].SetPoint = accelerationLimit(M[0].SetPoint, xOldMotorSpeed, acceleration_limit, 3*acceleration_limit);
  M[0].SetPoint = constrain(M[0].SetPoint, upLimit, lowLimit);
  M[1].SetPoint = accelerationLimit(M[1].SetPoint, xOldMotorSpeed, acceleration_limit, 3*acceleration_limit);
  M[1].SetPoint = constrain(M[1].SetPoint, upLimit, lowLimit);
  xOldMotorSpeed = (M[0].SetPoint+M[1].SetPoint)/2;
  //Shooter Speed-
  M[4].SetPoint = constrain( M[4].SetPoint, 3000, 200);
  M[5].SetPoint = constrain( M[5].SetPoint, 3000, 200);
  //Hoist Motor
  M[2].SetPoint = constrain(M[2].SetPoint, 32367, -32367);
  M[3].SetPoint = constrain(M[3].SetPoint, 32367, -32367);
  M[6].SetPoint = constrain(M[6].SetPoint, 300, -300);
  //YMotor Limit Control
  if(BSwitch && M[6].SetPoint<0) {//backward limit switch
    M[6].SetPoint = 0;
  }

  if (BSwitch){
    yEncoderOffset=M[6].Board.EncoderCount;
  }

  if (FSwitch && M[6].SetPoint>0) {//forward limit switch
    M[6].SetPoint = 0;
  }
}

//relocate is a term used when repositioning artilleries
void relocate(void) {

}

void runMode(PositionStates *set, int pos){

}



