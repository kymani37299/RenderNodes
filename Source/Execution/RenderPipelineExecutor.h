#pragma once

#include "../Common.h"
#include "ExecuteContext.h"

class RenderPipelineExecutor
{
public:
    void OnStart();
    void OnUpdate(float dt);
    void Render();

    void HandleKeyPressed(int key, int mods);
    void HandleKeyReleased(int key, int mods);

    void SetCompiledPipeline(CompiledPipeline pipeline);
private:
    ExecuteContext m_Context;
    CompiledPipeline m_Pipeline;
};