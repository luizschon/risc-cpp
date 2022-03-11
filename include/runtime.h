#ifndef __RUNTIME_H__
#define __RUNTIME_H__

#include <inttypes.h>
#include <iostream>

#define MEM_SIZE 4096
#define DATA_SEGMENT_START 0x2000
#define TEXT_SEGMENT_SIZE 0x2000
#define DATA_SEGMENT_SIZE (0x2ffc - 0x2000)

enum OPCODE {
  LoadType = 0x03,						  // Load type
  ILAType = 0x13,						    // logico-aritmeticas com imediato
  LUI = 0x37,		AUIPC = 0x17,		// atribui 20 msbits
  BranchType = 0x63,						// branch condicional
  JAL = 0x6F,		JALR = 0x67,		// jumps
  StoreType = 0x23,					    // store
  RegType = 0x33,
  ECALL = 0x73
};

enum FORMATS { RType, IType, SType, SBType, UType, UJType };

enum FUNCT3 {
  BEQ3=0x0,		  BNE3=0x01,	BLT3=0x04,	BGE3=0x05,	BLTU3=0x06, BGEU3=0x07,
  LB3=0x0,		  LH3=0x01,		LW3=0x02,		LBU3=0x04,	LHU3=0x05,
  SB3=0x0,		  SH3=0x01,		SW3=0x02,
  ADDSUB3=0x0,	SLL3=0x01,	SLT3=0x02,	SLTU3=0x03,
  XOR3=0x04,	  SR3=0x05,		OR3=0x06,		AND3=0x07,
  ADDI3=0x0,	  ORI3=0x06,	SLTI3=0x02,	XORI3=0x04,	ANDI3=0x07,
  SLTIU3=0x03,  SLLI3=0x01,	SRI3=0x05
};

enum FUNCT7 {
  ADD7=0x00,	SUB7=0x20,	SRA7=0x20,	SRL7=0x00, SRLI7=0x00,	SRAI7=0x20
};

enum INSTRUCTIONS {
  I_add, I_addi,  I_and,  I_andi, I_auipc,
  I_beq, I_bge,	  I_bgeu, I_blt,  I_bltu,
  I_bne, I_jal,	  I_jalr, I_lb,	  I_lbu,
  I_lw,  I_lh,    I_lhu,  I_lui,  I_sb,
  I_sh,  I_sw,    I_sll,  I_slt,  I_slli,
  I_srl, I_sra,   I_sub,  I_slti, I_sltiu,
  I_xor, I_or,	  I_srli, I_srai, I_sltu,
  I_ori, I_ecall, I_xori
};

enum REGISTERS {
  ZERO=0, RA=1,	  SP=2,	  GP=3,
  TP=4,	  T0=5,	  T1=6,	  T2=7,
  S0=8,	  S1=9,	  A0=10,	A1=11,
  A2=12,	A3=13,	A4=14,	A5=15,
  A6=16,	A7=17,  S2=18,	S3=19,
  S4=20,	S5=21, 	S6=22,	S7=23,
  S8=24,	S9=25,  S10=26,	S11=27,
  T3=28,	T4=29,	T5=30,	T6=31
};

typedef int32_t IMMEDIATE;

typedef struct instruction_context_st {
  INSTRUCTIONS ins_code;
  FORMATS ins_format;
  IMMEDIATE imm32_bits;
  FUNCT3 funct3;
  FUNCT7 funct7;

} instruction_context_st;


void init();
void load_mem(const char* text_file, const char* data_file);
void fetch ();
void decode ();
void execute (); 
void step();
void run();
void dump_reg(char format);
void dump_mem(int start_byte, int end_byte, char format);

int32_t get_field(int32_t word, int32_t start_index, int32_t mask);
int32_t get_bit(int32_t word, int32_t start_index);
int32_t generate_int32_imm(int32_t word, FORMATS ins_format);

#endif
