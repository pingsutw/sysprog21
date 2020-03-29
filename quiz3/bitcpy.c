#include <stdint.h>
#include <stdio.h>
#include <string.h>

void bitcpy(void *_dest,      /* Address of the buffer to write to */
            size_t _write,    /* Bit offset to start writing to */
            const void *_src, /* Address of the buffer to read from */
            size_t _read,     /* Bit offset to start reading from */
            size_t _count) {
  uint8_t data, original, mask;
  size_t bitsize;
  size_t read_lhs = _read & 7;
  size_t read_rhs = 8 - read_lhs;
  const uint8_t *source = _src + (_read / 8);
  size_t write_lhs = _write & 7;
  size_t write_rhs = 8 - write_lhs;
  uint8_t *dest = _dest + (_write / 8);

  static const uint8_t read_mask[] = {
      0x00, /*	== 0	00000000b	*/
      0x80, /*	== 1	10000000b	*/
      0xC0, /*	== 2	11000000b	*/
      0xE0, /*	== 3	11100000b	*/
      0xF0, /*	== 4	11110000b	*/
      0xF8, /*	== 5	11111000b	*/
      0xFC, /*	== 6	11111100b	*/
      0xFE, /*	== 7	11111110b	*/
      0xFF  /*	== 8	11111111b	*/
  };

  static const uint8_t write_mask[] = {
      0xFF, /*	== 0	11111111b	*/
      0x7F, /*	== 1	01111111b	*/
      0x3F, /*	== 2	00111111b	*/
      0x1F, /*	== 3	00011111b	*/
      0x0F, /*	== 4	00001111b	*/
      0x07, /*	== 5	00000111b	*/
      0x03, /*	== 6	00000011b	*/
      0x01, /*	== 7	00000001b	*/
      0x00  /*	== 8	00000000b	*/
  };

  while (_count > 0) {
    data = *source++;
    bitsize = (_count > 8) ? 8 : _count;
    if (read_lhs > 0) {
      data <<= read_lhs;
      if (bitsize > read_rhs)
        data |= (*source >> read_rhs);
    }

    if (bitsize < 8)
      data &= read_mask[bitsize];

    original = *dest;
    if (write_lhs > 0) {
      mask = read_mask[write_lhs];
      if (bitsize > write_rhs) {
        *dest++ = (original & mask) | (data >> write_lhs);
        original = *dest & write_mask[bitsize - write_rhs];
        *dest = original | (data << write_rhs);
      } else {
        if ((bitsize - write_lhs) > 0)
          mask = mask | write_mask[8 - (bitsize - write_lhs)];
        *dest++ = (original & mask) | (data >> write_lhs);
      }
    } else {
      if (bitsize < 8)
        data = data | (original & write_mask[bitsize]);
      *dest++ = data;
    }

    _count -= bitsize;
  }
}

static uint8_t output[8], input[8];

static inline void dump_8bits(uint8_t _data) {
  for (int i = 0; i < 8; ++i)
    printf("%d", (_data & (0x80 >> i)) ? 1 : 0);
}
static inline void dump_binary(uint8_t *_buffer, size_t _length) {
  for (int i = 0; i < _length; ++i)
    dump_8bits(*_buffer++);
}

int main(int _argc, char **_argv) {
  memset(&input[0], 0xFF, sizeof(input));

  for (int i = 1; i <= 32; ++i) {
    for (int j = 0; j < 16; ++j) {
      for (int k = 0; k < 16; ++k) {
        memset(&output[0], 0x00, sizeof(output));
        printf("%2d:%2d:%2d ", i, k, j);
        bitcpy(&output[0], k, &input[0], j, i);
        dump_binary(&output[0], 8);
        printf("\n");
      }
    }
  }

  return 0;
}
