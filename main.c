#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>

#define WINDOW_SIZE 601
#define CELL_SIZE 10

const int cell_number = WINDOW_SIZE/CELL_SIZE;

int loop_frequency = 100;

typedef struct {
    int x;
    int y;
} Pos;

int pos_to_index(Pos pos) {
    return (pos.x * (cell_number) + pos.y);
}

int neighbors(char* cells, Pos pos) {
    int result = 0;
    for (char x = -1; x <= 1; x++) {
        for (char y = -1; y <= 1; y++) {
            if ( !(pos.x + x < 0 || pos.x + x > cell_number || pos.y + y < 0 || pos.y +y > cell_number || (x == 0 && y == 0)) ) {
                Pos tmp_pos = {.x = pos.x + x, .y = pos.y + y};
                if (*(cells + pos_to_index(tmp_pos)) == 1) {
                    result++;
                }
            }
        }
    }

    return result;
}

void update_grid(char** cells_stack, int current_stack) {
    char* cells = *(cells_stack + current_stack);
    char* result = *(cells_stack + (current_stack+1) % 2);
    for (int x = 0; x < cell_number; x++) {
        for (int y = 0; y < cell_number; y++) {
            Pos pos = {.x = x, .y = y};
            if (*(cells + pos_to_index(pos)) == 1) {
                int update_value = neighbors(cells, pos);
                if (update_value > 3) {
                    *(result + pos_to_index(pos)) = 0;
                } else if (update_value < 2) {
                    *(result + pos_to_index(pos)) = 0;
                } else if (update_value == 2 || update_value == 3) {
                    *(result + pos_to_index(pos)) = 1;
                } else {
                    SDL_Log("Unexpected neighbors values: %d", update_value);
                    exit(1);
                }
            } else {
                int update_value = neighbors(cells, pos);
                if (update_value == 3) {
                    *(result + pos_to_index(pos)) = 1;
                } else {
                    *(result + pos_to_index(pos)) = 0;
                }
            }
        }
    }
}

void render(SDL_Renderer* r, char* cells) {
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderClear(r);

    SDL_SetRenderDrawColor(r, 150, 150, 150, 255);
    for (int x = 0; x < WINDOW_SIZE; x += CELL_SIZE) {
        SDL_RenderDrawLine(r, x, 0, x, WINDOW_SIZE);
    }
    for (int y = 0; y < WINDOW_SIZE; y += CELL_SIZE) {
        SDL_RenderDrawLine(r, 0, y, WINDOW_SIZE, y);
    }

    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    int index = 0;
    for (int x = 0; x < cell_number; x++) {
        for (int y = 0; y < cell_number; y++) {
            if (cells[index]) {
                SDL_Rect rect = {.x = x*CELL_SIZE+1, .y = y*CELL_SIZE+1, .w=CELL_SIZE-1, .h=CELL_SIZE-1};
                SDL_RenderFillRect(r, &rect);
            }

            index++;
        }
    }

    SDL_RenderPresent(r);
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Game of Life", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_SIZE, WINDOW_SIZE, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    unsigned char running = 1;
    SDL_Event event;

    char* cells1 = calloc((cell_number)*(cell_number), sizeof(char)); 
    char* cells2 = calloc((cell_number)*(cell_number), sizeof(char));
    char** cells = malloc(sizeof(char*) * 2);
    *cells = cells1;
    *(cells + 1) = cells2;
    int current_stack = 0;

    const Uint8* keyboard_state = SDL_GetKeyboardState(NULL);
    char set_state = 0;

    unsigned char enable_loop = 0;
    Uint64 last_loop_iteration = SDL_GetTicks64();

    while (running) {
        char* current_cells = *(cells + current_stack);
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == 1) {
                    Pos pos = {.x = event.button.x/CELL_SIZE, event.button.y/CELL_SIZE};
                    set_state = !*(current_cells + pos_to_index(pos));
                    *(current_cells + pos_to_index(pos)) = set_state;
                    //SDL_Log("Left click on %d, %d", event.button.x/CELL_SIZE, event.button.y/CELL_SIZE);
                }
            } else if (event.type == SDL_MOUSEMOTION) {
                if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LEFT) {
                    if (event.button.x >= 0 && event.button.x < WINDOW_SIZE && event.button.y >= 0 && event.button.y < WINDOW_SIZE) {
                        Pos pos = {.x = event.button.x/CELL_SIZE, event.button.y/CELL_SIZE};
                        *(current_cells + pos_to_index(pos)) = set_state;
                    }
                }
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = 0;
                } else if (event.key.keysym.sym == SDLK_a) {
                    update_grid(cells, current_stack);
                } else if (event.key.keysym.sym == SDLK_SPACE) {
                    enable_loop = !(enable_loop);
                    last_loop_iteration = SDL_GetTicks64();
                } else if (event.key.keysym.sym == SDLK_r) {
                    free(*(cells + current_stack));
                    *(cells + current_stack) = calloc(cell_number*cell_number, sizeof(char));
                } else if (event.key.keysym.sym == SDLK_UP) {
                    loop_frequency /= 2;
                    SDL_Log("%d", loop_frequency);
                } else if (event.key.keysym.sym == SDLK_DOWN) {
                    loop_frequency *= 2;
                    SDL_Log("%d", loop_frequency);
                }
            }
        }
        if (enable_loop && SDL_GetTicks64() - last_loop_iteration > loop_frequency) {
            update_grid(cells, current_stack);
            current_stack = (current_stack + 1)%2;
            last_loop_iteration = SDL_GetTicks64();       
        }
        render(renderer, *(cells + current_stack));
    }

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    free(cells1);
    free(cells2);
    free(cells);
    return 0;
}