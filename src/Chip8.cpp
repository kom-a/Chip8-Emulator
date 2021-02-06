#include <cstring>
#include <cstdlib>
#include <iostream>

#include "Chip8.h"

unsigned char chip8_fontset[80] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

Chip8::Chip8()
{
  Init();
}

Chip8::~Chip8()
{

}

void Chip8::Init()
{
  memset(memory, 0, MEMORY_SIZE);
  memset(stack, 0, STACK_SIZE * 2);
  memset(V, 0, REGISTERS);
  memset(gfx, 0, 64 * 32);
  memset(keys, 0, 16);

  for(int i = 0; i < 80; i++)
    memory[i] = chip8_fontset[i];

  I = 0;
  PC = 0x200;
  SP = 0;

  drawFlag = true;

  delayTimer = 0;
  soundTimer = 0;

  srand (time(NULL));
}

bool Chip8::LoadProgram(const char* filename)
{
  FILE* file = fopen(filename, "rb");
  if(!file)
    return false;

  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  rewind(file);

  if(size > MEMORY_SIZE - 512) // first 512 bytes of memory reserved
    return false;

  uint8_t* buffer = (uint8_t*)malloc(sizeof(uint8_t) * size);
  if(!buffer)
    return false;

  size_t readBytes = fread(buffer, 1, size, file);
  if(readBytes != size)
    return false;

  for(int i = 0; i < size; i++)
  {
    memory[i + 512] = buffer[i];
  }
  free(buffer);
  fclose(file);

  return true;
}

uint16_t Chip8::Fetch()
{
  return (memory[PC] << 8) | memory[PC + 1];
}

void Chip8::Execute(uint16_t instruction)
{
  switch (instruction & 0xF000)
  {
    case 0x0000:
    {
      switch(instruction & 0xFF)
      {
        case 0xE0: // 00E0 | Clears the screen
        {
          for(int i = 0; i < 64 * 32; i++)
						gfx[i] = 0x0;
					drawFlag = true;
					PC += 2;
        } break;
        case 0xEE: // 00EE | Returns form a subroutine
        {
          SP--;
          PC = stack[SP];
          PC += 2;
        } break;
      }
    } break;
    case 0x1000: // 1NNN | Jumps to address NNN
    {
      PC = instruction & 0x0FFF;
    } break;
    case 0x2000: // 2NNN | Calls subroutine at NNN
    {
      stack[SP] = PC;
      SP++;
      PC = instruction & 0x0FFF;
    } break;
    case 0x3000: // 3XNN | Skips the next instruction if VX equals to NN
    {
      if(V[(instruction >> 8) & 0xF] == (instruction & 0xFF))
        PC += 4;
      else
        PC += 2;
    } break;
    case 0x4000: // 4XNN | Skips the next instruction if VX doesn't equal NN
    {
      if(V[(instruction >> 8) & 0xF] != (instruction & 0xFF))
        PC += 4;
      else
        PC += 2;
    } break;
    case 0x5000: // 5XY0 | Skips the next instruction if VX equals VY
    {
      if((V[(instruction >> 8) & 0xF]) == (V[instruction >> 4] & 0xF))
        PC += 4;
      else
        PC += 2;
    } break;
    case 0x6000: // 6XNN | Sets VX to NN
    {
      V[(instruction >> 8) & 0xF] = instruction & 0xFF;
      PC += 2;
    } break;
    case 0x7000: // 7XNN | Adds NN to VX
    {
      V[(instruction >> 8) & 0xF] += instruction & 0xFF;
      PC += 2;
    } break;
    case 0x8000:
    {
      switch(instruction & 0xF)
      {
        case 0x0000: // 8XY0 | Sets VX to the value of VY
        {
            V[(instruction >> 8) & 0xF] = V[(instruction >> 4) & 0xF];
        } break;
        case 0x0001: // 8XY1 | Sets VX to VX or VY
        {
          V[(instruction >> 8) & 0xF] |= V[(instruction >> 4) & 0xF];
        } break;
        case 0x0002: // 8XY2 | Sets VX to VX and VY
        {
            V[(instruction >> 8) & 0xF] &= V[(instruction >> 4) & 0xF];
        } break;
        case 0x0003: // 8XY3 | Sets VX to VX xor VY
        {
          V[(instruction >> 8) & 0xF] ^= V[(instruction >> 4) & 0xF];
        } break;
        case 0x0004: // 8XY4 | Adds VY to VX
        {
            // V[(instruction >> 8) & 0xF] += V[(instruction >> 4) & 0xF];
            if(V[(instruction >> 4) & 0xF] > (0xFF - V[(instruction >> 8) & 0xF]))
  						V[0xF] = 1; //carry
  					else
  						V[0xF] = 0;
  					V[(instruction & 0x0F00) >> 8] += V[(instruction & 0x00F0) >> 4];
        } break;
        case 0x0005: // 8XY5 | VY is substructed from VX
        {
          if(V[(instruction >> 4) & 0xF] > V[(instruction >> 8) & 0xF])
						V[0xF] = 0; // there is a borrow
					else
						V[0xF] = 1;

          V[(instruction >> 8) & 0xF] -= V[(instruction >> 4) & 0xF];
        } break;
        case 0x0006: // 8XY6 | Stores the least significant bit of VX in VF and then shifts VX to the right by 1
        {
          V[0xF] = V[(instruction >> 8) & 0xF] & 0x1;
          V[(instruction >> 8) & 0xF] >>= 0x1;
        } break;
        case 0x0007: // 8XY7 | Sets VX to VY minus VX
        {
          if(V[(instruction >> 8) & 0xF] > V[(instruction >> 4) & 0xF])
            V[0xF] = 0;
          else
            V[0xF] = 1;
          V[(instruction >> 8) & 0xF] = V[(instruction >> 4) & 0xF] - V[(instruction >> 8) & 0xF];
        } break;
        case 0x000E: // 8XYE | Stores the most significant bit of VX in VF and then shifts VX to the left by 1
        {
          V[0xF] = V[(instruction >> 8) & 0xF] >> 7;
          V[(instruction >> 8) & 0xF] <<= 1;
        } break;
      }
      PC += 2;
    } break;
    case 0x9000: // 9XY0 | Skips the next instruction if VX doesn't equal to VY
    {
      if((V[instruction >> 8] & 0xF) != (V[instruction >> 4] & 0xF))
        PC += 4;
      else
        PC += 2;
    } break;
    case 0xA000: // ANNN | Sets I to the address NNN
    {
      I = instruction & 0xFFF;
      PC += 2;
    } break;
    case 0xB000: // BNNN | Jumps to the address NNN plus V0
    {
      PC = (instruction & 0xFFF) + V[0];
    } break;
    case 0xC000: // CXNN | Sets VX to the result of a bitwise and operation on a random number (0 to 255) and NN
    {
      V[(instruction >> 8) & 0xF] = (instruction & 0xFF) & (rand() % 255);
      PC += 2;
    } break;
    case 0xD000: // DXYN | Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N + 1 pixels
              // Each row of 8 pixels is read as bit-coded starting from memory location I; I value doesn't change after the execution of this instruction
              // As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that doesn't happen
    {
      unsigned short x = V[(instruction & 0x0F00) >> 8];
      unsigned short y = V[(instruction & 0x00F0) >> 4];
      unsigned short height = instruction & 0x000F;
      unsigned short pixel;

      V[0xF] = 0;
      for (int yline = 0; yline < height; yline++)
      {
        pixel = memory[I + yline];
        for(int xline = 0; xline < 8; xline++)
        {
          if((pixel & (0x80 >> xline)) != 0)
          {
            if(gfx[(x + xline + ((y + yline) * 64))] == 1)
              V[0xF] = 1;
            gfx[x + xline + ((y + yline) * 64)] ^= 1;
          }
        }
      }

      drawFlag = true;
      PC += 2;
    } break;
    case 0xE000:
    {
      switch(instruction & 0xFF)
      {
        case 0x009E: // EX9E | Skips the next instruction if the key stored in VX is pressed
        {
          if(V[(instruction >> 8) & 0xF] != 0)
            PC += 4;
          else
            PC += 2;
        } break;
        case 0x00A1: // EXA1 | Skips the next instruction if the key stored in VX isn't pressed
        {
          if(keys[V[(instruction >> 8) & 0xF]] == 0)
            PC += 4;
          else
            PC += 2;
        } break;
      }
    } break;
    case 0xF000:
    {
      switch(instruction & 0xFF)
      {
        case 0x0007: // FX07 | Sets VX to the value of the delay timer
        {
          V[(instruction >> 8) & 0xF] = delayTimer;
          PC += 2;
        } break;
        case 0x000A: // FX0A | A key press is awaited, and then stored in VX
        {
          bool keyPress = false;

          for(int i = 0; i < 16; i++)
          {
            if(keys[i] != 0)
            {
              V[(instruction >> 8) & 0xF] = i;
              keyPress = true;
            }
          }

          if(!keyPress)
            return;

          PC += 2;
        } break;
        case 0x0015: // FX15 | Sets the delay timer to VX
        {
          delayTimer = V[(instruction >> 8) & 0xF];
          PC += 2;
        } break;
        case 0x0018: // FX18 | Sets the sound timer to VX
        {
          soundTimer = V[(instruction >> 8) & 0xF];
          PC += 2;
        } break;
        case 0x001E: // FX1E | Adds VX to I. VF is not affected
        {
          if(I + V[(instruction >> 8) & 0xF] > 0xFFF)	// VF is set to 1 when range overflow (I+VX>0xFFF), and 0 when there isn't.
            V[0xF] = 1;
          else
            V[0xF] = 0;

          I += V[(instruction >> 8) & 0xF];
          PC += 2;
        } break;
        case 0x0029: // FX29 | Sets I to the location of the sprite for the character in VX. Characters 0-F(in hex) are represented by a 4x5 font
        {
          I = V[(instruction >> 8) & 0xF] * 0x5;
          PC += 2;
        } break;
        case 0x0033: // FX33 | Stores the binary-coded decimal representation of VX, with the most significant of three diguts at the address in I,
                   //  the middle digit at I plus 1, and the least significant digit at I plus 2
        {
          memory[I]     = V[(instruction & 0x0F00) >> 8] / 100;
          memory[I + 1] = (V[(instruction & 0x0F00) >> 8] / 10) % 10;
          memory[I + 2] = (V[(instruction & 0x0F00) >> 8] % 100) % 10;
          PC += 2;
        } break;
        case 0x0055: // FX55 | Stores V0 to VX in memory starting at address I. The offset from I in screased by 1 for eacg value written, but I itselt is left unmodified
        {
          for (int i = 0; i <= ((instruction >> 8) & 0xF); i++)
						memory[I + i] = V[i];

					I += ((instruction >> 8) & 0xF) + 1;
          PC += 2;
        } break;
        case 0x0065: // FX65 : Fills V0 to VX with values from memory starting at address I. The offset form I is increased by 1 for each value written, but I itself is left unmidified
        {
          for (int i = 0; i <= ((instruction & 0x0F00) >> 8); ++i)
            V[i] = memory[I + i];

          // On the original interpreter, when the operation is done, I = I + X + 1.
          I += ((instruction & 0x0F00) >> 8) + 1;
          PC += 2;
        } break;
      } break;
    }
    default:
    {
      std::cout << "Incorrect instruction: " << std::hex << instruction << std::dec << std::endl;
    }
  }
}

void Chip8::Cycle()
{
  uint16_t instruction = Fetch();
  Execute(instruction);

	// Update timers
	if(delayTimer > 0)
		--delayTimer;

	if(soundTimer > 0)
	{
		if(soundTimer == 1)
			printf("Beep\n");
		--soundTimer;
	}
}
