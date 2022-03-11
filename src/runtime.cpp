#include <iostream>
#include <fstream>
#include <string>

#include "../include/runtime.h"
#include "../include/riscv.h"

int32_t mem[MEM_SIZE];

int32_t pc = 0x00000000;
int32_t ri = 0x00000000;
int32_t sp = 0x00003ffc;
int32_t gp = 0x00001800;

OPCODE opcode;
FUNCT3 funct3;
FUNCT7 funct7;
int32_t rs1, rs2, rd, shamt;

int32_t breg[32];
instruction_context_st ic;
bool stop_flag = false;
bool verbose = false;

using namespace std;

int32_t get_field(int32_t word, int32_t start_index, int32_t mask) {
   return word & (mask << start_index);
}

int32_t get_bit(int32_t word, int32_t start_index) {
   return word & (1 << start_index);
}

int32_t generate_int32_imm(int32_t word, FORMATS ins_format) {
  switch(ins_format) {
    case IType:
      // Get [11:0] bits from RI and sign-extend by shifting
      return get_field(word, 20, 0xFFF) >> 20;
    case SType:
      // Get [11:5] and [4:0] bits from RI and sign-extend
      return 
          get_field(word, 7, 0x1F) >> 7 | get_field(word, 25, 0x7F) >> 20;
    case SBType:
      // Get [12|10:5] and [4:1] bits from RI and sign-extend
      return
        get_field(word, 8, 0xF) >> 7 |
        get_field(word, 25, 0x3F) >> 20 | 
        get_bit(word, 7) << 4 | 
        get_bit(word, 31) >> 19;
    case UType:
      // Get [31:12] bits from RI
      return get_field(word, 12, 0xFFFFF);
    case UJType: 
      // Get [20|10:1|11|19:12] bits from RI and sign-extend
      return 
        get_field(word, 21, 0x3FF) >> 20 |
        get_bit(word, 20) >> 9 |
        get_field(word, 12, 0xFF) |
        get_bit(word, 31) >> 11; 
    default:
      return 0;
  }
}

void init() {
  for (int i=0; i < MEM_SIZE; i++) 
    mem[i] = 0;
}

void load_mem(const char* text_file, const char* data_file) {
  ifstream text_segment(text_file, ios::binary);
  ifstream data_segment(data_file, ios::binary);

  if (!text_segment.is_open()) {
    cout << "Error while oppening text segment file!" << endl;
    return;
  }

  if (!data_segment.is_open()) {
    cout << "Error while oppening data segment file!" << endl;
    return;
  }

  char* textblock = new char[0x2000];
  char* datablock = new char[0x2ffc - 0x2000];

  text_segment.read(textblock, 0x2000);
  data_segment.read(datablock, 0x2ffc - 0x2000);

  char* p_mem = (char*)mem;

  for (int i=0; i < TEXT_SEGMENT_SIZE; i++) {
    p_mem[i] = textblock[i];
  }

  for (int i=0; i < DATA_SEGMENT_SIZE; i++) {
    p_mem[DATA_SEGMENT_START + i] = datablock[i];
  }

  text_segment.close();
  data_segment.close();
}

void fetch() {
  ri = lw(pc, 0);
}

void decode() {
  opcode = (OPCODE) get_field(ri, 0, 0x7F);   // Get [6:0] bits of ri with mask 111111
  
  switch (opcode) {
    case LoadType:
      {
        ic.ins_format = IType;
        rs1 = (REGISTERS) (get_field(ri, 15, 0x1F) >> 15);
        rd  = (REGISTERS) (get_field(ri,  7, 0x1F) >> 7);

        funct3 = (FUNCT3) (get_field(ri, 12, 0x7) >> 12);

        switch (funct3) {
          case LB3:  ic.ins_code = I_lb;  break;
          case LH3:  ic.ins_code = I_lh;  break;
          case LW3:  ic.ins_code = I_lw;  break;
          case LBU3: ic.ins_code = I_lbu; break;
          case LHU3: ic.ins_code = I_lhu; break;
        }
      } 
      break;

    case ILAType:
      {
        ic.ins_format = IType;
        rs1 = (REGISTERS) (get_field(ri, 15, 0x1F) >> 15);
        rd  = (REGISTERS) (get_field(ri,  7, 0x1F) >> 7);

        funct3 = (FUNCT3) (get_field(ri, 12, 0x7) >> 12); 
        funct7 = (FUNCT7) (get_field(ri, 25, 0x7F) >> 25);

        switch (funct3) {
          case ADDI3:  ic.ins_code = I_addi;  break;
          case SLLI3:  ic.ins_code = I_slli;  break;
          case SLTI3:  ic.ins_code = I_slti;  break;
          case SLTIU3: ic.ins_code = I_sltiu; break;
          case XORI3:  ic.ins_code = I_xori;  break;
          case SR3:    
            {
              switch (funct7) {
                case SRLI7: ic.ins_code = I_srli; break; 
                case SRAI7: ic.ins_code = I_srai; break;
              }
            }
            break;
          case ORI3:  ic.ins_code = I_ori;  break;
          case ANDI3: ic.ins_code = I_andi; break;
        }
      }
      break;

    case AUIPC:
      ic.ins_format = UType;
      rd = (REGISTERS) (get_field(ri, 7, 0x1F) >> 7);
      ic.ins_code = I_auipc;
      break;

    case StoreType: 
      {
        ic.ins_format = SType;
        rs1 = (REGISTERS) (get_field(ri, 15, 0x1F) >> 15);
        rs2 = (REGISTERS) (get_field(ri, 20, 0x1F) >> 20);

        funct3 = (FUNCT3) (get_field(ri, 12, 0x7) >> 12);

        switch (funct3) {
          case SB3: ic.ins_code = I_sb; break;
          case SH3: ic.ins_code = I_sh; break;
          case SW3: ic.ins_code = I_sw; break;
        }
      }
      break;

    case RegType: 
      {
        ic.ins_format = RType;
        rs1 = (REGISTERS) (get_field(ri, 15, 0x1F) >> 15);
        rs2 = (REGISTERS) (get_field(ri, 20, 0x1F) >> 20);
        rd  = (REGISTERS) (get_field(ri,  7, 0x1F) >> 7);

        funct3 = (FUNCT3) (get_field(ri, 12, 0x7) >> 12);
        funct7 = (FUNCT7) (get_field(ri, 25, 0x7F) >> 25);

        switch (funct3) {
          case ADDSUB3:
            {
              switch (funct7) {
                case ADD7: ic.ins_code = I_add; break;
                case SUB7: ic.ins_code = I_sub; break;
              }
            }
          break;

          case SLL3:  ic.ins_code = I_sll;  break;
          case SLT3:  ic.ins_code = I_slt;  break;
          case SLTU3: ic.ins_code = I_sltu; break;
          case XOR3:  ic.ins_code = I_xor;  break;
          case SR3: 
            {
              switch (funct7) {
                case SRL7: ic.ins_code = I_srl; break;
                case SRA7: ic.ins_code = I_sra; break;
              }
            }
            break;
          
          case OR3:   ic.ins_code = I_or;   break;
          case AND3:  ic.ins_code = I_and;  break;
        }
      }
      break;

    case LUI:
      ic.ins_format = UType;
      rd = (REGISTERS) (get_field(ri,  7, 0x1F) >> 7);
      ic.ins_code = I_lui;
      break;

    case BranchType:
      {
        ic.ins_format = SBType;
        rs1 = (REGISTERS) (get_field(ri, 15, 0x1F) >> 15);
        rs2 = (REGISTERS) (get_field(ri, 20, 0x1F) >> 20);

        funct3 = (FUNCT3) (get_field(ri, 12, 0x7) >> 12);

        switch (funct3) {
          case BEQ3:  ic.ins_code = I_beq;  break;
          case BNE3:  ic.ins_code = I_bne;  break;
          case BLT3:  ic.ins_code = I_blt;  break;
          case BGE3:  ic.ins_code = I_bge;  break;
          case BLTU3: ic.ins_code = I_bltu; break;
          case BGEU3: ic.ins_code = I_bgeu; break;
        }
      }
      break;

    case JAL:
      ic.ins_format = UJType;
      rd = (REGISTERS) (get_field(ri,  7, 0x1F) >> 7);
      ic.ins_code = I_jal;
      break;

    case JALR:
      ic.ins_format = IType;
      rs1 = (REGISTERS) (get_field(ri, 15, 0x1F) >> 15);
      rd  = (REGISTERS) (get_field(ri,  7, 0x1F) >> 7);
      ic.ins_code = I_jalr;
      break;

    case ECALL:
      ic.ins_format = IType;
      ic.ins_code = I_ecall;
      break;
  }

  if (ic.ins_format != RType)
    ic.imm32_bits = generate_int32_imm(ri, ic.ins_format);
}

void execute() {
  switch (ic.ins_code) {
    case I_add:   
      breg[rd] = (int32_t)breg[rs1] + (int32_t)breg[rs2];
      break;
    case I_addi:  
      breg[rd] = breg[rs1] + ic.imm32_bits;
      break;
    case I_and:   
      breg[rd] = breg[rs1] & breg[rs2];
      break;
    case I_andi:  
      breg[rd] = breg[rs1] & ic.imm32_bits;
      break;
    case I_auipc: 
      breg[rd] = pc + ic.imm32_bits;
      break;
    case I_beq:   
      if (breg[rs1] == breg[rs2]) pc += ic.imm32_bits - 4;  // Subtracts 4 because the pc is incremented in the end off run()
      break;
    case I_bge:   
      if (breg[rs1] >= breg[rs2]) pc += ic.imm32_bits - 4;  // Subtracts 4 because the pc is incremented in the end off run()
      break;
    case I_bgeu:  
      if ((uint32_t)breg[rs1] >= (uint32_t)breg[rs2]) pc += ic.imm32_bits;
      break;
    case I_blt:   
      if (breg[rs1] < breg[rs2]) pc += ic.imm32_bits;
      break;
    case I_bltu:  
      if ((uint32_t)breg[rs1] < (uint32_t)breg[rs2]) pc += ic.imm32_bits;
      break;
    case I_bne:   
      if (breg[rs1] != breg[rs2]) pc += ic.imm32_bits;
      break;
    case I_jal:   
      breg[rd] = pc + 4;
      pc += ic.imm32_bits - 4;  // Subtracts 4 because the pc is incremented in the end off run() 
      break;
    case I_jalr:   
      breg[rd] = pc + 4;
      pc = ((breg[rs1] + ic.imm32_bits) & ~1) - 4;  // Subtracts 4 because the pc is incremented in the end off run() 
      break;
    case I_lb:   
      breg[rd] = lb((uint32_t) breg[rs1], ic.imm32_bits);
      break;
    case I_lbu:   
      breg[rd] = lbu((uint32_t) breg[rs1], ic.imm32_bits);
      break;
    case I_lh:   
      lh((uint32_t) breg[rs1], ic.imm32_bits);
      break;
    case I_lhu:   
      lhu((uint32_t) breg[rs1], ic.imm32_bits);
      break;
    case I_lui:   
      breg[rd] = ic.imm32_bits;
      break;
    case I_lw:   
      breg[rd] = lw((uint32_t)breg[rs1], ic.imm32_bits);
      break;
    case I_or:   
      breg[rd] = breg[rs1] | breg[rs2];
      break;
    case I_ori:   
      breg[rd] = breg[rs1] | ic.imm32_bits;
      break;
    case I_sb:   
      sb((uint32_t) breg[rs1], ic.imm32_bits, breg[rs2]);
      break;
    case I_sh:   
      sh((uint32_t) breg[rs1], ic.imm32_bits, breg[rs2]);
      break;
    case I_sll:   
      breg[rd] = breg[rs1] << breg[rs2];
      break;
    case I_slli:   
      breg[rd] = breg[rs1] << ic.imm32_bits;
      break;
    case I_slt:   
      breg[rd] = breg[rs1] < breg[rs2];
      break;
    case I_slti:   
      breg[rd] = breg[rs1] < ic.imm32_bits;
      break;
    case I_sltiu:   
      breg[rd] = (uint32_t)breg[rs1] < (uint32_t)ic.imm32_bits;
      break;
    case I_sltu:   
      breg[rd] = (uint32_t)breg[rs1] < (uint32_t)breg[rs2];
      break;
    case I_sra:   
      breg[rd] = breg[rs1] >> breg[rs2];
      break;
    case I_srai:   
      breg[rd] = breg[rs1] >> ic.imm32_bits;
      break;
    case I_srl:  
      // The implementation of the right shift operator is compiler specific.
      // The g++ implementation uses arithmetical shifts if the type of the operand
      // is int32_t and uses logical shifts if the type is not specified as a signed int (int32_t)
      // as our breg in an int32_t array, a simple cast should trick the compiler into
      // performing logical shifts
      breg[rd] = (uint32_t) breg[rs1] >> (uint32_t) breg[rs2];
      break;
    case I_srli:   
      breg[rd] = (uint32_t) breg[rs1] >> (uint32_t) ic.imm32_bits;
      break;
    case I_sub:   
      breg[rd] = breg[rs1] - breg[rs2];
      break;
    case I_sw:   
      sw((uint32_t) breg[rs1], ic.imm32_bits, breg[rs2]);
      break;
    case I_xor:   
      breg[rd] = breg[rs1] ^ breg[rs2];
      break;
    case I_xori:   
      breg[rd] = breg[rs1] ^ ic.imm32_bits;
      break;
    case I_ecall:   
      ecall();
      break;
  }
}

void run() {
  // Initialize stack and global pointers
  breg[SP] = sp;
  breg[GP] = gp;

  while (!stop_flag) {
    breg[ZERO] = 0; // Ensures zero reg is set to 0
    fetch();
    decode();
    execute();
    pc = pc + 4;
  }
}

void dump_reg(char format) {
  for (int i=0; i<32; i++) {
    if (format == 'h')
      printf("x%02d:  0x%08x\n", i, breg[i]);
    else if (format == 'd')
      printf("x%02d:  %d\n", i, breg[i]);
  }
  cout << endl;
}

void dump_mem(int start_byte, int end_byte, char format) {
  uint8_t* p = (uint8_t*) mem;
  
  for (int i=start_byte; i <= end_byte; i++) {
    if (format == 'h') 
      printf("0x%04x:  0x%02x\n", i, p[i]);
    else if (format == 'd')
      printf("0x%04x:  %d\n", i, p[i]);
  }
  cout << endl;
}
