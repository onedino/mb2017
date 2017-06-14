#ifndef _MOTOR_H_
#define _MOTOR_H_


#include "udc.h"
#define MOTOR_STATE_OK 0x8000

#define CAL_ID_VMODE(n) (8+(n*4)+0)
#define CAL_ID_PMODE(n) (8+(n*4)+1)
#define CAL_ID_M_STATUS(n) (8+(n*4)+2)
#define CAL_ID_M_SETTING(n) (8+(n*4)+3)

#define ID_M0_VMODE CAL_ID_VMODE(0)
#define ID_M1_VMODE CAL_ID_VMODE(1)
#define ID_M2_VMODE CAL_ID_VMODE(2)
#define ID_M3_VMODE CAL_ID_VMODE(3)
#define ID_M4_VMODE CAL_ID_VMODE(4)
#define ID_M5_VMODE CAL_ID_VMODE(5)
#define ID_M6_VMODE CAL_ID_VMODE(6)
#define ID_M7_VMODE CAL_ID_VMODE(7)

#define ID_M0_PMODE CAL_ID_PMODE(0)
#define ID_M1_PMODE CAL_ID_PMODE(1)
#define ID_M2_PMODE CAL_ID_PMODE(2)
#define ID_M3_PMODE CAL_ID_PMODE(3)
#define ID_M4_PMODE CAL_ID_PMODE(4)
#define ID_M5_PMODE CAL_ID_PMODE(5)
#define ID_M6_PMODE CAL_ID_PMODE(6)
#define ID_M7_PMODE CAL_ID_PMODE(7)

#define ID_M0_STATUS CAL_ID_M_STATUS(0)
#define ID_M1_STATUS CAL_ID_M_STATUS(1)
#define ID_M2_STATUS CAL_ID_M_STATUS(2)
#define ID_M3_STATUS CAL_ID_M_STATUS(3)
#define ID_M4_STATUS CAL_ID_M_STATUS(4)
#define ID_M5_STATUS CAL_ID_M_STATUS(5)
#define ID_M6_STATUS CAL_ID_M_STATUS(6)
#define ID_M7_STATUS CAL_ID_M_STATUS(7)

#define ID_M0_SETTING CAL_ID_M_SETTING(0)
#define ID_M1_SETTING CAL_ID_M_SETTING(1)
#define ID_M2_SETTING CAL_ID_M_SETTING(2)
#define ID_M3_SETTING CAL_ID_M_SETTING(3)
#define ID_M4_SETTING CAL_ID_M_SETTING(4)
#define ID_M5_SETTING CAL_ID_M_SETTING(5)
#define ID_M6_SETTING CAL_ID_M_SETTING(6)
#define ID_M7_SETTING CAL_ID_M_SETTING(7)

typedef enum{
  motor_idle = 0,
  motor_brake,
  motor_Vmode,
  motor_Pmode
}motor_mode_e;

typedef struct{
  int16_t Voltage;      // in mV
  int16_t Current;      // in mA
  int16_t Temperature;  // in Degree Celsius
  int32_t EncoderCount;
  uint16_t ADCValue;
  uint16_t State;
}__attribute__((packed)) board_status_t;

typedef struct{
    /** 
     *  IDLE  : Motor terminals is left open (not shorted or driven)
     *  BRAKE : Motor terminals are short
	 *  VMODE : Motor is running in V Mode
	 *  PMODE : Motor is running in P Mode
	 *  For VMODE and PMODE, if there is no correct set point command received,
     *  motor will be in brake state
     */
    //enum { IDLE = 0, BRAKE = 1, VMODE = 2, PMODE = 3 } Mode ;
  uint16_t Mode;
  float ScaleFactor;				// PID Controller SetPoint = Input SetPoint * ScaleFactor
  uint16_t AccelerationLimit;	// in Encoder count / CommandCycle
  uint16_t SpeedLimit;		// in Encoder count / CommandCycle
	uint16_t CommandCycle;		// in us
	uint16_t MotorVoltage;		// in mV
	uint16_t CurrentLimit;		// in mA
	int16_t kP;
	int16_t kI;
	int16_t kD;
	int16_t kFF;
}__attribute__((packed)) motor_setting_t;

typedef void (*setAliveCallback) (void);

typedef struct{
	int16_t SetPoint;
	int16_t Feedback;

	board_status_t Board;
	motor_setting_t Setting;
	const uint8_t id;
//<<<<<<< HEAD
//	uint8_t Alive;
//=======
	uint8_t timeout;
//>>>>>>> b1ab300e10eec07a0da4c8f514d46b53bed302ea
}MotorObj;

extern MotorObj M[8];
extern const motor_setting_t DefaultIdle;
extern const motor_setting_t DefaultBrake;
extern const motor_setting_t DefaultVMode;
extern const motor_setting_t DefaultPMode;
extern const motor_setting_t M0VMode;
extern const motor_setting_t M1VMode;
extern const motor_setting_t M2VMode;
extern const motor_setting_t M3VMode;
extern const motor_setting_t M4VMode;
extern const motor_setting_t M5VMode;
extern const motor_setting_t M6VMode;
extern const motor_setting_t M6PMode;
extern const motor_setting_t M7VMode;


void motor_init(MotorObj *motor, const motor_setting_t *cfg);
//<<<<<<< HEAD
////<<<<<<< HEAD
////udc_rx_state_e motor_send_setpoint(MotorObj *motor);
////udc_rx_state_e motor_get_status(MotorObj *motor);
////udc_rx_state_e motor_send_setting(MotorObj *motor);
////udc_rx_state_e motor_setIdle(MotorObj *motor);
////udc_rx_state_e motor_setBrake(MotorObj *motor);
//void motor_setAlive( void * arg);
//void motor_decAlive(void * arg);
////=======
//umd_return_e motor_send_setpoint(MotorObj *motor);
//umd_return_e motor_get_status(MotorObj *motor);
//umd_return_e motor_send_setting(MotorObj *motor);
//umd_return_e motor_setIdle(MotorObj *motor);
//umd_return_e motor_setBrake(MotorObj *motor);
//=======
udc_rx_state_e motor_send_setpoint(MotorObj *motor);
udc_rx_state_e motor_get_status(MotorObj *motor);
udc_rx_state_e motor_send_setting(MotorObj *motor);
udc_rx_state_e motor_setIdle(MotorObj *motor);
udc_rx_state_e motor_setBrake(MotorObj *motor);
void motor_setAlive(void * arg);
void motor_decAlive(void * arg);
//2016 board


typedef enum{
  FAULT_UVLO,
  FAULT_PARAM_INVALID,
  FAULT_EMERGENCY_SWITCH
}board_fault_2016_e;

typedef struct{
  uint8_t fault : 1;
  uint8_t align_ok : 1;
  uint8_t param_loaded : 1;
  uint8_t est_ok : 1;
  uint8_t st_id_ok : 1;
  uint8_t profile_idle : 1;
  uint8_t homing_ok : 1;
  uint8_t setting_loaded : 1;

  uint8_t fault_code;
}__attribute__((packed)) board_state_2016_t;

typedef enum{
  UNLOCK,
  VMODE,
  PMODE,
  HOMING
}motor_mode_2016_e;

typedef struct{
  float voltage_V;
  float current_A;
  float torque_nm;

  uint32_t adc1 : 12;
  uint32_t adc2 : 12;
  uint32_t temperature : 8;

  board_state_2016_t state;
}__attribute__((packed)) board_status_2016_t;

typedef struct{
  uint8_t mode;
  uint8_t curvetype;
  float bandwidth;
  float max_jerk_krpmps2;
  float max_accel_krpmps;
  float max_speed_krpm;
  float max_current_A;
}__attribute__((packed)) motor_setting_2016_t;

typedef struct{
  float speed_ref_krpm;
  int32_t pos_ref_mrev;

  float speed_krpm;
  int32_t pos_mrev;

  motor_setting_2016_t setting;
  board_status_2016_t status;

  uint32_t timeout;
  uint8_t id;

}__attribute__((packed)) motor_2016_t;

extern motor_2016_t m[8];
extern const motor_setting_2016_t DefaultIdle_2016;
//extern const motor_setting_t DefaultBrake;
extern const motor_setting_2016_t DefaultVMode_2016;
extern const motor_setting_2016_t DefaultPMode_2016;


void motor_init_2016(motor_2016_t *motor, const motor_setting_2016_t *cfg);
udc_rx_state_e motor_send_setpoint_2016(motor_2016_t *motor);
udc_rx_state_e motor_get_status_2016(motor_2016_t *motor);
udc_rx_state_e motor_send_setting_2016(motor_2016_t *motor);
udc_rx_state_e motor_setIdle_2016(motor_2016_t *motor);

#endif  /* _MOTOR_H_ */
