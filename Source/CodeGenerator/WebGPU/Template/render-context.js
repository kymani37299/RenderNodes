export class RenderContext {
    static instance = null;

    constructor() {
        if (RenderContext.instance) {
            return RenderContext.instance;
        }

        this.device = null;
        this.context = null;
        this.presentationFormat = null;
        this.meshVAOCache = new Map();
        this.bindGroupCache = new Map();

        RenderContext.instance = this;
    }

    static getInstance() {
        if (!RenderContext.instance) {
            RenderContext.instance = new RenderContext();
        }
        return RenderContext.instance;
    }

    async initialize(canvas) {
        if (!navigator.gpu) {
            throw new Error('WebGPU is not supported in your browser');
        }

        const adapter = await navigator.gpu.requestAdapter();
        if (!adapter) {
            throw new Error('Failed to get WebGPU adapter');
        }

        this.device = await adapter.requestDevice();
        this.context = canvas.getContext('webgpu');
        this.presentationFormat = navigator.gpu.getPreferredCanvasFormat();

        this.context.configure({
            device: this.device,
            format: this.presentationFormat,
            alphaMode: 'opaque'
        });
        return this;
    }

    getDevice() {
        return this.device;
    }

    getContext() {
        return this.context;
    }

    getPresentationFormat() {
        return this.presentationFormat;
    }

    clearCaches() {
        this.meshVAOCache.clear();
        this.bindGroupCache.clear();
    }
}
