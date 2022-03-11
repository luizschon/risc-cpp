#include "../include/riscv.h"
#include "../include/runtime.h"
#include <cstdlib>

extern int32_t mem[MEM_SIZE];
extern int32_t breg[32];  // Extern global register bank to read a7 and a0 values in ecall funct
extern bool stop_flag;

using namespace std;

void sw(uint32_t address, int32_t kte, int32_t dado) {
  // Ensures address is aligned for correct word storage
  if ((address + kte) % 4 != 0) {
    cout << "sw Error: Word address not aligned properly!" << endl;
    return;
  }

  // Calculates proper word address by dividing the address by 4.
  // Remember each index of the mem array represents a 32 bit word and 
  // the address and kte parameters are represented by bytes.
  uint32_t word_addr = (address + kte)/4;

  mem[word_addr] = dado;
}

void sb(uint32_t address, int32_t kte, int8_t dado) {
  uint32_t byte_addr = address + kte;

  // Create pointer to access memory byte per byte
  int8_t* byte_p = (int8_t*) mem;

  byte_p[byte_addr] = dado;
}

void sh(uint32_t address, int32_t kte, int8_t dado) {
  uint32_t word_addr = (address + kte)/4;

  mem[word_addr] = get_field(dado, 0, 0xFFFF);
}

int32_t lw(uint32_t address, int32_t kte) {
  // Ensures address is aligned to correctly read the word 
  if ((address + kte) % 4 != 0) {
    cout << "lw Error: Word address not aligned properly!" << endl;
    return 0;
  }

  uint32_t word_addr = (address + kte)/4;

  return mem[word_addr];
}

int32_t lb(uint32_t address, int32_t kte) {
  uint32_t byte_addr = address + kte;

  // Create pointer to access memory byte per byte
  int8_t* byte_p = (int8_t*) mem;

  uint8_t byte_value = byte_p[byte_addr];

  // Extends byte value to its 32 bit signed representation
  // and returns the resulting value
  return sext(byte_value);
}

int32_t lbu(uint32_t address, int32_t kte) {
  uint32_t byte_addr = address + kte;

  // Creates pointer to access memory byte per byte
  int8_t* byte_p = (int8_t*) mem;

  uint8_t byte_value = byte_p[byte_addr]; 

  // Extends byte value to its 32 bits unsigned representation
  // by filling it with zeros
  return uext(byte_value);
}

int32_t lh(uint32_t address, int32_t kte) {
  int32_t word = mem[(address + kte)/4];
  return (get_field(word, 0, 0xFFFF) << 16) >> 16;
}

int32_t lhu(uint32_t address, int32_t kte) {
  int32_t word = mem[(address + kte)/4];
  return get_field(word, 0, 0xFFFF);
}

void ecall() {
  switch (breg[A7]) {
    case 1:
      cout << (int32_t) breg[A0];
      break;
    case 4:
      {
        char* p = (char*) mem;
        int32_t char_index = breg[A0];

        while (p[char_index] != '\0') {
          cout << p[char_index];
          char_index++;
        } 
      }
      break;
    case 10:
      stop_flag = true;
      break;
  }
}

int32_t sext(uint8_t byte) {
  uint32_t mask;
  uint32_t ones = ~0;
  uint32_t zeros = 0;

  // If the byte's MSB is 1 (aka >= 10000000) then
  // the number is negative and we want to extend it with ones,
  // otherwise, we wanto to extend it with zeros.
  if (byte >= 0x80)
    mask = ones << 8; // mask = 0xFFFFFFF0
  else 
    mask = zeros; // mask = 0x00000000
  
  // Performs a bitwise OR operation to add the loaded byte value 
  // inside the 32 bit mask and returns it
  return mask | byte;  
}

int32_t uext(uint8_t byte) {
  uint32_t zeros = 0;

  // Performs bitwise OR operation to add the loaded byte value
  // inside the 32 bit number and returns it
  return zeros | byte;
}
