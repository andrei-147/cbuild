#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <cstdint>

constexpr SDL_FColor col = {0.0f, 1.0f, 0.0f, 1.0f};
constexpr SDL_Vertex v[] = {
    {SDL_FPoint{0.0f, 0.0f},     col, SDL_FPoint{0.0f, 0.0f}},
    {SDL_FPoint{0.25f, 0.0f},   col, SDL_FPoint{0.0f, 0.0f}},
    {SDL_FPoint{0.25f, 0.25f}, col, SDL_FPoint{0.0f, 0.0f}},
    {SDL_FPoint{0.0f, 0.25f},   col, SDL_FPoint{0.0f, 0.0f}}
};

constexpr int i[] = {
    0, 1, 2, 0, 2, 3
};

constexpr float wf = 800.0f;
constexpr uint64_t w = (uint64_t)wf;

constexpr float hf = 600.0f;
constexpr uint64_t h = (uint64_t)hf;

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("sfml", w, h, 0);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    SDL_SetRenderLogicalPresentation(renderer, 2, 2, SDL_RendererLogicalPresentation::SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);

    bool running = true;
    uint64_t vcount = sizeof(v) / sizeof(v[0]);
    uint64_t icount = sizeof(i) / sizeof(i[0]);


    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
                running = false;
            }
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderGeometry(renderer, 0, v, vcount, i, icount);
        SDL_RenderPresent(renderer);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
