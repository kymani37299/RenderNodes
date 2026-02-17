import { RenderContext } from './render-context.js';
import { RenderNodesEngine } from './engine.js'
import { Float2, Float3, Float4, Float4x4 } from './math-types.js';

export class RenderNodeAPI {

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

    static GetOrCreateBlitPipeline(device, targetFormat) {
        if (!this._blitPipelines) {
            this._blitPipelines = new Map();
        }
        
        if (this._blitPipelines.has(targetFormat)) {
            return this._blitPipelines.get(targetFormat);
        }

        const shaderModule = device.createShaderModule({
            code: `
                struct VertexOutput {
                    @builtin(position) position: vec4f,
                    @location(0) texCoord: vec2f,
                };

                @vertex
                fn VS(@builtin(vertex_index) vertexIndex: u32) -> VertexOutput {
                    var output: VertexOutput;
                    // Fullscreen triangle
                    let x = f32((vertexIndex & 1u) << 2u) - 1.0;
                    let y = f32((vertexIndex & 2u) << 1u) - 1.0;
                    output.position = vec4f(x, -y, 0.0, 1.0);
                    output.texCoord = vec2f((x + 1.0) * 0.5, (y + 1.0) * 0.5);
                    return output;
                }

                @group(0) @binding(0) var textureSampler: sampler;
                @group(0) @binding(1) var textureData: texture_2d<f32>;

                @fragment
                fn FS(input: VertexOutput) -> @location(0) vec4f {
                    return textureSample(textureData, textureSampler, input.texCoord);
                }
            `
        });

        const pipeline = device.createRenderPipeline({
            layout: 'auto',
            vertex: {
                module: shaderModule,
                entryPoint: 'VS',
            },
            fragment: {
                module: shaderModule,
                entryPoint: 'FS',
                targets: [{
                    format: targetFormat
                }]
            },
            primitive: {
                topology: 'triangle-list',
            }
        });

        this._blitPipelines.set(targetFormat, pipeline);
        return pipeline;
    }

    static ClearFramebuffer(framebuffer, clearColor) {
        const ctx = RenderContext.getInstance();
        const device = ctx.getDevice();

        if(!framebuffer || !framebuffer.color) {
            console.warn('ClearFramebuffer: Invalid framebuffer');
            return;
        }

        const commandEncoder = device.createCommandEncoder();
        const renderPass = commandEncoder.beginRenderPass({
            colorAttachments: [{
                view: framebuffer.color.view,
                clearValue: {
                    r: clearColor.x,
                    g: clearColor.y,
                    b: clearColor.z,
                    a: clearColor.w
                },
                loadOp: 'clear',
                storeOp: 'store'
            }],
            depthStencilAttachment: framebuffer.depth ? {
                view: framebuffer.depth.view,
                depthClearValue: 1.0,
                depthLoadOp: 'clear',
                depthStoreOp: 'store'
            } : undefined
        });
        renderPass.end();
        device.queue.submit([commandEncoder.finish()]);
    }

    static GetOrCreateMeshVAO(mesh, vertexBits) {
        const ctx = RenderContext.getInstance();
        const key = mesh.id || mesh;

        if (ctx.meshVAOCache.has(key)) {
            return ctx.meshVAOCache.get(key);
        }

        const vao = {
            buffers: [],
            attributes: []
        };

        let attributeLocation = 0;

        if (vertexBits.Position && mesh.positions) {
            vao.buffers.push({
                buffer: mesh.positions,
                arrayStride: 12, // 3 floats
                attributes: [{
                    shaderLocation: attributeLocation++,
                    offset: 0,
                    format: 'float32x3'
                }]
            });
        }

        if (vertexBits.Texcoord && mesh.texcoords) {
            vao.buffers.push({
                buffer: mesh.texcoords,
                arrayStride: 8, // 2 floats
                attributes: [{
                    shaderLocation: attributeLocation++,
                    offset: 0,
                    format: 'float32x2'
                }]
            });
        }

        if (vertexBits.Normal && mesh.normals) {
            vao.buffers.push({
                buffer: mesh.normals,
                arrayStride: 12, // 3 floats
                attributes: [{
                    shaderLocation: attributeLocation++,
                    offset: 0,
                    format: 'float32x3'
                }]
            });
        }

        if (vertexBits.Tangent && mesh.tangents) {
            vao.buffers.push({
                buffer: mesh.tangents,
                arrayStride: 16, // 4 floats
                attributes: [{
                    shaderLocation: attributeLocation++,
                    offset: 0,
                    format: 'float32x4'
                }]
            });
        }

        ctx.meshVAOCache.set(key, vao);
        return vao;
    }

    static CreateBindGroup(shader, bindTable, bindGroupLayout) {
        const ctx = RenderContext.getInstance();
        const device = ctx.getDevice();

        if (!bindTable) return null;

        const entries = [];
        let binding = 0;

        // Textures (samplers + texture views)
        if (bindTable.Textures) {
            for (const texBinding of bindTable.Textures) {
                const texture = texBinding.Value;
                if (!texture) continue;

                // Sampler
                entries.push({
                    binding: binding++,
                    resource: texture.sampler || device.createSampler({
                        magFilter: 'linear',
                        minFilter: 'linear'
                    })
                });

                // Texture view
                entries.push({
                    binding: binding++,
                    resource: texture.view
                });
            }
        }

        // Uniform buffer for scalar values
        const uniformData = RenderNodeAPI._packUniformData(bindTable);

        if (uniformData.length > 0) {
            const uniformBuffer = device.createBuffer({
                size: uniformData.length * 4,
                usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST
            });
            device.queue.writeBuffer(uniformBuffer, 0, new Float32Array(uniformData));

            entries.push({
                binding: binding++,
                resource: { buffer: uniformBuffer }
            });
        }

        if (entries.length === 0) return null;

        return device.createBindGroup({
            layout: bindGroupLayout,
            entries
        });
    }

    /**
     * Pack bind table uniforms into flat array
     * @private
     */
    static _packUniformData(bindTable) {
        const uniformData = [];

        if (bindTable.Floats) {
            for (const floatBinding of bindTable.Floats) {
                uniformData.push(floatBinding.Value);
            }
        }

        if (bindTable.Float2s) {
            for (const float2Binding of bindTable.Float2s) {
                const v = float2Binding.Value;
                uniformData.push(v.x, v.y, 0, 0); // Pad to vec4
            }
        }

        if (bindTable.Float3s) {
            for (const float3Binding of bindTable.Float3s) {
                const v = float3Binding.Value;
                uniformData.push(v.x, v.y, v.z, 0); // Pad to vec4
            }
        }

        if (bindTable.Float4s) {
            for (const float4Binding of bindTable.Float4s) {
                const v = float4Binding.Value;
                uniformData.push(v.x, v.y, v.z, v.w);
            }
        }

        if (bindTable.Float4x4s) {
            for (const mat4Binding of bindTable.Float4x4s) {
                const mat = mat4Binding.Value;
                // Flatten 4x4 matrix (column-major)
                for (let col = 0; col < 4; col++) {
                    for (let row = 0; row < 4; row++) {
                        uniformData.push(mat.data[col][row]);
                    }
                }
            }
        }

        return uniformData;
    }

    /**
     * Draw mesh with all parameters
     */
    static DrawMesh(framebuffer, shader, mesh, renderState, bindTable, vertexBits) {
        const ctx = RenderContext.getInstance();
        const device = ctx.getDevice();

        if (!framebuffer || !shader || !mesh) {
            console.error('DrawMesh: Missing required parameters');
            return;
        }

        const vao = RenderNodeAPI.GetOrCreateMeshVAO(mesh, vertexBits);
        const commandEncoder = device.createCommandEncoder();

        // Setup render pass descriptor
        const renderPassDescriptor = {
            colorAttachments: [{
                view: framebuffer.view,
                loadOp: 'load',
                storeOp: 'store'
            }]
        };

        // Add depth attachment if available
        if (framebuffer.depthView) {
            renderPassDescriptor.depthStencilAttachment = {
                view: framebuffer.depthView,
                depthClearValue: 1.0,
                depthLoadOp: renderState?.DepthWrite ? 'clear' : 'load',
                depthStoreOp: 'store'
            };
        }

        const renderPass = commandEncoder.beginRenderPass(renderPassDescriptor);

        // Set pipeline
        if (shader.pipeline) {
            renderPass.setPipeline(shader.pipeline);
        }

        // Bind vertex buffers
        vao.buffers.forEach((bufferInfo, index) => {
            renderPass.setVertexBuffer(index, bufferInfo.buffer);
        });

        // Bind index buffer
        if (mesh.indices) {
            renderPass.setIndexBuffer(mesh.indices, mesh.indexFormat || 'uint32');
        }

        // Bind resources (textures, uniforms)
        if (bindTable && shader.bindGroupLayout) {
            const bindGroup = RenderNodeAPI.CreateBindGroup(shader, bindTable, shader.bindGroupLayout);
            if (bindGroup) {
                renderPass.setBindGroup(0, bindGroup);
            }
        }

        // Draw
        if (mesh.indices) {
            renderPass.drawIndexed(mesh.indexCount || mesh.numPrimitives);
        } else {
            renderPass.draw(mesh.vertexCount || mesh.numVertices);
        }

        renderPass.end();
        device.queue.submit([commandEncoder.finish()]);
    }

    static PresentTexture(texture) {
        const ctx = RenderContext.getInstance();
        const device = ctx.getDevice();
        const context = ctx.getContext();

        if (!texture || !texture.gpuTexture) {
            console.warn('PresentTexture: Invalid texture');
            return;
        }

        const commandEncoder = device.createCommandEncoder();

        const swapChainFormat = context.getCurrentTexture().format;
        if (texture.gpuTexture.format === swapChainFormat) {
            
            commandEncoder.copyTextureToTexture(
                { texture: texture.gpuTexture },
                { texture: context.getCurrentTexture() },
                [texture.width, texture.height]
            );
        } else {
            const pipeline = this.GetOrCreateBlitPipeline(device, context.getCurrentTexture().format);
            const renderPass = commandEncoder.beginRenderPass({
                colorAttachments: [{
                    view: context.getCurrentTexture().createView(),
                    loadOp: 'clear',
                    storeOp: 'store',
                    clearValue: { r: 0, g: 0, b: 0, a: 1 }
                }]
            });
            renderPass.setPipeline(pipeline);
            const sampler = device.createSampler({
                magFilter: 'linear',
                minFilter: 'linear',
            });

            const bindGroup = device.createBindGroup({
                layout: pipeline.getBindGroupLayout(0),
                entries: [
                    { binding: 0, resource: sampler },
                    { binding: 1, resource: texture.gpuTexture.createView() }
                ]
            });
            renderPass.setBindGroup(0, bindGroup);
            renderPass.draw(3);
            renderPass.end();
        }
        device.queue.submit([commandEncoder.finish()]);
    }

    static PresentFramebuffer(framebuffer) {
        if(!framebuffer || !framebuffer.color) {
            console.warn('PresentFramebuffer: Invalid framebuffer');
            return;
        }
        this.PresentTexture(framebuffer.color);
    }

    static Print(value) {
        console.log(value);
    }

    static CreateBuffer(data, usage) {
        const ctx = RenderContext.getInstance();
        const device = ctx.getDevice();

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

    static CreateFramebuffer(width, height, isFramebuffer, isDepthStencil) {
        let colorUsage = GPUTextureUsage.TEXTURE_BINDING;
        if(isFramebuffer)
            colorUsage |= GPUTextureUsage.RENDER_ATTACHMENT;
        const colorTex = CreateTexture(width, height, 'rgba8unorm', colorUsage);
        let depthTex = null;
        if(isDepthStencil) {
            depthTex = CreateTexture(width, height, 'depth24plus', GPUTextureUsage.RENDER_ATTACHMENT);
        }
        return {
            color : colorTex,
            depth : depthTex
        };
    }

    static CreateTexture(width, height, format, textureUsage) {
        const ctx = RenderContext.getInstance();
        const device = ctx.getDevice();
        const textureFormat = format || ctx.getPresentationFormat();

        const gpuTexture = device.createTexture({
            size: [width, height],
            format: textureFormat,
            usage: textureUsage
        });

        return {
            gpuTexture,
            view: gpuTexture.createView(),
            width,
            height,
            format: textureFormat
        };
    }

    // ===== RESOURCE LOADING =====

    /**
     * Load a GLTF/GLB scene
     */
    static async LoadScene(path) {
        const ctx = RenderContext.getInstance();
        const device = ctx.getDevice();

        try {
            const response = await fetch(path);
            if (!response.ok) {
                throw new Error(`Failed to load scene: ${response.statusText}`);
            }

            const arrayBuffer = await response.arrayBuffer();
            const filename = path.split('/').pop();
            
            // Parse GLTF
            let gltf, buffers;
            if (filename.endsWith('.glb')) {
                const result = RenderNodeAPI._parseGLB(arrayBuffer);
                gltf = result.json;
                buffers = result.buffers;
            } else {
                throw new Error('Only GLB format is currently supported in LoadScene');
            }

            // Load all meshes from the scene
            const sceneObjects = [];
            
            if (gltf.meshes) {
                for (const mesh of gltf.meshes) {
                    for (const primitive of mesh.primitives) {
                        const meshData = await RenderNodeAPI._loadPrimitive(gltf, buffers, primitive, device);
                        if (meshData) {
                            sceneObjects.push({
                                MeshData: meshData,
                                Transform: Float4x4.identity()
                            });
                        }
                    }
                }
            }

            return {
                SceneObjects: sceneObjects,
                Name: path
            };
        } catch (error) {
            console.error('LoadScene error:', error);
            throw error;
        }
    }

    /**
     * Load a shader from WGSL file
     */
    static async LoadShader(path) {
        const ctx = RenderContext.getInstance();
        const device = ctx.getDevice();
        const presentationFormat = ctx.getPresentationFormat();

        try {
            const response = await fetch(path);
            if (!response.ok) {
                throw new Error(`Failed to load shader: ${response.statusText}`);
            }

            const shaderCode = await response.text();
            const shaderModule = device.createShaderModule({ code: shaderCode });

            // Create a basic pipeline layout
            const bindGroupLayout = device.createBindGroupLayout({
                entries: [
                    // Texture sampler
                    {
                        binding: 0,
                        visibility: GPUShaderStage.FRAGMENT,
                        sampler: { type: 'filtering' }
                    },
                    // Texture view
                    {
                        binding: 1,
                        visibility: GPUShaderStage.FRAGMENT,
                        texture: { sampleType: 'float' }
                    },
                    // Uniform buffer
                    {
                        binding: 2,
                        visibility: GPUShaderStage.VERTEX | GPUShaderStage.FRAGMENT,
                        buffer: { type: 'uniform' }
                    }
                ]
            });

            const pipelineLayout = device.createPipelineLayout({
                bindGroupLayouts: [bindGroupLayout]
            });

            const pipeline = device.createRenderPipeline({
                layout: pipelineLayout,
                vertex: {
                    module: shaderModule,
                    entryPoint: 'vertexMain',
                    buffers: [
                        {
                            arrayStride: 12, // Position
                            attributes: [{ shaderLocation: 0, offset: 0, format: 'float32x3' }]
                        },
                        {
                            arrayStride: 8, // Texcoord
                            attributes: [{ shaderLocation: 1, offset: 0, format: 'float32x2' }]
                        },
                        {
                            arrayStride: 12, // Normal
                            attributes: [{ shaderLocation: 2, offset: 0, format: 'float32x3' }]
                        }
                    ]
                },
                fragment: {
                    module: shaderModule,
                    entryPoint: 'fragmentMain',
                    targets: [{ format: presentationFormat }]
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

            return {
                pipeline,
                bindGroupLayout,
                path
            };
        } catch (error) {
            console.error('LoadShader error:', error);
            throw error;
        }
    }

    /**
     * Create or load a texture from file
     */
    static async LoadTexture(path, width, height, hasDepth, mipmaps) {
        const ctx = RenderContext.getInstance();
        const device = ctx.getDevice();

        // If path is empty, create an empty render target
        if (!path || path === '') {
            const texture = RenderNodeAPI.CreateTexture(width, height);
            
            if (hasDepth) {
                const depthTexture = RenderNodeAPI.CreateDepthTexture(width, height);
                texture.depthView = depthTexture.view;
            }
            
            return texture;
        }

        // Load texture from file
        try {
            const response = await fetch(path);
            if (!response.ok) {
                throw new Error(`Failed to load texture: ${response.statusText}`);
            }

            const blob = await response.blob();
            const bitmap = await createImageBitmap(blob);

            const textureDescriptor = {
                size: [bitmap.width, bitmap.height],
                format: 'rgba8unorm',
                usage: GPUTextureUsage.TEXTURE_BINDING | 
                       GPUTextureUsage.COPY_DST | 
                       GPUTextureUsage.RENDER_ATTACHMENT
            };

            const gpuTexture = device.createTexture(textureDescriptor);

            device.queue.copyExternalImageToTexture(
                { source: bitmap },
                { texture: gpuTexture },
                [bitmap.width, bitmap.height]
            );

            const sampler = device.createSampler({
                magFilter: 'linear',
                minFilter: 'linear',
                mipmapFilter: 'linear'
            });

            return {
                gpuTexture,
                view: gpuTexture.createView(),
                sampler,
                width: bitmap.width,
                height: bitmap.height,
                format: 'rgba8unorm'
            };
        } catch (error) {
            console.error('LoadTexture error:', error);
            throw error;
        }
    }

    // ===== MATH FUNCTIONS =====

    /**
     * Create perspective projection matrix
     */
    static PerspectiveFloat4x4(fovDegrees, aspectRatio, near, far) {
        const matrix = new Float4x4();
        const fovRadians = (fovDegrees * Math.PI) / 180;
        const f = 1.0 / Math.tan(fovRadians / 2);
        const rangeInv = 1.0 / (near - far);

        matrix.data[0][0] = f / aspectRatio;
        matrix.data[1][1] = f;
        matrix.data[2][2] = (near + far) * rangeInv;
        matrix.data[2][3] = -1;
        matrix.data[3][2] = near * far * rangeInv * 2;
        matrix.data[3][3] = 0;

        return matrix;
    }

    /**
     * Create look-at view matrix
     */
    static LookAtFloat4x4(eye, target, up) {
        const matrix = new Float4x4();
        
        // Calculate forward vector (normalized)
        const fx = target.x - eye.x;
        const fy = target.y - eye.y;
        const fz = target.z - eye.z;
        const fLen = Math.sqrt(fx * fx + fy * fy + fz * fz);
        const forward = {
            x: fx / fLen,
            y: fy / fLen,
            z: fz / fLen
        };

        // Calculate right vector (normalized)
        const rx = forward.y * up.z - forward.z * up.y;
        const ry = forward.z * up.x - forward.x * up.z;
        const rz = forward.x * up.y - forward.y * up.x;
        const rLen = Math.sqrt(rx * rx + ry * ry + rz * rz);
        const right = {
            x: rx / rLen,
            y: ry / rLen,
            z: rz / rLen
        };

        // Calculate up vector
        const upVec = {
            x: right.y * forward.z - right.z * forward.y,
            y: right.z * forward.x - right.x * forward.z,
            z: right.x * forward.y - right.y * forward.x
        };

        // Build view matrix
        matrix.data[0][0] = right.x;
        matrix.data[0][1] = upVec.x;
        matrix.data[0][2] = -forward.x;
        matrix.data[0][3] = 0;

        matrix.data[1][0] = right.y;
        matrix.data[1][1] = upVec.y;
        matrix.data[1][2] = -forward.y;
        matrix.data[1][3] = 0;

        matrix.data[2][0] = right.z;
        matrix.data[2][1] = upVec.z;
        matrix.data[2][2] = -forward.z;
        matrix.data[2][3] = 0;

        matrix.data[3][0] = -(right.x * eye.x + right.y * eye.y + right.z * eye.z);
        matrix.data[3][1] = -(upVec.x * eye.x + upVec.y * eye.y + upVec.z * eye.z);
        matrix.data[3][2] = forward.x * eye.x + forward.y * eye.y + forward.z * eye.z;
        matrix.data[3][3] = 1;

        return matrix;
    }

    /**
     * Create translation matrix
     */
    static TranslateFloat4x4(baseMatrix, translation) {
        const matrix = baseMatrix ? JSON.parse(JSON.stringify(baseMatrix)) : new Float4x4();
        
        matrix.data[3][0] = translation.x;
        matrix.data[3][1] = translation.y;
        matrix.data[3][2] = translation.z;

        return matrix;
    }

    /**
     * Create rotation matrix from Euler angles (degrees)
     */
    static RotateFloat4x4(baseMatrix, rotation) {
        const matrix = baseMatrix ? JSON.parse(JSON.stringify(baseMatrix)) : new Float4x4();
        
        const rx = (rotation.x * Math.PI) / 180;
        const ry = (rotation.y * Math.PI) / 180;
        const rz = (rotation.z * Math.PI) / 180;

        const cx = Math.cos(rx), sx = Math.sin(rx);
        const cy = Math.cos(ry), sy = Math.sin(ry);
        const cz = Math.cos(rz), sz = Math.sin(rz);

        // YXZ rotation order
        matrix.data[0][0] = cy * cz + sy * sx * sz;
        matrix.data[0][1] = cy * sz - sy * sx * cz;
        matrix.data[0][2] = sy * cx;

        matrix.data[1][0] = -cx * sz;
        matrix.data[1][1] = cx * cz;
        matrix.data[1][2] = sx;

        matrix.data[2][0] = -sy * cz + cy * sx * sz;
        matrix.data[2][1] = -sy * sz - cy * sx * cz;
        matrix.data[2][2] = cy * cx;

        return matrix;
    }

    /**
     * Create scale matrix
     */
    static ScaleFloat4x4(baseMatrix, scale) {
        const matrix = baseMatrix ? JSON.parse(JSON.stringify(baseMatrix)) : new Float4x4();
        
        matrix.data[0][0] *= scale.x;
        matrix.data[1][1] *= scale.y;
        matrix.data[2][2] *= scale.z;

        return matrix;
    }

    /**
     * Multiply two matrices
     */
    static MultiplyFloat4x4(a, b) {
        const result = new Float4x4();
        
        for (let i = 0; i < 4; i++) {
            for (let j = 0; j < 4; j++) {
                result.data[i][j] = 0;
                for (let k = 0; k < 4; k++) {
                    result.data[i][j] += a.data[i][k] * b.data[k][j];
                }
            }
        }
        
        return result;
    }

    // ===== PRIVATE HELPER METHODS =====

    static _parseGLB(arrayBuffer) {
        const view = new DataView(arrayBuffer);
        const magic = view.getUint32(0, true);
        
        if (magic !== 0x46546C67) {
            throw new Error('Invalid GLB file');
        }

        const jsonLength = view.getUint32(12, true);
        const jsonStart = 20;
        const jsonData = new Uint8Array(arrayBuffer, jsonStart, jsonLength);
        const gltf = JSON.parse(new TextDecoder().decode(jsonData));

        const binStart = jsonStart + jsonLength + 8;
        const binLength = view.getUint32(jsonStart + jsonLength + 4, true);
        const buffers = [arrayBuffer.slice(binStart, binStart + binLength)];

        return { json: gltf, buffers };
    }

    static async _loadPrimitive(gltf, buffers, primitive, device) {
        const positions = RenderNodeAPI._getAccessorData(gltf, buffers, primitive.attributes.POSITION);
        const texcoords = RenderNodeAPI._getAccessorData(gltf, buffers, primitive.attributes.TEXCOORD_0);
        const normals = RenderNodeAPI._getAccessorData(gltf, buffers, primitive.attributes.NORMAL);
        const indices = primitive.indices !== undefined 
            ? RenderNodeAPI._getAccessorData(gltf, buffers, primitive.indices)
            : null;

        if (!positions) return null;

        const positionBuffer = RenderNodeAPI._createBufferFromData(device, new Float32Array(positions), GPUBufferUsage.VERTEX);
        const texcoordBuffer = texcoords 
            ? RenderNodeAPI._createBufferFromData(device, new Float32Array(texcoords), GPUBufferUsage.VERTEX)
            : RenderNodeAPI._createBufferFromData(device, new Float32Array(positions.length / 3 * 2), GPUBufferUsage.VERTEX);
        const normalBuffer = normals 
            ? RenderNodeAPI._createBufferFromData(device, new Float32Array(normals), GPUBufferUsage.VERTEX)
            : RenderNodeAPI._createBufferFromData(device, RenderNodeAPI._generateNormals(positions), GPUBufferUsage.VERTEX);

        let indexBuffer = null;
        let indexCount = 0;
        if (indices) {
            indexBuffer = RenderNodeAPI._createBufferFromData(device, indices, GPUBufferUsage.INDEX);
            indexCount = indices.length;
        }

        return {
            positions: positionBuffer,
            texcoords: texcoordBuffer,
            normals: normalBuffer,
            indices: indexBuffer,
            indexCount,
            vertexCount: positions.length / 3,
            indexFormat: indices instanceof Uint16Array ? 'uint16' : 'uint32'
        };
    }

    static _getAccessorData(gltf, buffers, accessorIndex) {
        if (accessorIndex === undefined) return null;

        const accessor = gltf.accessors[accessorIndex];
        const bufferView = gltf.bufferViews[accessor.bufferView];
        const buffer = buffers[bufferView.buffer];

        const offset = (bufferView.byteOffset || 0) + (accessor.byteOffset || 0);
        const componentSize = { 5126: 4, 5123: 2, 5125: 4 }[accessor.componentType] || 4;
        const componentCount = { SCALAR: 1, VEC2: 2, VEC3: 3, VEC4: 4 }[accessor.type] || 1;
        const totalSize = accessor.count * componentCount * componentSize;

        const arrayBuffer = buffer.slice(offset, offset + totalSize);

        const typeMap = { 5126: Float32Array, 5123: Uint16Array, 5125: Uint32Array };
        return new (typeMap[accessor.componentType])(arrayBuffer);
    }

    static _createBufferFromData(device, data, usage) {
        const buffer = device.createBuffer({
            size: data.byteLength,
            usage,
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

    static _generateNormals(positions) {
        const normals = new Float32Array(positions.length);
        for (let i = 0; i < positions.length; i += 9) {
            const v0 = [positions[i], positions[i + 1], positions[i + 2]];
            const v1 = [positions[i + 3], positions[i + 4], positions[i + 5]];
            const v2 = [positions[i + 6], positions[i + 7], positions[i + 8]];

            const edge1 = [v1[0] - v0[0], v1[1] - v0[1], v1[2] - v0[2]];
            const edge2 = [v2[0] - v0[0], v2[1] - v0[1], v2[2] - v0[2]];

            const normal = [
                edge1[1] * edge2[2] - edge1[2] * edge2[1],
                edge1[2] * edge2[0] - edge1[0] * edge2[2],
                edge1[0] * edge2[1] - edge1[1] * edge2[0]
            ];

            const len = Math.sqrt(normal[0] ** 2 + normal[1] ** 2 + normal[2] ** 2);
            if (len > 0) {
                normal[0] /= len;
                normal[1] /= len;
                normal[2] /= len;
            }

            for (let j = 0; j < 3; j++) {
                normals[i + j * 3] = normal[0];
                normals[i + j * 3 + 1] = normal[1];
                normals[i + j * 3 + 2] = normal[2];
            }
        }
        return normals;
    }
}

export const ClearFramebuffer = RenderNodeAPI.ClearFramebuffer.bind(RenderNodeAPI);
export const DrawMesh = RenderNodeAPI.DrawMesh.bind(RenderNodeAPI);
export const PresentFramebuffer = RenderNodeAPI.PresentFramebuffer.bind(RenderNodeAPI);
export const Print = RenderNodeAPI.Print.bind(RenderNodeAPI);
export const CreateBuffer = RenderNodeAPI.CreateBuffer.bind(RenderNodeAPI);
export const CreateTexture = RenderNodeAPI.CreateTexture.bind(RenderNodeAPI);