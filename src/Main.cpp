#include <iostream>

#include <GL/glut.h>
#include "Chip8.h"

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define MODIFIER 10

Chip8 chip8;

float deltaTime = 0;
float unprocessedTime = 0;
int lastTime = 0;

void drawPixel(int x, int y)
{
  glBegin(GL_QUADS);
		glVertex3f((x * MODIFIER) + 0.0f,     (y * MODIFIER) + 0.0f,	 0.0f);
		glVertex3f((x * MODIFIER) + 0.0f,     (y * MODIFIER) + MODIFIER, 0.0f);
		glVertex3f((x * MODIFIER) + MODIFIER, (y * MODIFIER) + MODIFIER, 0.0f);
		glVertex3f((x * MODIFIER) + MODIFIER, (y * MODIFIER) + 0.0f,	 0.0f);
	glEnd();
}

void display()
{
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  // glClear(GL_COLOR_BUFFER_BIT);

  int currentTime = glutGet(GLUT_ELAPSED_TIME);
  deltaTime = (currentTime - lastTime) / 1000.0f;
  lastTime = currentTime;

  unprocessedTime += deltaTime;

  if(unprocessedTime > 1.0f / 600.0f)
  {
    chip8.Cycle();
    unprocessedTime = 0;
    // std::cout << std::hex << (int)chip8.memory[chip8.PC] << (int)chip8.memory[chip8.PC + 1] << std::dec << std::endl;

    if(chip8.drawFlag)
    {
      for(int y = 0; y < SCREEN_HEIGHT; y++)
      {
        for(int x = 0; x < SCREEN_WIDTH; x++)
        {
          if(chip8.gfx[y * SCREEN_WIDTH + x] == 0)
            glColor3f(0.0f, 0.0f, 0.0f);
          else
            glColor3f(1.0f, 1.0f, 1.0f);

          drawPixel(x, y);
        }
      }

      chip8.drawFlag = false;
      glFlush();
    }
  }
}

void keyPressed(unsigned char key, int x, int y)
{
  if(key == 27) exit(0);

       if (key == '1') chip8.keys[0x1] = 1;
  else if (key == '2') chip8.keys[0x2] = 1;
  else if (key == '3') chip8.keys[0x3] = 1;
  else if (key == '4') chip8.keys[0xC] = 1;

  else if (key == 'q') chip8.keys[0x4] = 1;
  else if (key == 'w') chip8.keys[0x5] = 1;
  else if (key == 'e') chip8.keys[0x6] = 1;
  else if (key == 'r') chip8.keys[0xD] = 1;

  else if (key == 'a') chip8.keys[0x7] = 1;
  else if (key == 's') chip8.keys[0x8] = 1;
  else if (key == 'd') chip8.keys[0x9] = 1;
  else if (key == 'f') chip8.keys[0xE] = 1;

  else if (key == 'z') chip8.keys[0xA] = 1;
  else if (key == 'x') chip8.keys[0x0] = 1;
  else if (key == 'c') chip8.keys[0xB] = 1;
  else if (key == 'v') chip8.keys[0xF] = 1;
}

void keyReleased(unsigned char key, int x, int y)
{
     if (key == '1') chip8.keys[0x1] = 0;
else if (key == '2') chip8.keys[0x2] = 0;
else if (key == '3') chip8.keys[0x3] = 0;
else if (key == '4') chip8.keys[0xC] = 0;

else if (key == 'q') chip8.keys[0x4] = 0;
else if (key == 'w') chip8.keys[0x5] = 0;
else if (key == 'e') chip8.keys[0x6] = 0;
else if (key == 'r') chip8.keys[0xD] = 0;

else if (key == 'a') chip8.keys[0x7] = 0;
else if (key == 's') chip8.keys[0x8] = 0;
else if (key == 'd') chip8.keys[0x9] = 0;
else if (key == 'f') chip8.keys[0xE] = 0;

else if (key == 'z') chip8.keys[0xA] = 0;
else if (key == 'x') chip8.keys[0x0] = 0;
else if (key == 'c') chip8.keys[0xB] = 0;
else if (key == 'v') chip8.keys[0xF] = 0;
}

int main(int argc, char** argv)
{
  if(argc < 2)
  {
    std::cout << "Usage: chip8 *program*" << std::endl;
    return -1;
  }

  glutInit(&argc, argv);
  glutInitWindowSize(SCREEN_WIDTH * MODIFIER, SCREEN_HEIGHT * MODIFIER);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA | GLUT_STENCIL);
  glutCreateWindow("Chip8");

  glutDisplayFunc(display);
  glutIdleFunc(display);
  glutKeyboardFunc(keyPressed);
  glutKeyboardUpFunc(keyReleased);

  if(!chip8.LoadProgram(argv[1]))
  {
    std::cout << "Failed to load program: " << argv[1] << std::endl;
    return 1;
  }

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, SCREEN_WIDTH * MODIFIER, SCREEN_HEIGHT * MODIFIER, 0);
  glMatrixMode(GL_MODELVIEW);
  glViewport(0, 0, SCREEN_WIDTH * MODIFIER, SCREEN_HEIGHT * MODIFIER);

  glutMainLoop();

  return 0;
}
