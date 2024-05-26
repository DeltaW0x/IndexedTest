#include "Context.hpp"
#include <SDL3/SDL_gpu.h>

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    Context ctx;
    ctx.Setup(1280, 720, "Indexed");

    while (!ctx.IsQuiting()) {
        ctx.Update();
        ctx.Draw();
    }
    return 0;
}
