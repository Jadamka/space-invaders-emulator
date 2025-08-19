#include <SDL2/SDL.h>
#include <iostream>

#include "config.h"
#include "cpu.h"

typedef struct
{
    SDL_Window* window;
    SDL_Renderer* renderer;
} sdl_t;

bool init_sdl(const config_t& config, sdl_t* sdl)
{
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) != 0)
    {
        std::cout << "Couldn't initialize SDL! Error: " << SDL_GetError() << std::endl;
        return false;
    }

    sdl->window = SDL_CreateWindow("Space Invaders",
                                   SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED,
                                   config.windowWidth * config.windowScale,
                                   config.windowHeight * config.windowScale,
                                   0);
    if(sdl->window == nullptr)
    {
        std::cout << "Couldn't create window! Error: " << SDL_GetError() << std::endl;
        return false;
    }

    sdl->renderer = SDL_CreateRenderer(sdl->window, -1, SDL_RENDERER_ACCELERATED);
    if(sdl->renderer == nullptr)
    {
        std::cout << "Couldn't create renderer! Error: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

void loop(const config_t& config, sdl_t* sdl, CPU* cpu)
{
    (void)cpu;
    const uint8_t bgRed = (config.backgroundColor >> 24) & 0xFF;
    const uint8_t bgGreen = (config.backgroundColor >> 16) & 0xFF;
    const uint8_t bgBlue = (config.backgroundColor >>  8) & 0xFF;
    const uint8_t bgAlpha = (config.backgroundColor >>  0) & 0xFF;

    bool quit = false;
    SDL_Event event;

    while(!quit)
    {
        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_QUIT)
                quit = true;
        }

        cpu->disassembler();
        SDL_SetRenderDrawColor(sdl->renderer, bgRed, bgGreen, bgBlue, bgAlpha);
        SDL_RenderClear(sdl->renderer);
        SDL_RenderPresent(sdl->renderer);
    }
}

void cleanup(sdl_t* sdl)
{
    SDL_DestroyRenderer(sdl->renderer);
    SDL_DestroyWindow(sdl->window);
    SDL_Quit();
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    config_t config;
    sdl_t sdl = {nullptr, nullptr};
    CPU* cpu = new CPU();

    if(init_sdl(config, &sdl) && cpu->load_rom("roms/invaders.h"))
        loop(config, &sdl, cpu);

    delete cpu;
    cleanup(&sdl);
    return 0;
}
