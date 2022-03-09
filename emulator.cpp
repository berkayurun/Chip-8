#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
class chip8 {
	public:
		unsigned short opcode;
		
		unsigned char memory[4096];
		unsigned char registerFile[16];

		unsigned short I;
		unsigned short pc;

		unsigned char screen[64 * 32];

		unsigned char delay_timer;
		unsigned char sound_timer;

		unsigned short stack[16];
		unsigned short sp;

		unsigned char key[16];


		unsigned char chip8_fontset[80] =
		{
		  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		  0x20, 0x60, 0x20, 0x20, 0x70, // 1
		  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
		};

		void initialize(void);
		void emulateCycle(void);
};

void chip8::initialize()
{
	pc = 0x200;
	opcode = 0;
	I = 0;
	sp = 0;

	memset(memory, 0, 4096);
	memset(registerFile, 0, 16);
	memset(screen, 0, 64*32);

	memset(stack, 0, sizeof(short) * 16);
	memset(key, 0, 16);
	
	for(int i = 0; i < 80; i++){
		memory[i] = chip8_fontset[i];
	}

	delay_timer = 0;
	sound_timer = 0;

	//load programm into memory
	FILE *f = fopen("./programs/picture.ch8", "rb");

	if(f == NULL){
		perror("Program could not opened");
		exit(EXIT_FAILURE);
	}

	char *buff;
	size_t size;

	fseek(f, 0, SEEK_END);
	size = ftell(f);
	rewind(f);
	buff = (char *) malloc(sizeof(char) * size);

	if(buff == NULL){
		perror("Buffer could not be allocated");
		exit(EXIT_FAILURE);
	}

	fread(buff, 1, size, f);

	for(int i = 0; i < size; i++){
		memory[i + 0x200] = buff[i];
	}
}

void chip8::emulateCycle()
{
	//fetch
	opcode = memory[pc] << 8 | memory[pc + 1];

	//decode
	switch(opcode & 0xF00)
	{
		case 0xA00:
			I = opcode & 0x0FFF;
			pc += 2;
			break;
		default:
			perror("Opcode not yet implemented!");
			exit(EXIT_FAILURE);
	}

	if(delay_timer > 0)
		delay_timer--;

	if(sound_timer > 0)
		sound_timer--;
}

chip8 chip;

int main(){

	chip.initialize();
	chip.emulateCycle();

	return 0;
}
