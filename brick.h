#ifndef BRICK_H
#define BRICK_H

#define BRICK_TYPES_COUNT 6
#define BRICK_ROTATIONS_COUNT 4

enum BrickType: uint8_t {
  O = 0,
  L = 1,
  I = 2,
  T = 3,
  S = 4,
  Z = 5
};

struct ControlledBrick {
   MatrixCoord pos;
   BrickType type;
   uint8_t rotation;
};

struct BrickTemplate {
  uint16_t bitset;
};

/**
 R.compose(
  R.concat('0b'),
  v => v.toString(2),
  n => Number.parseInt(n, 2),
  R.join(''),
  R.map(R.reverse),
  R.reverse,
  R.split('\n'),
)
 */
BrickTemplate BRICKS[BRICK_TYPES_COUNT][BRICK_ROTATIONS_COUNT] = {
  // O
  {
    {.bitset = 0b110011},
    {.bitset = 0b110011},
    {.bitset = 0b110011},
    {.bitset = 0b110011},
  },

  // L
  {
    {.bitset = 0b1000111},
    {.bitset = 0b1100100010},
    {.bitset = 0b1110001},
    {.bitset = 0b100010011},
  },

  // I
  {
    {.bitset = 0b111},
    {.bitset = 0b100010001},
    {.bitset = 0b111},
    {.bitset = 0b100010001},
  },

  // T
  {
    {.bitset = 0b100111},
    {.bitset = 0b1000110010},
    {.bitset = 0b1110010},
    {.bitset = 0b100110001},
  },

  // S
  {
    {.bitset = 0b110110},
    {.bitset = 0b100110010},
    {.bitset = 0b110110},
    {.bitset = 0b100110010},
  },

  // Z
  {
    {.bitset = 0b1100011},
    {.bitset = 0b1000110001},
    {.bitset = 0b1100011},
    {.bitset = 0b1000110001},
  }
};

inline const BrickTemplate& pick_brick_template(const ControlledBrick& brick) {
  return BRICKS[brick.type][brick.rotation];
}

#endif
