//
// Created by lucac on 26/05/2024.
//

#ifndef CONTEXT_HPP
#define CONTEXT_HPP
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#define EX_ASSERT(cond, message,...) \
if(!(cond))                        \
{                                   \
printf(message);                    \
    return -1;                       \
}

struct Vertex {
    float position[3];
    float color[3];
};


class Context {
public:
    Context();
    int Setup(int with,int height,const char* title);
    void Draw();
    void Update();
    void CleanUp();
    bool IsQuiting() {
        return m_bQuit;
    }
private:
    Vertex m_vertices[4] = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}}
    };

    const uint16_t m_indices[6] = {0, 1, 2, 2, 3, 0};

    const uint32_t m_vertexBufferSize = sizeof(m_vertices[0]) * 4;
    const uint32_t m_indexBufferSize = sizeof(uint16_t) * 6;


    SDL_Window *m_pWindow = NULL;
    SDL_GpuDevice *m_pGpuDevice = NULL;
    SDL_GpuGraphicsPipeline *m_pPipeline = NULL;
    SDL_GpuViewport m_viewport;
    SDL_GpuRect m_scissor;
    SDL_GpuVertexBinding m_vertexBindings[1];
    SDL_GpuBuffer *m_pVertexBuffer = NULL;
    SDL_GpuBuffer *m_pIndexBuffer = NULL;
    SDL_GpuBufferBinding m_vertexBufferBinding[1];
    SDL_GpuBufferBinding m_indexBufferBinding;

    SDL_GpuShader *m_pVertexShader = NULL;
    SDL_GpuShader *m_pFragmentShader = NULL;

    bool m_bQuit = false;
};


#endif //CONTEXT_HPP
