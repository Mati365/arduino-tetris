#include "matrix.h"
#include "types.h"
#include "board.h"

#define DATA_PIN 2
#define CLOCK_PIN 4
#define LATCH_PIN 3

#define BUTTON1_PIN 5
#define BUTTON2_PIN 6
#define BUTTON3_PIN 7
#define BUTTON4_PIN 8

#define UPDATE_DELAY 400
#define CLEAR_ROW_BLINK_COUNT 4

const bool is_click_button(int8_t pin) {
  static int16_t active_buttons = 0;
  
  const boolean prev_pressed = bitRead(active_buttons, pin) == 1;
  const boolean pressed = digitalRead(pin) == HIGH;

  if (prev_pressed != pressed) {
    active_buttons = (
      pressed 
        ? bitSet(active_buttons, pin)
        : bitClear(active_buttons, pin)
    ); 
    
    return pressed;
  }

  return false;
}

Board game_board; 
ControlledBrick controlled_brick;
boolean reset_anim = false;
boolean pause = false;
uint16_t blink_collapse_rows_anim = 0x0;

void render_board() {
  const BoardRowBitset* bitset = game_board.get_bitset();
  
  for (uint8_t i = 0; i < MATRIX_SIZE; ++i) {
    union MatrixDescriptor MATRIX_1 = convert_to_matrix(bitset, i);
    union MatrixDescriptor MATRIX_2 = convert_to_matrix(&bitset[MATRIX_SIZE], i);

    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, MATRIX_2.reg.bottom);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, MATRIX_1.reg.bottom);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, MATRIX_1.reg.upper);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, MATRIX_2.reg.upper);
    digitalWrite(LATCH_PIN, HIGH);
  }
}

void update_anims() {
  BoardRowBitset* board_bitset = game_board.get_bitset();

  if (blink_collapse_rows_anim != 0x0) {
    static int8_t blink_count = CLEAR_ROW_BLINK_COUNT;

    if (blink_count-- >= 0) {
      for (uint8_t i = 0; i < BOARD_HEIGHT; ++i) {
        if (bitRead(blink_collapse_rows_anim, i))
          board_bitset[i] = ~board_bitset[i];
      }
      
    } else {
      game_board.collapse_rows(blink_collapse_rows_anim);

      blink_count = CLEAR_ROW_BLINK_COUNT;
      blink_collapse_rows_anim = 0x0;
    }
  }
  
  if (reset_anim) {
    static int8_t scan_line = BOARD_HEIGHT * 2; 
      
    // fill with 1
    if (scan_line < 0) {
      scan_line = BOARD_HEIGHT * 2;
      reset_anim = false;
    } else if (scan_line >= BOARD_HEIGHT)
      board_bitset[scan_line - BOARD_HEIGHT] = 0xFF;
    else
      board_bitset[scan_line] = 0x0;

    scan_line--;
  }
}

void update_board() {
  const boolean collision = game_board.move_brick(Direction::BOTTOM, controlled_brick);

  if (collision) {
    controlled_brick = game_board.spawn_controlled_brick(false);
    blink_collapse_rows_anim = game_board.get_collapse_rows();
    
    // LOSE "ANIM" if newly resp brick has collisions
    if (game_board.check_brick_collisions(controlled_brick, controlled_brick.pos))
      reset_anim = true;
    else if (blink_collapse_rows_anim == 0x0)
      game_board.assign_brick(controlled_brick);
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(BUTTON1_PIN, INPUT);
  pinMode(BUTTON2_PIN, INPUT);

  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);

  controlled_brick = game_board.spawn_controlled_brick(true);
}

void loop() {  
  static unsigned long prev_update_time = -1;
  unsigned long current_time = millis();

  if (is_click_button(BUTTON4_PIN))
    pause = !pause;
      
  if (!pause) {
    if (!reset_anim && !blink_collapse_rows_anim) {
      if (current_time - prev_update_time >= UPDATE_DELAY) {
        if (prev_update_time != -1)
          update_board();
      
        prev_update_time = current_time;
      }
        
      if (is_click_button(BUTTON1_PIN))
        game_board.move_brick(Direction::LEFT, controlled_brick);
      
      if (is_click_button(BUTTON2_PIN))
        game_board.move_brick(Direction::RIGHT, controlled_brick);
      
      if (is_click_button(BUTTON3_PIN))
        game_board.rotate_brick(controlled_brick);   
    } else {
      if (current_time - prev_update_time >= 100) {
        if (prev_update_time != -1)
          update_anims();
      
        prev_update_time = current_time;
      }
    }
  }
     
  render_board();
}
