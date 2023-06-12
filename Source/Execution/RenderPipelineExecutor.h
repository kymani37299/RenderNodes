#pragma once

#include "../Common.h"
#include "ExecutorNode.h"

class RenderPipelineExecutor
{
public:
    void OnStart();
    void OnUpdate(float dt);
    void Render();
    
    void SetCompiledPipeline(CompiledPipeline pipeline)
    {
        delete m_Pipeline.OnStartNode;
        delete m_Pipeline.OnUpdateNode;
        m_Pipeline = pipeline;
    }
private:
    ExecuteContext m_Context;
    CompiledPipeline m_Pipeline;
};