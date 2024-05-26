#include "Context.hpp"
#include <SDL3/SDL_gpu.h>

int main() {
    Context ctx;
    ctx.Setup(1280, 720, "Indexed");

    while (!ctx.IsQuiting()) {
        ctx.Update();
        ctx.Draw();
    }
    return 0;
}
