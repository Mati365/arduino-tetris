#ifndef BOARD_H
#define BOARD_H
#include "brick.h"

#define BRICK_SIZE 3
#define BOARD_WIDTH 8
#define BOARD_HEIGHT 16

using BoardRowBitset = uint8_t;
using BoardBitset = BoardRowBitset[BOARD_HEIGHT];
  
class Board {
  private:
    BoardBitset board = {0x0};

  public:
    const BoardRowBitset* get_bitset() const { return board; }

    ControlledBrick spawn_controlled_brick(bool assign_to_board) {
      ControlledBrick brick = {
        .pos = {
          .x = 3,
          .y = 0,
        },
        .type = random(0, BRICK_TYPES_COUNT),
        .rotation = random(0, BRICK_ROTATIONS_COUNT),
      };  

      if (assign_to_board)
        assign_brick(brick);
        
      return brick;
    }
    
    #define BRICK_TEMPLATE_OPERATION(__fn_content) \
      const BrickTemplate& brick_template = pick_brick_template(brick); \
      for (uint8_t row = 0; row < BRICK_SIZE; ++row) { \
        const uint8_t template_row = (brick_template.bitset >> (row << 2)) & 0xF; \
        const uint8_t row_index = row + brick.pos.y; \
        const uint16_t row_bits = board[row_index]; \
        __fn_content \
      } 
    
    void remove_brick(const ControlledBrick& brick) {
      BRICK_TEMPLATE_OPERATION(
        board[row_index] = row_bits & ~(template_row << brick.pos.x);
      )
    }
    
    void assign_brick(const ControlledBrick& brick) {
      BRICK_TEMPLATE_OPERATION(
        board[row_index] = row_bits | (template_row << brick.pos.x);
      )
    }

    bool check_brick_collisions(ControlledBrick& brick, const MatrixCoord& new_pos) const {
      // collision with left board corner
      const BrickTemplate& brick_template = pick_brick_template(brick);
      
      for (uint8_t row = 0; row < BRICK_SIZE; ++row) {
        const uint8_t row_index = row + new_pos.y;
        const uint8_t template_row = (brick_template.bitset >> (row << 2)) & 0xFF;
        const uint16_t board_row_bits = board[row_index];

        for (uint8_t col = 0; col < BRICK_SIZE; ++col) {
          const int8_t col_index = col + new_pos.x;
          const uint8_t template_bit = bitRead(template_row, col);

          // collision with map borders
          if (template_bit == 1 && (col_index < 0 || col_index >= BOARD_WIDTH || row_index >= BOARD_HEIGHT))
            return true;

          // collision with other bricks
          if (template_bit && bitRead(board_row_bits, col_index))
            return true;
        }
      }
 
      return false; 
    }

    bool rotate_brick(ControlledBrick& brick) {
      remove_brick(brick);

      uint8_t prev_rotation = brick.rotation;
      brick.rotation = (brick.rotation + 1) % BRICK_ROTATIONS_COUNT;

      boolean brick_collision = check_brick_collisions(brick, brick.pos);
      if (brick_collision)
        brick.rotation = prev_rotation;

      assign_brick(brick);
      return !brick_collision;
    }

    uint16_t get_collapse_rows() {
      uint16_t bitset = 0;
      
      for (uint8_t row = 0; row < BOARD_HEIGHT; ++row) {
        if (board[row] == 0xFF)
          bitset = bitSet(bitset, row);
      }

      return bitset;
    }

    void collapse_rows(uint16_t rows) {
      for (uint8_t row = 0; row < BOARD_HEIGHT; ++row) {
        if (!bitRead(rows, row))
          continue;

        board[0] = 0x0;
        for (uint8_t i = row; i > 0; --i)
          board[i] = board[i - 1];
      } 
    }
    
    bool move_brick(const Direction& dir, ControlledBrick& brick) {
      MatrixCoord new_pos = brick.pos;

      // perform move
      switch (dir) {
        case Direction::TOP:    new_pos.y--; break;
        case Direction::BOTTOM: new_pos.y++; break;
        case Direction::LEFT:   new_pos.x--; break;
        case Direction::RIGHT:  new_pos.x++; break;
      };

      // remove brick with PREVIOUS pos
      remove_brick(brick);

      bool collisions = check_brick_collisions(brick, new_pos);
      if (collisions) {
        assign_brick(brick);
        return true;
      }
      
      // assign brick with NEW pos
      brick.pos = new_pos;
      assign_brick(brick);
      return false;
    }
};

#endif
