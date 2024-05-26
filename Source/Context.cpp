//
// Created by lucac on 26/05/2024.
//

#include "Context.hpp"

#include <cstddef>
#include <cstdio>


#include "triangle_spirv.h"

Context::Context() {

}

int Context::Setup(int with,int height,const char* title) {

    m_pGpuDevice= SDL_GpuCreateDevice(SDL_GPU_BACKEND_ALL,SDL_TRUE);
    m_pWindow = SDL_CreateWindow(title,with,height,0);
    SDL_GpuClaimWindow(m_pGpuDevice,m_pWindow,SDL_GPU_COLORSPACE_NONLINEAR_SRGB,SDL_GPU_PRESENTMODE_VSYNC);

    SDL_GpuShaderCreateInfo vertexShader;
    SDL_GpuShaderCreateInfo fragmentShader;

    vertexShader.entryPointName = "main";
    vertexShader.stage = SDL_GPU_SHADERSTAGE_VERTEX;
    vertexShader.format = SDL_GPU_SHADERFORMAT_SPIRV;
    vertexShader.code = triangle_vert_spv;
    vertexShader.codeSize = sizeof(triangle_vert_spv);

    fragmentShader.entryPointName = "main";
    fragmentShader.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    fragmentShader.format = SDL_GPU_SHADERFORMAT_SPIRV;
    fragmentShader.code = triangle_frag_spv;
    fragmentShader.codeSize = sizeof(triangle_frag_spv);

    m_pVertexShader = SDL_GpuCreateShader(m_pGpuDevice, &vertexShader);
    m_pFragmentShader = SDL_GpuCreateShader(m_pGpuDevice, &fragmentShader);

    //Setup Vertex Binding
    SDL_GpuVertexBinding vertexBindings[]{{}};
    vertexBindings[0].binding = 0;
    vertexBindings[0].inputRate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    vertexBindings[0].stride = sizeof(Vertex);
    vertexBindings[0].stepRate = 1;

    //Setup Vertex Attributes (Position/Color)
    SDL_GpuVertexAttribute vertexAttributes[2] = {{}, {}};
    vertexAttributes[0].binding = 0;
    vertexAttributes[0].location = 0;
    vertexAttributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_VECTOR3;
    vertexAttributes[0].offset = offsetof(Vertex, Vertex::position);

    vertexAttributes[1].binding = 0;
    vertexAttributes[1].location = 1;
    vertexAttributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_VECTOR3;
    vertexAttributes[1].offset = offsetof(Vertex, Vertex::color);

    //Setup Vertex Input State
    SDL_GpuVertexInputState vertexInputState;
    vertexInputState.vertexAttributeCount = 2;
    vertexInputState.vertexAttributes = vertexAttributes;
    vertexInputState.vertexBindingCount = 1;
    vertexInputState.vertexBindings = vertexBindings;

    // I/V Buffer creation
    m_pVertexBuffer = SDL_GpuCreateGpuBuffer(m_pGpuDevice, SDL_GPU_BUFFERUSAGE_VERTEX_BIT, m_vertexBufferSize);
    m_pIndexBuffer = SDL_GpuCreateGpuBuffer(m_pGpuDevice, SDL_GPU_BUFFERUSAGE_INDEX_BIT, m_indexBufferSize);

    EX_ASSERT(m_pVertexBuffer != nullptr, "Failed to create vertex buffer! \n")
    EX_ASSERT(m_pVertexBuffer != nullptr, "Failed to create index buffer! \n")

    //First we copy the vertex buffer to the GPU memory with a staging buffer
    SDL_GpuCommandBuffer *cmdVertexCopy = SDL_GpuAcquireCommandBuffer(m_pGpuDevice);
    void *pVertexData;

    SDL_GpuCopyPass *pVertexCopyPass = SDL_GpuBeginCopyPass(cmdVertexCopy);
    SDL_GpuBufferCopy vertexBufferCopy{0, 0, m_vertexBufferSize};

    SDL_GpuTransferBuffer *vertexTransferBuffer = SDL_GpuCreateTransferBuffer(m_pGpuDevice,
                                                                              SDL_GPU_TRANSFERUSAGE_BUFFER,
                                                                              SDL_GPU_TRANSFER_MAP_WRITE,
                                                                              m_vertexBufferSize);

    EX_ASSERT(vertexTransferBuffer != nullptr, "Failed to create vertex transfer buffer! \n")

    //Mapping the data and copying, identical to vulkan
    SDL_GpuMapTransferBuffer(m_pGpuDevice, vertexTransferBuffer, SDL_FALSE, &pVertexData);
    memcpy(pVertexData, m_vertices, m_vertexBufferSize);
    SDL_GpuUnmapTransferBuffer(m_pGpuDevice, vertexTransferBuffer);

    SDL_GpuUploadToBuffer(pVertexCopyPass, vertexTransferBuffer, m_pVertexBuffer, &vertexBufferCopy, SDL_FALSE);
    SDL_GpuEndCopyPass(pVertexCopyPass);
    SDL_GpuSubmit(cmdVertexCopy);


    //First we copy the index buffer to the GPU memory with a staging buffer
    SDL_GpuCommandBuffer *cmdIndexCopy = SDL_GpuAcquireCommandBuffer(m_pGpuDevice);
    void *pIndexdata;

    SDL_GpuCopyPass *indexCopyPass = SDL_GpuBeginCopyPass(cmdIndexCopy);
    SDL_GpuBufferCopy indexBufferCopy{0, 0, m_indexBufferSize};

    SDL_GpuTransferBuffer *indexTransferBuffer = SDL_GpuCreateTransferBuffer(m_pGpuDevice,
                                                                             SDL_GPU_TRANSFERUSAGE_BUFFER,
                                                                             SDL_GPU_TRANSFER_MAP_WRITE,
                                                                             m_indexBufferSize);

    EX_ASSERT(indexTransferBuffer != nullptr, "Failed to create index transfer buffer! \n")

    //Mapping the data and copying, identical to vulkan
    SDL_GpuMapTransferBuffer(m_pGpuDevice, indexTransferBuffer, SDL_FALSE, &pIndexdata);
    memcpy(pIndexdata, m_indices, m_indexBufferSize);
    SDL_GpuUnmapTransferBuffer(m_pGpuDevice, indexTransferBuffer);

    SDL_GpuUploadToBuffer(indexCopyPass, indexTransferBuffer, m_pIndexBuffer, &indexBufferCopy, SDL_FALSE);
    SDL_GpuEndCopyPass(indexCopyPass);
    SDL_GpuSubmit(cmdIndexCopy);

    //I don't know if it's necessary
    SDL_GpuWait(m_pGpuDevice);

    SDL_GpuQueueDestroyTransferBuffer(m_pGpuDevice, vertexTransferBuffer);
    SDL_GpuQueueDestroyTransferBuffer(m_pGpuDevice, indexTransferBuffer);


    m_vertexBufferBinding[0].gpuBuffer = m_pVertexBuffer;
    m_vertexBufferBinding[0].offset = 0;
    m_indexBufferBinding.gpuBuffer = m_pIndexBuffer;
    m_indexBufferBinding.offset = 0;


    //Setting up various pipeline states
    SDL_GpuRasterizerState rasterizerState = {};
    rasterizerState.cullMode = SDL_GPU_CULLMODE_BACK;
    rasterizerState.frontFace = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
    rasterizerState.depthBiasEnable = SDL_FALSE;

    SDL_GpuMultisampleState multisampleState = {};
    multisampleState.multisampleCount = SDL_GPU_SAMPLECOUNT_1;
    multisampleState.sampleMask = UINT32_MAX;

    SDL_GpuColorAttachmentBlendState blendState = {};
    blendState.blendEnable = SDL_FALSE;
    blendState.colorWriteMask = SDL_GPU_COLORCOMPONENT_R_BIT | SDL_GPU_COLORCOMPONENT_G_BIT |
                                SDL_GPU_COLORCOMPONENT_B_BIT | SDL_GPU_COLORCOMPONENT_A_BIT;

    SDL_GpuColorAttachmentDescription attachmentDesc[1];
    attachmentDesc[0].format = SDL_GpuGetSwapchainFormat(m_pGpuDevice, m_pWindow);
    attachmentDesc[0].blendState = blendState;

    SDL_GpuGraphicsPipelineAttachmentInfo attachmentInfo = {};
    attachmentInfo.colorAttachmentCount = 1;
    attachmentInfo.colorAttachmentDescriptions = attachmentDesc;
    attachmentInfo.hasDepthStencilAttachment = SDL_FALSE;

    SDL_GpuGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.vertexShader = m_pVertexShader;
    pipelineInfo.fragmentShader = m_pFragmentShader;
    pipelineInfo.vertexInputState = vertexInputState;
    pipelineInfo.primitiveType = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pipelineInfo.rasterizerState = rasterizerState;
    pipelineInfo.multisampleState = multisampleState;
    pipelineInfo.depthStencilState = {};
    pipelineInfo.attachmentInfo = attachmentInfo;
    pipelineInfo.vertexResourceLayoutInfo = {};
    pipelineInfo.fragmentResourceLayoutInfo = {};
    pipelineInfo.blendConstants[0] = 0;
    pipelineInfo.blendConstants[1] = 0;
    pipelineInfo.blendConstants[2] = 0;
    pipelineInfo.blendConstants[3] = 0;

    m_pPipeline = SDL_GpuCreateGraphicsPipeline(m_pGpuDevice, &pipelineInfo);
    EX_ASSERT(m_pPipeline != nullptr, "Failed to create pipeline!\n")
    return 0;
}

void Context::Draw() {
    SDL_GpuCommandBuffer *cmdbuf = SDL_GpuAcquireCommandBuffer(m_pGpuDevice);
    Uint32 w, h;
    SDL_GpuTexture *swapchainTexture = SDL_GpuAcquireSwapchainTexture(cmdbuf, m_pWindow, &w, &h);

    m_viewport.w = w;
    m_viewport.h = h;
    m_viewport.minDepth = 0.0f;
    m_viewport.maxDepth = 1.0f;

    m_scissor.w = w;
    m_scissor.h = h;
    m_scissor.x = 0;
    m_scissor.y = 0;


    if (swapchainTexture != NULL) {
        SDL_GpuColorAttachmentInfo colorAttachmentInfo = {};
        colorAttachmentInfo.textureSlice.texture = swapchainTexture;
        colorAttachmentInfo.clearColor = SDL_GpuColor{0.0f, 0.0f, 0.0f, 1.0f};
        colorAttachmentInfo.loadOp = SDL_GPU_LOADOP_CLEAR;
        colorAttachmentInfo.storeOp = SDL_GPU_STOREOP_STORE;

        SDL_GpuRenderPass *renderPass = SDL_GpuBeginRenderPass(cmdbuf, &colorAttachmentInfo, 1, NULL);

        SDL_GpuBindGraphicsPipeline(renderPass, m_pPipeline);

        SDL_GpuSetViewport(renderPass, &m_viewport);
        SDL_GpuSetScissor(renderPass, &m_scissor);

        SDL_GpuBindVertexBuffers(renderPass, 0, 1, m_vertexBufferBinding);
        SDL_GpuBindIndexBuffer(renderPass, &m_indexBufferBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);

        SDL_GpuDrawInstancedPrimitives(renderPass, 0, 0, 2, 1);
        SDL_GpuEndRenderPass(renderPass);
    }
    SDL_GpuSubmit(cmdbuf);
}

void Context::Update() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                CleanUp();
                SDL_Quit();
                m_bQuit = true;
                break;
        }
    }
}

void Context::CleanUp() {
    SDL_GpuQueueDestroyGraphicsPipeline(m_pGpuDevice, m_pPipeline);
    SDL_GpuQueueDestroyShader(m_pGpuDevice, m_pVertexShader);
    SDL_GpuQueueDestroyShader(m_pGpuDevice, m_pFragmentShader);
    SDL_GpuQueueDestroyGpuBuffer(m_pGpuDevice, m_pVertexBuffer);
    SDL_GpuQueueDestroyGpuBuffer(m_pGpuDevice, m_pIndexBuffer);
    SDL_GpuUnclaimWindow(m_pGpuDevice,m_pWindow);
    SDL_DestroyWindow(m_pWindow);
    SDL_GpuDestroyDevice(m_pGpuDevice);
    SDL_Quit();
}
