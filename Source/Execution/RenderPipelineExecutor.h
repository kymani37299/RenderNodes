#pragma once

#include "../Common.h"
#include "ExecuteContext.h"

class RenderPipelineExecutor : public IInputListener
{
public:
    void OnStart();
    void OnUpdate(float dt);
    void Render();

    void SetCompiledPipeline(CompiledPipeline pipeline);

    void OnKeyInputEvent(const KeyInput& input) override;

private:
    void HandleErrors();

private:
    ExecuteContext m_Context;
    CompiledPipeline m_Pipeline;
};