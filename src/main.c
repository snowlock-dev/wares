#include <stdio.h>
#include <stdbool.h>
#include <SDL3/SDL.h>


#define GB_WIDTH 160
#define GB_HEIGHT 144
#define SCALE 4


int main(int argc, char* argv[]) {

	if (argc < 2) {
		fprintf(stderr, "Usage: wares <path_to_rom>");
		return 1;
	}

	const char* rom_path = argv[1];
	printf("Attempting to load ROM: %s\n", rom_path);

	if (!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_Log("SDL_Init failed: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow(
			"Wares",
			GB_WIDTH * SCALE,
			GB_HEIGHT * SCALE,
			SDL_WINDOW_RESIZABLE
			);
	
	if (!window) {
		SDL_Log("SDL_CreateWindow failed: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL) ;
	if (!renderer) {
		SDL_Log("SDL_CreateRenderer failed: %s\n", SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}
	bool running = true;
	SDL_Event event;

	printf("Window opened successfully, Press X to exit\n");

	while (running) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				running = false;
			}
		}
	
		SDL_SetRenderDrawColor(renderer, 32, 32, 32, 255); //painting grey for now
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);

        SDL_Delay(16);
	}


	SDL_DestroyWindow(window);
	SDL_Quit();

	printf("Wares shut down\n");
	return 0;

}
