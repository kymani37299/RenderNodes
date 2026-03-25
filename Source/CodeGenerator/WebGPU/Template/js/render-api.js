import { RenderNodesEngine } from './engine.js'
import { MathTypeOps } from './math-types.js';

export class RenderNodeAPI {

    static Print(value) {
        console.log(value);
    }

/////////////////////////////////// MATH_TYPES ////////////////////////////////////////
    static PerspectiveFloat4x4(fovDegrees, aspectRatio, near, far) {
        return MathTypeOps.PerspectiveFloat4x4(fovDegrees, aspectRatio, near, far);
    }

    static LookAtFloat4x4(eye, target, up) {
        return MathTypeOps.LookAtFloat4x4(eye, target, up);
    }

    static TranslateFloat4x4(baseMatrix, translation) {
        return MathTypeOps.TranslateFloat4x4(baseMatrix, translation);
    }

    static RotateFloat4x4(baseMatrix, rotation) {
        return MathTypeOps.RotateFloat4x4(baseMatrix, rotation);
    }

    static VectorNormalize(vector) {
        return MathTypeOps.VectorNormalize(vector);
    }

    static VectorCross(a, b) {
        return MathTypeOps.VectorCross(a, b);
    }

    static VectorAdd(a, b) {
        return MathTypeOps.VectorAdd(a, b);
    }

    static VectorSubtract(a, b) {
        return MathTypeOps.VectorSubtract(a, b);
    }

    static VectorMultiply(a, b) {
        return MathTypeOps.VectorMultiply(a, b);
    }

    static VectorDivide(a, b) {
        return MathTypeOps.VectorDivide(a, b);
    }

    static ScaleFloat4x4(baseMatrix, scale) {
        return MathTypeOps.ScaleFloat4x4(baseMatrix, scale);
    }

    static MultiplyFloat4x4(a, b) {
        return MathTypeOps.MultiplyFloat4x4(a, b);
    }
////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////// INPUT API ///////////////////////////////////////////

    static RegisterKeyDownEvent(keyString, callback) {
        RenderNodesEngine.instance.inputManager.registerKeyDown(keyString, callback);
    }

    static RegisterKeyReleasedEvent(keyString, callback) {
        RenderNodesEngine.instance.inputManager.registerKeyReleased(keyString, callback);
    }

    static RegisterKeyPressedEvent(keyString, callback) {
        RenderNodesEngine.instance.inputManager.registerKeyPressed(keyString, callback);
    }

////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////// FRAMEBUFFER/TEXTURE ////////////////////////////////////////
    static ClearFramebuffer(framebuffer, clearColor) {
        RenderNodesEngine.instance.renderer.clearFramebuffer(framebuffer, clearColor);
    }

    static PresentFramebuffer(framebuffer) {
        RenderNodesEngine.instance.renderer.presentFramebuffer(framebuffer);
    }

    static CreateFramebuffer(width, height, isFramebuffer, isDepthStencil) {
        return RenderNodesEngine.instance.renderer.createFramebuffer(width, height, isFramebuffer, isDepthStencil);
    }

    static async LoadTexture(path, width, height, isFramebuffer) {
        return await RenderNodesEngine.instance.renderer.loadTexture(path, width, height, isFramebuffer);
    }
////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////// RESOURCES ///////////////////////////////////////////////////
    static async LoadScene(path) {
        return await RenderNodesEngine.instance.renderer.loadScene(path);
    }

    static async LoadShader(path, bindMapping) {
        return await RenderNodesEngine.instance.renderer.loadShader(path, bindMapping);
    }

    static async InitStaticResources() {
        RenderNodesEngine.instance.renderer.initStaticResources();
    }

    static GetStaticResources() {
        return RenderNodesEngine.instance.renderer.getStaticResources();
    }
////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////// GFX COMMANDS ///////////////////////////////////////////////////
    static DrawMesh(framebuffer, shader, meshData, renderState, bindTable, vertexBufferBits) {
        return RenderNodesEngine.instance.renderer.drawMesh(framebuffer, shader, meshData, renderState, bindTable, vertexBufferBits);
    }
////////////////////////////////////////////////////////////////////////////////////////
}