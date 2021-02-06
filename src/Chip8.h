#include <cstdint>

#define MEMORY_SIZE 4096
#define REGISTERS 16
#define STACK_SIZE 16

class Chip8
{
public:
  Chip8();
  ~Chip8();

  void Cycle();

  bool LoadProgram(const char* filename);
  void ExecuteProgram();
public:
  uint8_t memory[MEMORY_SIZE];
  uint8_t V[REGISTERS];
  uint16_t stack[STACK_SIZE];
  uint8_t gfx[64 * 32];
  uint8_t keys[16];

  bool drawFlag;

  uint16_t I;
  uint16_t PC;
  uint16_t SP;

  uint8_t delayTimer;
  uint8_t soundTimer;

  uint16_t Fetch();
  void Execute(uint16_t opcode);
  void Init();
};
