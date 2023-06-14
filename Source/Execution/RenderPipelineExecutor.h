#pragma once

#include "../Common.h"
#include "ExecuteContext.h"

class RenderPipelineExecutor
{
public:
    void OnStart();
    void OnUpdate(float dt);
    void Render();
    
    void SetCompiledPipeline(CompiledPipeline pipeline);
private:
    ExecuteContext m_Context;
    CompiledPipeline m_Pipeline;
};