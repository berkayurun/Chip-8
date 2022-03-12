#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <raylib.h>

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
		bool drawFlag = false;

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
		void loadProgram(void);
		void drawGraphics(void);
		void setKeys(void);
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
}

void chip8::loadProgram()
{
	//load programm into memory
	FILE *f = fopen("./programs/BRIX", "rb");
	//FILE *f = fopen("./chip8-test-rom/test_opcode.ch8", "rb");

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

	for(size_t i = 0; i < size; i++){
		memory[i + 0x200] = buff[i];
	}

}

void chip8::emulateCycle()
{
	//fetch
	opcode = memory[pc] << 8 | memory[pc + 1];

	//printf("Opcode: %x\n", opcode);
	//decode
	switch(opcode & 0xF000)
	{
		case 0x0000:
		{
			if((opcode & 0x0F00) != 0){
				//CALL
				//FIXME
				short nnn = opcode & 0x0FFF;
				pc = nnn;
			} else if((opcode & 0x000F) != 0xE){
				//Display
				for(int i = 0; i < 64 * 32; i++)
					screen[i] = 0;
				pc += 2;
				drawFlag = false;
				break;
			} else {
				//Return
				sp--;
				pc = stack[sp];
				pc += 2;
			}
			break;
		}
		case 0xA000:
		{
			I = opcode & 0x0FFF;
			pc += 2;
			break;
		}
		case 0x1000:
		{
			//printf("In 0x1000\n");
			//printf("pc: %x\n", pc);
			//if(pc  == (opcode & 0x0FFF))
			//	exit(EXIT_FAILURE);
			pc = opcode & 0x0FFF;
			break;
		}
		case 0x2000:
		{
			short nnn = opcode & 0x0FFF;
			stack[sp] = pc;
			sp++;
			pc = nnn;
			break;
		}
		case 0x3000:
		{
			short x = (opcode & 0x0F00) >> 8;
			short nn = opcode & 0x00FF;
			
			if(registerFile[x] == nn)
				pc += 4;
			else
				pc += 2;
			break;
		}
		case 0x4000:
		{
			short x = (opcode & 0x0F00) >> 8;
			short nn = opcode & 0x00FF;
			
			if(registerFile[x] != nn)
				pc += 4;
			else
				pc += 2;
			break;
		}
		case 0x5000:
		{
			short x = (opcode & 0x0F00) >> 8;
			short y = (opcode & 0x00F0) >> 4;
			
			if(registerFile[x] == registerFile[y])
				pc += 4;
			else
				pc += 2;
			break;
		}
		case 0x6000:
		{
			short x = (opcode & 0x0F00) >> 8;
			short nn = opcode & 0x00FF;

			registerFile[x] = nn;
			pc += 2;
			break;
		}	
		case 0x7000:
		{
			short x = (opcode & 0x0F00) >> 8;
			short nn = opcode & 0x00FF;

			registerFile[x] += nn;
			pc += 2;
			break;
		}	
		case 0x8000:
		{
			short x = (opcode & 0x0F00) >> 8;
			short y = (opcode & 0x00F0) >> 4;

			switch(opcode & 0xF){
				case 0x0:
				{
					registerFile[x] = registerFile[y];
					pc += 2;	
					break;	
				}
				case 0x1:
				{
					registerFile[x] = registerFile[x] | registerFile[y];
					pc += 2;
					break;
				}
				case 0x2:
				{
					registerFile[x] = registerFile[x] & registerFile[y];
					pc += 2;
					break;
					
				}
				case 0x3:
				{
					registerFile[x] = registerFile[x] ^ registerFile[y];
					pc += 2;
					break;
					
				}
				case 0x4:
				{
					if(registerFile[y] + registerFile[x] > 255)
						registerFile[15] = 1;
					else
						registerFile[15] = 0;

					registerFile[x] += registerFile[y];
					pc += 2;
					break;
				}
				case 0x5:
				{
					if(registerFile[x] > registerFile[y])
						registerFile[15] = 1;
					else
						registerFile[15] = 0;

					registerFile[x] -= registerFile[y];
					pc += 2;
					break;
				}
				case 0x6:
				{
					registerFile[15] = registerFile[x] & 0x0001;
					registerFile[x] = registerFile[x] >> 1;
					pc += 2;
					break;					
				}
				case 0x7:
				{
					if(registerFile[y] > registerFile[x])
						registerFile[15] = 1;
					else
						registerFile[15] = 0;

					registerFile[x] = registerFile[y] - registerFile[x];
					pc += 2;
					break;
				}
				case 0xE:
				{
					registerFile[15] = (registerFile[x] & 0x80) >> 7;
					registerFile[x] = registerFile[x] << 1;
					pc += 2;
					break;					
				}
			}
			break;
		}	
		case 0x9000:
		{
			short x = (opcode & 0x0F00) >> 8;
			short y = (opcode & 0x00F0) >> 4;
			
			if(registerFile[x] != registerFile[y])
				pc += 4;
			else
				pc += 2;
			break;
		}	
		case 0xC000:
		{
			short x = (opcode & 0x0F00) >> 8;
			short nn = (opcode & 0x00FF);
			
			registerFile[x] = nn & (rand() % 256);

			pc += 2;
			break;
		}	
		case 0xD000:		   
		{
		  unsigned short x = (registerFile[(opcode & 0x0F00) >> 8]) % 64;
		  unsigned short y = (registerFile[(opcode & 0x00F0) >> 4]) % 32;
		  unsigned short height = opcode & 0x000F;
		  unsigned short pixel;

		  registerFile[0xF] = 0;
		  for (int yline = 0; yline < height; yline++)
		  {
		    pixel = memory[I + yline];
		    for(int xline = 0; xline < 8; xline++)
		    {
		      if((pixel & (0x80 >> xline)) != 0)
		      {
			if(screen[(x + xline + ((y + yline) * 64))] == 1)
			  registerFile[0xF] = 1;                                 
			screen[x + xline + ((y + yline) * 64)] ^= 1;
		      }
		    }
		  }

		  drawFlag = true;
		  pc += 2;
		  break;
		}
		case 0xE000:
		{
			short x = (opcode & 0x0F00) >> 8;
			switch(opcode & 0x000F){
				case 0x000E:
				{
					if(key[registerFile[x]] == 1)
						pc += 4;
					else
						pc += 2;
					break;	
				}
				case 0x0001:
				{
					if(key[registerFile[x]] != 1)
						pc += 4;
					else
						pc += 2;
					break;	
					
				}
			}
		}
		case 0xF000:
		{
			short x = (opcode & 0x0F00) >> 8;

			switch(opcode & 0x00FF){
				case 0x0007:
				{
					registerFile[x] = delay_timer;
					pc += 2;
					break;
				}
				case 0x000A:
				{
					pc += 2;

					if(key[0]) registerFile[x] = 0;
					else if(key[1]) registerFile[x] = 1;
					else if(key[2]) registerFile[x] = 2;
					else if(key[3]) registerFile[x] = 3;
					else if(key[4]) registerFile[x] = 4;
					else if(key[5]) registerFile[x] = 5;
					else if(key[6]) registerFile[x] = 6;
					else if(key[7]) registerFile[x] = 7;
					else if(key[8]) registerFile[x] = 8;
					else if(key[9]) registerFile[x] = 9;
					else if(key[10]) registerFile[x] = 10;
					else if(key[11]) registerFile[x] = 11;
					else if(key[12]) registerFile[x] = 12;
					else if(key[13]) registerFile[x] = 13;
					else if(key[14]) registerFile[x] = 14;
					else if(key[15]) registerFile[x] = 15;
					else pc -= 2;
				}
				case 0x0015:
				{
					delay_timer = registerFile[x];
					pc += 2;
					break;
				}
				case 0x0018:
				{
					sound_timer = registerFile[x];
					pc += 2;
					break;
				}
				case 0x001E:
				{
					I += registerFile[x];
					pc += 2;
					break;
				}
				case 0x0029:
				{
					//FIXME
					I = registerFile[x]*5;
					pc += 2;
					break;
				}
				case 0x0033:
				{
					memory[I] = registerFile[x] / 100;
					memory[I + 1] = (registerFile[x] % 100) / 10;
					memory[I + 2] = (registerFile[x] % 10);
					pc += 2;
					break;
				}
				case 0x0055:
				{
					for(int i = 0; i <= x; i++){
						memory[I + i] = registerFile[i];
					}
					pc += 2;
					break;
				}
				case 0x0065:
				{
					for(int i = 0; i <= x; i++){
						registerFile[i] = memory[I + i];
					}
					pc += 2;
					break;
				}
			}
			break;
		}
		default:
			perror("Opcode not avaliable!");
			exit(EXIT_FAILURE);
	}

	if(delay_timer > 0)
		delay_timer--;

	if(sound_timer > 0)
		sound_timer--;
}

void chip8::drawGraphics(){
	ClearBackground(BLACK);
	for(int i = 0; i < 32; i++){
		for(int j = 0; j < 64; j++){
			if(screen[j + (64 * i)] != 0)
				DrawRectangle(j * 10, i * 10, 10, 10, GREEN);
		}
	}
	drawFlag = false;
}


chip8 chip;

int main(){
	//Init Raylab
	const int screenWidth = 640;
    	const int screenHeight = 320;

	InitWindow(screenWidth, screenHeight, "Chip-8 Emulator");

	InitAudioDevice();

	Sound fxWav = LoadSound("beep.wav");
	SetTargetFPS(60);

	//Init Chip-8 Emulator
	chip.initialize();

	//Load Program
	chip.loadProgram();
	
	while(!WindowShouldClose()){
		if (IsKeyDown(KEY_ZERO)) chip.key[0] = 1;
		if (IsKeyDown(KEY_ONE)) chip.key[1] = 1;
		if (IsKeyDown(KEY_TWO)) chip.key[2] = 1;
		if (IsKeyDown(KEY_THREE)) chip.key[3] = 1;
		if (IsKeyDown(KEY_FOUR)) chip.key[4] = 1;
		if (IsKeyDown(KEY_FIVE)) chip.key[5] = 1;
		if (IsKeyDown(KEY_SIX)) chip.key[6] = 1;
		if (IsKeyDown(KEY_SEVEN)) chip.key[7] = 1;
		if (IsKeyDown(KEY_EIGHT)) chip.key[8] = 1;
		if (IsKeyDown(KEY_NINE)) chip.key[9] = 1;
		if (IsKeyDown(KEY_A)) chip.key[10] = 1;
		if (IsKeyDown(KEY_B)) chip.key[11] = 1;
		if (IsKeyDown(KEY_C)) chip.key[12] = 1;
		if (IsKeyDown(KEY_D)) chip.key[13] = 1;
		if (IsKeyDown(KEY_E)) chip.key[14] = 1;
		if (IsKeyDown(KEY_F)) chip.key[15] = 1;
		
		chip.emulateCycle();

		if(chip.sound_timer != 0)
			PlaySound(fxWav);

		BeginDrawing();
		if(chip.drawFlag){
			chip.drawGraphics();	
		}

        	EndDrawing();
		for(int i = 0; i < 16; i++)
			chip.key[i] = 0;
	}

	UnloadSound(fxWav);
	CloseAudioDevice();
	CloseWindow();
	return 0;
}
