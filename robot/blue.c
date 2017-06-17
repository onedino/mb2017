
/*
 * red.c
 *
 *  Created on: 2017-01-02
 *      Author: Lau
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
#include "DS4.h"
#include "analog.h"
#include "airboard.h"
#include "distancesensor.h"
#include "red.h"
/*
 * for searching purpose
 * START_STATE
 * AUTO_STATE
 * MANUAL_STATE
 */

/*
 * common for both manual and auto
 */

void updateOldPS4Data (void) {
    savePS4Data();
	old_cross_state = DS4.cross;
	old_circle_state = DS4.circle;
	old_square_state = DS4.square;
	old_triangle_state = DS4.triangle;

	old_l1_state = DS4.l1;
	old_r1_state = DS4.r1;
	old_dpad_state = DS4.dpad_code;
}





PositionStates blueStateSet[10]={
// {id      ,x      ,y      ,pitch  ,roll   ,shootspd   },
//   {        ,       ,       ,       ,       ,           },

   {0       , 500    ,0      ,0      ,0      ,0          }, //Start Zone
   {1       , -883   ,336    ,130    ,-93    ,905        },
   {2       , -1864  ,250    ,235    ,85    ,1050       },
   {3       , -4600  ,250    ,238    ,-24      ,710        }, //Middle Near
   {4       , -4454  ,398    ,202    ,193    ,1105       }, //Middle Middle
   {5       , -2830  ,353    ,196    ,154     ,1390       }, //Middle Far
   {6       , -5730  ,350    ,215    ,91    ,1040       },
   {7       , -7767  ,320    ,190    ,33     ,990        },
   {8       , -12475 ,0      ,108    ,5      ,0          },  //Loading Zone
   {9		, -6000	 ,0		 ,1		 ,-2	 ,1800		 }
};


void displayDebug (void) {
	debug_display[0] = blueStateSet[targetPosition].pitch;//M[0].Board.ADCValue;
	debug_display[1] = blueStateSet[targetPosition].roll;//M[1].Board.ADCValue;
	debug_display[2] = blueStateSet[targetPosition].x;
//	debug_display[3] = M[3].Board.ADCValue;
//	debug_display[4] = M[6].Board.ADCValue;//M[4].Feedback;	//left to mid
//	debug_display[5] = DS4.tpad_click;//M[5].Feedback;	//right to mid
//    debug_display[6] = 0;
//	debug_display[7] = DS4.tpad_info[0].finger[0].y;
//    debug_display[8] = DS4.tpad_info[0].finger[0].not_touching;
//    debug_display[9] = targetPosition;
}

bool init = false;
void RunPath (void) {
	UpdatePosition();
    distanceSum = (int32_t)((float) encoderSum * 13000.0 / 667170) - xDistanceOffset;

	if (!init){
	  /*
	  for(int i=0; i<9; i++){
	    blueStateSet[i]=redStateSet[i];
	  }
	  blueStateSet[0].x =500;
	  blueStateSet[1].x = (redStateSet[7].x - 12000);
	  blueStateSet[2].x = (redStateSet[6].x - 12000);
	    blueStateSet[3].x = (redStateSet[3].x - 12000);
	      blueStateSet[4].x = (redStateSet[4].x - 12000);
	      blueStateSet[5].x = (redStateSet[5].x - 12000);
	      blueStateSet[6].x = (redStateSet[2].x - 12000);
	      blueStateSet[7].x = (redStateSet[1].x - 12000);
	  */
//      xDistanceOffset = blueStateSet[0].x;

      airSetState(&airBoard, 2, 1);
      airSetState(&airBoard, 4, 1);
      airSetState(&airBoard, 0, 1);
      airSetState(&airBoard, 1, 1);
//      if (!BSwitch && !PS4_ButtonPress(TRIANGLE)){
//        M[6].SetPoint = -100;
//      }
//      else{
//        M[6].SetPoint = 0;
//        if (!DS4.circle){
//          init = true;
//        }
//      }
      init = true;
	}
	else{
        if(DS4.l2_trigger>200) {
          targetPosition = 0; //reload position
          firstPush=false;
          leftDisc=false;
          rightDisc=false;
          runAuto(blueStateSet, targetPosition);
          defenseState = 0;
        }
        else if(DS4.r2_trigger>200) {
          targetPosition = 8;
          runAuto(blueStateSet, targetPosition);
          defenseState = 0;
        }
        else if(DS4.l1) {
	      runAuto(blueStateSet, targetPosition);
	    }
	    if (PS4_ButtonPress(OPTIONS)){
	      encoder_resetcount(0);
	      encoder_resetcount(1);
	    }
//        else if(PS4_ButtonPress(OPTIONS)) {
//        	if(defenseStxate == 0) {
//				prevPosition = targetPosition;
//				targetPosition = 9;
//				defenseState = 1;
//        	}
//        	else if(defenseState == 1) {
//        		targetPosition = prevPosition;
//        		defenseState = 0;
//        	}
//        }
	    else{
	         //Manual Mode
	        //updateData ();
	        runManual();
	        //checkForAuto ();
	    }

	    if(PS4_ButtonPress(R1)){
	        if(PS4_ButtonPress(SQUARE)) {
	             EEPread(blueStateSet, targetPosition);
	        }
	     }
	     else if(PS4_ButtonPress(SQUARE)) {
	    	     blueStateSet[targetPosition].pitch = getPitch();
	    	     blueStateSet[targetPosition].roll = getRoll();
	    	     blueStateSet[targetPosition].x = distanceSum;
	    	     blueStateSet[targetPosition].y = yDistance;
	    	     blueStateSet[targetPosition].shootspd = (M[4].SetPoint+M[5].SetPoint)/2;

	    	     EEPwrite(blueStateSet, targetPosition);
	     }
	//Update position according to pole aiming
	    if(PS4_ButtonPress(RIGHT) && !DS4.r1) {
	      //Shift the pole aiming at
	        //For Wizley
	       targetPosition = constrain(++targetPosition, 8, 0);
			if(targetPosition != 9) {
				defenseState = 0;
			}
	    }

	    if(PS4_ButtonPress(LEFT) && !DS4.r1) {
	        //Shift the pole aiming at
	        //For Wizley
	        targetPosition = constrain(--targetPosition, 8, 0);
	        if(targetPosition != 9) {
	        	defenseState = 0;
	        }
	    }

//	    if(DS4.tpad_click){
//	      switch(DS4.tpad_info[0].finger[0].x / 400){
//	      case 0:
//	        targetPosition=1;
//	        break;
//	      case 1:
//	        targetPosition=2;
//	        break;
//	      case 2:
//	        switch(DS4.tpad_info[0].finger[0].y / 300){
//	        case 0:
//	          targetPosition=5;
//	          break;
//	        case 1:
//	          targetPosition=4;
//	          break;
//	        case 2:
//	          targetPosition=3;
//	          break;
//	        }
//	        break;
//	      case 3:
//	        targetPosition=6;
//	        break;
//	      case 4:
//	        targetPosition=7;
//	        break;
//	      }
//	    }

        if(PS4_ButtonPress(PS)&& DS4.cross) {
          pusher(true);
        }

	    if(DS4.cross) {
	      //Reset all pneumatic to initial mode
	       // airSetState(&airBoard, 0, 0);
	       // airSetState(&airBoard, 1, 0);
	        airSetState(&airBoard, 2, 0);
	        airSetState(&airBoard, 3, 0);
	        airSetState(&airBoard, 4, 0);
	        airSetState(&airBoard, 5, 0);
	        airSetState(&airBoard, 6, 0);
	        airSetState(&airBoard, 7, 0);
	        airSetState(&airBoard, 8, 0);
	        Servo1.command[0] = (ROLL_MIN+ROLL_MAX)/2;
	        Servo1.command[1] = PITCH_MIN;
	        Servo1.command[2] = RAMMER_MIN;
	        shooterAlive=0;
	    }


	    if(DS4.circle) {
	        airSetState(&airBoard, 0,1);//base
	        airSetState(&airBoard, 1,1);//base
	        if (targetPosition != 8 && targetPosition != 0){
	           reload_and_shoot();
	        }
	    }
	    else if ((airBoard.state)&(1<<8)){
	      int servoRoll = constrain(ROLL_DEFAULT + getRoll()*SERVO_STEP_ROLL, ROLL_MAX, ROLL_MIN);
	      if (servoRoll > Servo1.command[0]) Servo1.command[0]+= (servoRoll > Servo1.command[0] + 20)?3:1;
	      else if (servoRoll < Servo1.command[0]) Servo1.command[0]-= (servoRoll < Servo1.command[0] - 20)?2:1;

	      int servoPitch = constrain(PITCH_MIN + getPitch()*SERVO_STEP_PITCH, PITCH_MAX, PITCH_MIN);
	      if (servoPitch > Servo1.command[1] )Servo1.command[1]+=(servoPitch > Servo1.command[1] + 20)?2:1;
	      else if (servoPitch < Servo1.command[1] )Servo1.command[1]-=(servoPitch < Servo1.command[1] - 20)?2:1;

	    }
        else if (!((airBoard.state)&(1<<8))){
          Servo1.command[0] = ROLL_DEFAULT;
          Servo1.command[1] = PITCH_MIN;
        }
	}
    castLimit();
    displayDebug ();
	updateOldPS4Data ();


}
