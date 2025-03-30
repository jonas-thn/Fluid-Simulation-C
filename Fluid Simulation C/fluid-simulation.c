#include <stdio.h>
#include <SDl.h>

#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 600

#define COLOR_WHITE 0xffffffff
#define COLOR_BLACK 0x00000000
#define COLOR_BLUE 0xff34c3eb
#define COLOR_GRAY 0x0f0f0f0f
#define CELL_SIZE 20
#define LINE_WIDTH 2
#define COLUMNS SCREEN_WIDTH / CELL_SIZE
#define ROWS SCREEN_HEIGHT / CELL_SIZE
#define WATER_TYPE 0
#define SOLID_TYPE 1

struct Cell
{
	int type;
	double fill_level;
	int x;
	int y;
};

void draw_cell(SDL_Surface* surface, struct Cell cell)
{
	int pixel_x = cell.x * CELL_SIZE;
	int pixel_y = cell.y * CELL_SIZE;
	SDL_Rect cell_rect = (SDL_Rect){ pixel_x, pixel_y, CELL_SIZE, CELL_SIZE };
	SDL_FillRect(surface, &cell_rect, COLOR_BLACK);

	if (cell.type == WATER_TYPE) 
	{
		int water_height = cell.fill_level * CELL_SIZE;
		int empty_height = CELL_SIZE - water_height;
		SDL_Rect water_rect = (SDL_Rect){ pixel_x, pixel_y + empty_height, CELL_SIZE, water_height };
		SDL_FillRect(surface, &water_rect, COLOR_BLUE);
	}

	if (cell.type == SOLID_TYPE)
	{
		SDL_FillRect(surface, &cell_rect, COLOR_WHITE);
	}
}

void draw_enviroment(SDL_Surface * surface, struct Cell enviroment[ROWS * COLUMNS])
{
	for (int i = 0; i < ROWS * COLUMNS; i++)
	{
		draw_cell(surface, enviroment[i]);
	}
}

void draw_grid(SDL_Surface* surface)
{
	for (int i = 0; i < COLUMNS; i++)
	{
		SDL_Rect column = (SDL_Rect){ i * CELL_SIZE, 0, LINE_WIDTH, SCREEN_HEIGHT };
		SDL_FillRect(surface, &column, COLOR_GRAY);
	
	}

	for (int j = 0; j < ROWS; j++)
	{
		SDL_Rect row = (SDL_Rect){ 0, j * CELL_SIZE, SCREEN_WIDTH, LINE_WIDTH };
		SDL_FillRect(surface, &row, COLOR_GRAY);	
	}
}

void initialize_enviroment(struct Cell enviroment[ROWS * COLUMNS])
{
	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLUMNS; j++)
		{
			enviroment[j + COLUMNS * i] = (struct Cell){ WATER_TYPE, 0, j, i };
		}
	}
}

void simulation_step(struct Cell enviroment[ROWS * COLUMNS])
{
	struct Cell enviroment_next[ROWS * COLUMNS];

	for (int i = 0; i < ROWS * COLUMNS; i++)
	{
		enviroment_next[i] = enviroment[i];
	}

	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLUMNS; j++)
		{
			//water flows down
			struct Cell source_cell = enviroment[j + COLUMNS * i];
			if (source_cell.type == WATER_TYPE && i < ROWS - 1)
			{
				struct Cell destination_cell = enviroment[j + COLUMNS * (i + 1)];
				if (destination_cell.fill_level < source_cell.fill_level)
				{
					enviroment_next[j + COLUMNS * i].fill_level = 0;
					enviroment_next[j + COLUMNS * (i + 1)].fill_level += source_cell.fill_level;
				}
			}
		}
	}

	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLUMNS; j++)
		{
			struct Cell source_cell = enviroment[j + COLUMNS * i];

			//check sell below full or solid or bottom border
			if (i + 1 == ROWS || enviroment[j + COLUMNS * (i + 1)].fill_level >= 1 || enviroment[j + COLUMNS * (i+1)].type == SOLID_TYPE)
			{
				//water flowing left
				if (source_cell.type == WATER_TYPE && j > 0)
				{
					struct Cell destination_cell = enviroment[(j - 1) + COLUMNS * i];
					if (destination_cell.type == WATER_TYPE && destination_cell.fill_level < source_cell.fill_level)
					{
						double delta_fill = source_cell.fill_level - destination_cell.fill_level;

						enviroment_next[j + COLUMNS * i].fill_level -= delta_fill / 3;
						enviroment_next[(j - 1) + COLUMNS * i].fill_level += delta_fill / 3;
					}
				}
			}
		}
	}

	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLUMNS; j++)
		{
			struct Cell source_cell = enviroment[j + COLUMNS * i];

			//check sell below full or solid or bottom border
			if (i + 1 == ROWS || enviroment[j + COLUMNS * (i + 1)].fill_level >= 1 || enviroment[j + COLUMNS * (i + 1)].type == SOLID_TYPE)
			{
				//water flowing right
				if (source_cell.type == WATER_TYPE && j < COLUMNS - 1)
				{
					struct Cell destination_cell = enviroment[(j + 1) + COLUMNS * i];
					if (destination_cell.type == WATER_TYPE && destination_cell.fill_level < source_cell.fill_level)
					{
						double delta_fill = source_cell.fill_level - destination_cell.fill_level;

						enviroment_next[j + COLUMNS * i].fill_level -= delta_fill / 3;
						enviroment_next[(j + 1) + COLUMNS * i].fill_level += delta_fill / 3;
					}
				}
			}
		}
	}

	for (int i = 0; i < ROWS * COLUMNS; i++)
	{
		enviroment[i] = enviroment_next[i];
	}
}

int main()
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = SDL_CreateWindow("Liquid Simulation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);

	SDL_Surface* surface = SDL_GetWindowSurface(window);

	struct Cell enviroment[ROWS * COLUMNS];
	initialize_enviroment(enviroment);

	int simulation_running = 1;
	int current_type = SOLID_TYPE;
	int delete_mode = 0;

	SDL_Event event;

	while(simulation_running)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				simulation_running = 0;
			}

			if (event.type == SDL_MOUSEMOTION)
			{
				if(event.motion.state != 0)
				{
					int cell_x = event.motion.x / CELL_SIZE;
					int cell_y = event.motion.y / CELL_SIZE;

					int fill_level = delete_mode ? 0 : 1;

					if (delete_mode != 0)
					{
						current_type = WATER_TYPE;
					}

					struct Cell cell = (struct Cell){ current_type, fill_level, cell_x, cell_y };

					enviroment[cell_x + COLUMNS * cell_y] = cell;
				}

			}

			if (event.type == SDL_KEYDOWN)
			{
				if (event.key.keysym.sym == SDLK_SPACE)
				{
					current_type = !current_type;
				}
				if (event.key.keysym.sym == SDLK_BACKSPACE)
				{
					delete_mode = !delete_mode;
				}
			}
		}

		simulation_step(enviroment);
		draw_enviroment(surface, enviroment);
		draw_grid(surface);
		SDL_UpdateWindowSurface(window);

		SDL_Delay(30);
	}

	
}