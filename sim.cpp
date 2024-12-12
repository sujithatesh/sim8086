#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <inttypes.h>
#include <sys/types.h>

const char* registers[] = { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" };
const char* registersBYTE[] = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };
typedef enum{ AX, CX, DX, BX, SP, BP, SI, DI } Reg;

typedef struct{
    int mod;
    Reg reg;
    Reg rm;
}modregrm;
typedef struct{
    const char* inst;
    int d;
    int w;
}OpCode;

static const modregrm decodeModRegRM(unsigned char &byte){
    modregrm temp;
    int mod =	     (byte  >> 6) & 0b11; 
    Reg reg =	(Reg)((byte >> 3) & 0b111);
    Reg rm =	(Reg)(byte	  & 0b111);

    temp.mod = mod;
    temp.reg = reg;
    temp.rm = rm;

    return temp;
}

static void instructionDecode(uint8_t *buff, uint8_t size){
    uint8_t* start = buff;
    uint8_t* end = start + size;

    while(start < end){
	int instructionSize = 0;

	OpCode opCode = { NULL, 0, 0 };
	modregrm ModRegRm = { 0, AX, AX };

	if((start[0] >> 2) == 0b100010){
	    ModRegRm = decodeModRegRM(start[1]);
	    if(ModRegRm.mod == 0b11){

		opCode.inst = "mov";
		opCode.w = (start[0]) & 1;
		opCode.d = (start[0]>>1) & 1;

		if(opCode.d){
		    if(opCode.w){
			printf("%s %s, %s\n", opCode.inst, registers[ModRegRm.reg], registers[ModRegRm.rm]);
		    }
		    else{
			printf("%s %s, %s\n", opCode.inst, registersBYTE[ModRegRm.reg], registersBYTE[ModRegRm.rm]);
		    }
		    instructionSize = 2;
		}
		else{
		    if(opCode.w){
			printf("%s %s, %s\n", opCode.inst, registers[ModRegRm.rm], registers[ModRegRm.reg]);
		    }
		    else{
			printf("%s %s, %s\n", opCode.inst, registersBYTE[ModRegRm.rm], registersBYTE[ModRegRm.reg]);
		    }
		    instructionSize = 2;
		}
	    }
	}
	else if((start[0] >> 4) & 0b1011){

	    opCode.inst = "mov";
	    opCode.w = (start[0] >> 3) & 1;
	    ModRegRm.reg = (Reg)((start[0]) & 0b111);

	    int nativeInt = 0;
	    if(opCode.w){
		const long large = (start[2] << 8 | start[1]);
		const int negative = (( large >> 15) & 1) == 1;

		if (negative){
		    nativeInt = large| ~((1 << 16) - 1);
		}
		else{
		    nativeInt = large;
		}
		printf("%s %s, %d\n", opCode.inst, registers[ModRegRm.reg], nativeInt);
		instructionSize = 3;
	    }
	    else{
		const int negative = ((start[1] >> 7) & 1) == 1;

		if (negative){
		    nativeInt = start[1]| ~((1 << 8) - 1);
		}
		else{
		    nativeInt = start[1];
		}
		printf("mov %s, %d\n",  registersBYTE[ModRegRm.reg], nativeInt);
		instructionSize = 2;
	    }
	}
	start += instructionSize;
    }
}

int main (int argc, char *argv[]) {
    FILE *file = fopen("test", "rw");
    if(file == NULL){
	perror("Error opening file");
	return 1;
    }

    fseek(file, 0, SEEK_END);
    uint8_t size = ftell(file);
    rewind(file);
    uint8_t* buff = (uint8_t*)malloc(size);
    memset(buff, 0, size);
    assert(fread(buff, size, 1, file));

    fclose(file);

    instructionDecode(buff, size);

    return 0;
}
