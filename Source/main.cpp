#include "Context.hpp"
#include <SDL3/SDL_gpu.h>

int main() {
    Context ctx;
    if(ctx.Setup(1280, 720, "Indexed") != 0){
        printf("Failed to setup context \n");
        return -1;
    }

    while (!ctx.IsQuiting()) {
        ctx.Update();
        ctx.Draw();
    }
    ctx.CleanUp();
    return 0;
}
