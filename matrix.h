#ifndef MATRIX_H
#define MATRIX_H

#define MATRIX_SIZE 8

struct MatrixCoord {
  int8_t x;
  int8_t y;

  MatrixCoord() : x(0), y(0) {};
  MatrixCoord(int8_t _x, int8_t _y) : x(_x), y(_y) {};
};

union MatrixDescriptor {
  struct {
    uint8_t upper: 8;
    uint8_t bottom: 8;
  } reg;  

  uint16_t value;
};

const union MatrixDescriptor convert_to_matrix(const uint8_t rows[], uint8_t col) {
  static uint8_t COLS_SHIFT[] = {
    // 1, 2, 3, 4, 5, 6, 7, 8
    4, 5, 6, 7, 15, 14, 13, 12,
  };

  static uint8_t ROWS_SHIFT[] = {
    // 1, 2, 3, 4, 5, 6, 7, 8
    0, 1, 2, 3, 11, 10, 9, 8,
  };
  
  uint16_t value = (uint16_t) 0x0;  

  for (uint8_t row = 0; row < MATRIX_SIZE; ++row) {
    uint8_t bits = rows[row];
    uint16_t row_shift = 1 << ROWS_SHIFT[row];
    
    if ((bits >> col & 0x1) == 0x1)
      value = (value & ~row_shift) | (1 << COLS_SHIFT[col]);
    else
      value |= row_shift;   
  }
  
  return (MatrixDescriptor) {
    .value = value,
  };
}

#endif
