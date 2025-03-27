#include <stdio.h>
#include <SDl.h>

#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 600

int main()
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_CreateWindow("Liquid Simulation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
}