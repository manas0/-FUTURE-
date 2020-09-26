#include <stdio.h>
#include <stdint.h>

#define KEY_SIZE 4
#define STATE_SIZE 4
#define SBOX_SIZE 16
#define MAX_ROUNDS 11

#define AES_BLOCK_SIZE      16
#define AES_ROUNDS          10  // 12, 14
#define AES_ROUND_KEY_SIZE  176 // AES-128 has 10 rounds, and there is a AddRoundKey before first round. (10+1)x16=176.

#define SBOX_LOOKUP(lookup_box, num) lookup_box[(num) >> 4][(num) & 0x0F];


void addRoundKey(int round) {
    for(int r = 0; r < STATE_SIZE; r++) {
        for (int c = 0; c < STATE_SIZE; c++) {
            state[r][c] ^= sub_keys[round][r][c];
        }
    }
}

void subCell(uint8_t sbox_to_use[SBOX_SIZE][SBOX_SIZE]) {
    for(int r = 0; r < STATE_SIZE; r++) {
        for (int c = 0; c < STATE_SIZE; c++) {
            state[r][c] = SBOX_LOOKUP(sbox_to_use, state[r][c]);
        }
    }
}

void helperShiftRow(int row) {
    // shift row by 1 byte
    uint8_t temp = state[row][0];
    state[row][0] = state[row][1];
    state[row][1] = state[row][2];
    state[row][2] = state[row][3];
    state[row][3] = temp;
}

void shiftRows() {
    helperShiftRow(1);
    helperShiftRow(2);
    helperShiftRow(2);
    helperShiftRow(3);
    helperShiftRow(3);
    helperShiftRow(3);
}

void invShiftRows() {
    helperShiftRow(1);
    helperShiftRow(1);
    helperShiftRow(1);
    helperShiftRow(2);
    helperShiftRow(2);
    helperShiftRow(3);
}