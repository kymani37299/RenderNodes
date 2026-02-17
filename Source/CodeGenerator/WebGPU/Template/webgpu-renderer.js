/**
 * WebGPURenderer - Manages WebGPU rendering pipeline and operations
 */

import { RenderContext } from './render-context.js';

export class WebGPURenderer {
    constructor(canvas) {
        this.canvas = canvas;
        this.renderContext = null;
        this.pipeline = null;
        this.uniformBuffer = null;
        this.bindGroup = null;
        this.depthTexture = null;
    }

    async initialize() {
        this.renderContext = await RenderContext.getInstance().initialize(this.canvas);
        await this._setupPipeline();
    }

    async _setupPipeline() {
        const device = this.renderContext.getDevice();
        const presentationFormat = this.renderContext.getPresentationFormat();

        const shaderCode = `
            struct Uniforms {
                modelViewProj: mat4x4<f32>,
            };
            
            @group(0) @binding(0) var<uniform> uniforms: Uniforms;
            
            struct VertexInput {
                @location(0) position: vec3<f32>,
                @location(1) normal: vec3<f32>,
            };
            
            struct VertexOutput {
                @builtin(position) position: vec4<f32>,
                @location(0) normal: vec3<f32>,
                @location(1) worldPos: vec3<f32>,
            };
            
            @vertex
            fn vertexMain(input: VertexInput) -> VertexOutput {
                var output: VertexOutput;
                output.position = uniforms.modelViewProj * vec4<f32>(input.position, 1.0);
                output.normal = input.normal;
                output.worldPos = input.position;
                return output;
            }
            
            @fragment
            fn fragmentMain(input: VertexOutput) -> @location(0) vec4<f32> {
                let lightDir = normalize(vec3<f32>(1.0, 1.0, 1.0));
                let normal = normalize(input.normal);
                let diffuse = max(dot(normal, lightDir), 0.0);
                let ambient = 0.3;
                let lighting = ambient + diffuse * 0.7;
                
                let baseColor = vec3<f32>(0.7, 0.7, 0.8);
                let finalColor = baseColor * lighting;
                
                return vec4<f32>(finalColor, 1.0);
            }
        `;

        const shaderModule = device.createShaderModule({ code: shaderCode });

        // Create uniform buffer (4x4 matrix = 64 bytes)
        this.uniformBuffer = device.createBuffer({
            size: 64,
            usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST
        });

        // Create bind group layout
        const bindGroupLayout = device.createBindGroupLayout({
            entries: [{
                binding: 0,
                visibility: GPUShaderStage.VERTEX,
                buffer: { type: 'uniform' }
            }]
        });

        // Create bind group
        this.bindGroup = device.createBindGroup({
            layout: bindGroupLayout,
            entries: [{
                binding: 0,
                resource: { buffer: this.uniformBuffer }
            }]
        });

        // Create render pipeline
        this.pipeline = device.createRenderPipeline({
            layout: device.createPipelineLayout({
                bindGroupLayouts: [bindGroupLayout]
            }),
            vertex: {
                module: shaderModule,
                entryPoint: 'vertexMain',
                buffers: [
                    {
                        arrayStride: 12, // Position: 3 floats
                        attributes: [{
                            shaderLocation: 0,
                            offset: 0,
                            format: 'float32x3'
                        }]
                    },
                    {
                        arrayStride: 12, // Normal: 3 floats
                        attributes: [{
                            shaderLocation: 1,
                            offset: 0,
                            format: 'float32x3'
                        }]
                    }
                ]
            },
            fragment: {
                module: shaderModule,
                entryPoint: 'fragmentMain',
                targets: [{
                    format: presentationFormat
                }]
            },
            primitive: {
                topology: 'triangle-list',
                cullMode: 'back'
            },
            depthStencil: {
                depthWriteEnabled: true,
                depthCompare: 'less',
                format: 'depth24plus'
            }
        });

        // Create initial depth texture
        this._createDepthTexture();
    }

    _createDepthTexture() {
        const device = this.renderContext.getDevice();

        if (this.depthTexture) {
            this.depthTexture.destroy();
        }

        this.depthTexture = device.createTexture({
            size: [this.canvas.width, this.canvas.height],
            format: 'depth24plus',
            usage: GPUTextureUsage.RENDER_ATTACHMENT
        });
    }

    createBuffer(data, usage) {
        const device = this.renderContext.getDevice();

        const buffer = device.createBuffer({
            size: data.byteLength,
            usage: usage,
            mappedAtCreation: true
        });

        const mappedRange = buffer.getMappedRange();

        if (data instanceof Float32Array) {
            new Float32Array(mappedRange).set(data);
        } else if (data instanceof Uint16Array) {
            new Uint16Array(mappedRange).set(data);
        } else if (data instanceof Uint32Array) {
            new Uint32Array(mappedRange).set(data);
        }

        buffer.unmap();
        return buffer;
    }

    updateUniforms(mvpMatrix) {
        const device = this.renderContext.getDevice();
        device.queue.writeBuffer(this.uniformBuffer, 0, mvpMatrix);
    }

    render(primitives) {
        const device = this.renderContext.getDevice();
        const context = this.renderContext.getContext();

        const commandEncoder = device.createCommandEncoder();
        const textureView = context.getCurrentTexture().createView();

        const renderPass = commandEncoder.beginRenderPass({
            colorAttachments: [{
                view: textureView,
                clearValue: { r: 0.1, g: 0.1, b: 0.15, a: 1.0 },
                loadOp: 'clear',
                storeOp: 'store'
            }],
            depthStencilAttachment: {
                view: this.depthTexture.createView(),
                depthClearValue: 1.0,
                depthLoadOp: 'clear',
                depthStoreOp: 'store'
            }
        });

        renderPass.setPipeline(this.pipeline);
        renderPass.setBindGroup(0, this.bindGroup);

        // Draw each primitive
        for (const primitive of primitives) {
            renderPass.setVertexBuffer(0, primitive.positionBuffer);
            renderPass.setVertexBuffer(1, primitive.normalBuffer);

            if (primitive.indexBuffer) {
                renderPass.setIndexBuffer(primitive.indexBuffer, primitive.indexFormat);
                renderPass.drawIndexed(primitive.indexCount);
            } else {
                renderPass.draw(primitive.vertexCount);
            }
        }

        renderPass.end();
        device.queue.submit([commandEncoder.finish()]);
    }

    resize(width, height) {
        this.canvas.width = width;
        this.canvas.height = height;
        this._createDepthTexture();
    }
}
