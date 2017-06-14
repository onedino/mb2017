
#ifndef MODULES_DS4_DS4_H_
#define MODULES_DS4_DS4_H_

typedef enum{
  DPAD_UP = 0x0,
  DPAD_UP_RIGHT = 0x1,
  DPAD_RIGHT = 0x2,
  DPAD_RIGHT_DOWN = 0x3,
  DPAD_DOWN = 0x4,
  DPAD_DOWN_LEFT = 0x5,
  DPAD_LEFT = 0x6,
  DPAD_LEFT_UP = 0x7,
  DPAD_OFF = 0x8
}ds4_dpad_state_e;

typedef struct __attribute__((packed)) {
  uint8_t padding;
  struct {
    uint8_t counter :7;
    uint8_t not_touching :1;
    uint16_t x :12;
    uint16_t y :12;
  }__attribute__((packed)) finger[2];
} ds4_tpad_info_t;

typedef struct __attribute__((packed)) {
  uint8_t hat_left_x;

  uint8_t hat_left_y;

  uint8_t hat_right_x;

  uint8_t hat_right_y;

  uint8_t dpad_code :4;
  uint8_t square :1;
  uint8_t cross :1;
  uint8_t circle :1;
  uint8_t triangle :1;

  uint8_t l1 :1;
  uint8_t r1 :1;
  uint8_t l2 :1;
  uint8_t r2 :1;
  uint8_t share :1;
  uint8_t options :1;
  uint8_t l3 :1;
  uint8_t r3 :1;

  uint8_t ps :1;
  uint8_t tpad_click :1;
  uint8_t reportCounter :6;

  uint8_t l2_trigger;

  uint8_t r2_trigger;

  uint8_t padding1[2];

  uint8_t battery;

  int16_t gyroY;

  int16_t gyroZ;

  int16_t gyroX;

  int16_t accX;

  int16_t accZ;

  int16_t accY;

  uint8_t padding2[5];

  uint8_t status;

  uint8_t padding3[3];

  ds4_tpad_info_t tpad_info[3];
} DS4_status_t;

typedef struct __attribute__((packed)){
  uint8_t rumble_small;
  uint8_t rumble_big;
  uint8_t led_r;
  uint8_t led_g;
  uint8_t led_b;
  uint8_t flash_on;
  uint8_t flash_off;
}DS4_command_t;

typedef enum{
        UP = 0,
        RIGHT = 1,
        DOWN = 2,
        LEFT = 3,
        SHARE = 4,
        OPTIONS = 5,
        L3 = 6,
        R3 = 7,
        L2 = 8,
        R2 = 9,
        L1 = 10,
        R1 = 11,
        TRIANGLE = 12,
        CIRCLE = 13,
        CROSS = 14,
        SQUARE = 15,
        PS = 16,
        TOUCHPAD = 17,
}ButtonEnum;


extern DS4_status_t DS4;
extern DS4_command_t DS4_cmd;

void DS4_Start(void);
void DS4_SetLED(uint8_t r, uint8_t g, uint8_t b);
void DS4_SetFlash(uint8_t ontime, uint8_t offtime);
void DS4_SetRumble(uint8_t small, uint8_t big);
bool DS4_IsConnected(void);
void DS4_ResetStatus(void);


#endif /* MODULES_DS4_DS4_H_ */
