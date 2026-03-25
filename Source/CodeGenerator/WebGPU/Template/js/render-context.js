import { Float3, Float4x4, MathTypeOps } from './math-types.js';

export class RenderContext {
    static SAMPLER_BINDING_INDEX = 16;

    constructor() {
        this.shaderCache = new Map();
        this.pipelineCache = new Map();
        this.staticResources = {};
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
        const presentationFormat = navigator.gpu.getPreferredCanvasFormat();

        this.context.configure({
            device: this.device,
            format: presentationFormat,
            alphaMode: 'opaque'
        });
        this.cmd = this.device.createCommandEncoder();
    }

    resize(width, height) {
        // TODO
    }

    initStaticResources() {
        // Cube mesh data
        const positions = [
            // Front
            -0.5, -0.5, 0.5,
            0.5, -0.5, 0.5,
            0.5, 0.5, 0.5,
            -0.5, 0.5, 0.5,
            // Back
            -0.5, -0.5, -0.5,
            -0.5, 0.5, -0.5,
            0.5, 0.5, -0.5,
            0.5, -0.5, -0.5,
            // Left
            -0.5, -0.5, -0.5,
            -0.5, -0.5, 0.5,
            -0.5, 0.5, 0.5,
            -0.5, 0.5, -0.5,
            // Right
            0.5, -0.5, -0.5,
            0.5, 0.5, -0.5,
            0.5, 0.5, 0.5,
            0.5, -0.5, 0.5,
            // Top
            -0.5, 0.5, -0.5,
            -0.5, 0.5, 0.5,
            0.5, 0.5, 0.5,
            0.5, 0.5, -0.5,
            // Bottom
            -0.5, -0.5, -0.5,
            0.5, -0.5, -0.5,
            0.5, -0.5, 0.5,
            -0.5, -0.5, 0.5
        ];

        const normals = [
            // Front
            0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1,
            // Back
            0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1,
            // Left
            -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0,
            // Right
            1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0,
            // Top
            0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0,
            // Bottom
            0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0
        ];

        const texcoords = [
            // Each face: 0,0 1,0 1,1 0,1
            0, 0, 1, 0, 1, 1, 0, 1,  // front
            0, 0, 1, 0, 1, 1, 0, 1,  // back
            0, 0, 1, 0, 1, 1, 0, 1,  // left
            0, 0, 1, 0, 1, 1, 0, 1,  // right
            0, 0, 1, 0, 1, 1, 0, 1,  // top
            0, 0, 1, 0, 1, 1, 0, 1   // bottom
        ];

        const indices = [
            // Front: 0,1,2,2,3,0
            0, 1, 2, 2, 3, 0,
            // Back: 4,5,6,6,7,4
            4, 5, 6, 6, 7, 4,
            // Left: 8,9,10,10,11,8
            8, 9, 10, 10, 11, 8,
            // Right: 12,13,14,14,15,12
            12, 13, 14, 14, 15, 12,
            // Top: 16,17,18,18,19,16
            16, 17, 18, 18, 19, 16,
            // Bottom: 20,21,22,22,23,20
            20, 21, 22, 22, 23, 20
        ];

        this.staticResources.CubeMesh = {
            PrimitiveCount: 1,
            Positions: { data: positions },
            Texcoords: { data: texcoords },
            Normals: { data: normals },
            Tangents: { data: [] },
            Indices: { data: indices }
        };
    }

    getStaticResources() {
        return this.staticResources;
    }

    clearFramebuffer(framebuffer, clearColor) {
        if(!framebuffer || !framebuffer.color) {
            console.warn('ClearFramebuffer: Invalid framebuffer');
            return;
        }

        const renderPass = this.cmd.beginRenderPass({
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
    }

    presentFramebuffer(framebuffer) {
        if(!framebuffer || !framebuffer.color) {
            console.warn('PresentFramebuffer: Invalid framebuffer');
            return;
        }
        this._presentTexture(framebuffer.color);
    }
    
    createFramebuffer(width, height, isFramebuffer, isDepthStencil) {
        let colorUsage = GPUTextureUsage.TEXTURE_BINDING;
        if(isFramebuffer)
            colorUsage |= GPUTextureUsage.RENDER_ATTACHMENT;
        const colorTex = this._createTexture(width, height, 'rgba8unorm', colorUsage);
        let depthTex = null;
        if(isDepthStencil) {
            depthTex = this._createTexture(width, height, 'depth24plus', GPUTextureUsage.RENDER_ATTACHMENT);
        }
        return {
            color : colorTex,
            depth : depthTex
        };
    }

    async loadTexture(path, width, height, isFramebuffer) {
        const BROWSER_SUPPORTED = ['jpg', 'jpeg', 'png', 'bmp', 'gif', 'webp'];
        const ext = path.split('.').pop().toLowerCase();

        if (!BROWSER_SUPPORTED.includes(ext)) {
            console.warn(`Unsupported texture format: .${ext}. Supported formats: ${BROWSER_SUPPORTED.join(', ')}. Returning 1x1 black texture.`);
            const gpuTexture = this.device.createTexture({
                size: [1, 1],
                format: 'rgba8unorm',
                usage: GPUTextureUsage.TEXTURE_BINDING | GPUTextureUsage.COPY_DST | GPUTextureUsage.RENDER_ATTACHMENT
            });
            const blackData = new Uint8Array([0, 0, 0, 255]);
            this.device.queue.writeTexture(
                { texture: gpuTexture },
                blackData,
                { bytesPerRow: 4, rowsPerImage: 1 },
                [1, 1]
            );
            const loadedTexture = {
                gpuTexture,
                view: gpuTexture.createView(),
                width: 1,
                height: 1,
                format: 'rgba8unorm'
            };
            return {
                color: loadedTexture,
                depth: null
            };
        }

        const response = await fetch(path);
        const blob = await response.blob();

        const resizeOptions = (width || height)
            ? { resizeWidth: width, resizeHeight: height, resizeQuality: 'high' }
            : {};

        const imageBitmap = await createImageBitmap(blob, resizeOptions);

        const finalWidth  = imageBitmap.width;
        const finalHeight = imageBitmap.height;
        const format = 'rgba8unorm';

        // Must have render attachment for copyExternalImageToTexture, so ignoring isFramebuffer
        let usage =
            GPUTextureUsage.TEXTURE_BINDING |
            GPUTextureUsage.COPY_DST |
            GPUTextureUsage.RENDER_ATTACHMENT;
        
        const gpuTexture = this.device.createTexture({
            size: [finalWidth, finalHeight],
            format,
            usage
        });

        this.device.queue.copyExternalImageToTexture(
            { source: imageBitmap },
            { texture: gpuTexture },
            [finalWidth, finalHeight]
        );

        const loadedTexture = {
            gpuTexture,
            view: gpuTexture.createView(),
            width: finalWidth,
            height: finalHeight,
            format
        }; 

        return  {
            color : loadedTexture,
            depth : null
        };
    }

    async loadScene(path) {
        const sceneUrl = new URL(path, window.location.href);
        const sceneDir = new URL('.', sceneUrl).href;
        const baseUrl = sceneDir;

        const response = await fetch(sceneUrl.href);
        if (!response.ok) {
            throw new Error(`Failed to load scene: ${path} (${response.status})`);
        }

        const gltf = await response.json();
        const buffers = await Promise.all((gltf.buffers || []).map((bufferDesc) => this._loadGltfBuffer(bufferDesc.uri, sceneDir)));

        const sceneIndex = gltf.scene || 0;
        const sceneDef = (gltf.scenes || [])[sceneIndex];
        if (!sceneDef) {
            throw new Error(`glTF scene index ${sceneIndex} not found`);
        }

        const sceneObjects = [];

        const traverseNode = (nodeIndex, parentMatrix) => {
            const node = (gltf.nodes || [])[nodeIndex];
            if (!node) return;

            const localMatrix = this._getNodeLocalMatrix(node);
            const worldMatrix = MathTypeOps.MultiplyFloat4x4(parentMatrix, localMatrix);

            if (node.mesh !== undefined && gltf.meshes) {
                const meshDef = gltf.meshes[node.mesh];
                if (meshDef && meshDef.primitives) {
                    meshDef.primitives.forEach((prim) => {
                        const meshData = this._extractPrimitiveMeshData(prim, gltf, buffers);
                        const material = this._extractPrimitiveMaterial(prim, gltf, baseUrl);

                        const object = {
                            ModelTransform: worldMatrix,
                            Mesh: meshData,
                            MeshData: meshData,
                            Material: material
                        };

                        sceneObjects.push(object);
                    });
                }
            }

            (node.children || []).forEach((child) => traverseNode(child, worldMatrix));
        };

        const rootMatrix = this._identityMatrix();
        (sceneDef.nodes || []).forEach((nodeIndex) => traverseNode(nodeIndex, rootMatrix));

        return {
            Objects: sceneObjects,
            SceneObjects: sceneObjects
        };
    }

    async loadShader(basePath, bindMapping) {
        if (this.shaderCache.has(basePath)) {
            return this.shaderCache.get(basePath);
        }

        const vertPath = `${basePath}.vert.wgsl`;
        const fragPath = `${basePath}.frag.wgsl`;

        const [vertCode, fragCode] = await Promise.all([
            this._fetchText(vertPath),
            this._fetchText(fragPath)
        ]);

        const vertexModule = this.device.createShaderModule({ code: vertCode });
        const fragmentModule = this.device.createShaderModule({ code: fragCode });

        await this._validateShaderModule(vertexModule, vertPath);
        await this._validateShaderModule(fragmentModule, fragPath);

        const shader = {
            basePath,
            bindMapping,
            vertexModule,
            fragmentModule,
            vertexCode: vertCode,
            fragmentCode: fragCode
        };

        this.shaderCache.set(basePath, shader);
        return shader;
    }

    drawMesh(framebuffer, shader, meshData, renderState, bindTable, vertexBufferBits) {
        const pipeline = this._getOrCreatePipeline(shader, vertexBufferBits, framebuffer.color.format);
        const vertexBuffers = this._createVertexBuffers(meshData, vertexBufferBits);
        const indexBuffer = meshData.Indices ? this._createIndexBuffer(meshData.Indices.data) : null;
        const bindGroup = this._createBindGroup(pipeline, bindTable, shader);

        const renderPass = this.cmd.beginRenderPass({
            colorAttachments: [{
                view: framebuffer.color.view,
                clearValue: { r: 0, g: 0, b: 0, a: 1 },
                loadOp: 'load',
                storeOp: 'store'
            }],
            depthStencilAttachment: framebuffer.depth ? {
                view: framebuffer.depth.view,
                depthClearValue: 1.0,
                depthLoadOp: 'load',
                depthStoreOp: 'store'
            } : undefined
        });

        renderPass.setPipeline(pipeline);
        renderPass.setBindGroup(0, bindGroup);

        let bufferIndex = 0;
        for (const buffer of vertexBuffers) {
            renderPass.setVertexBuffer(bufferIndex++, buffer);
        }

        if (indexBuffer) {
            renderPass.setIndexBuffer(indexBuffer, 'uint32');
            renderPass.drawIndexed(meshData.Indices.data.length, 1, 0, 0, 0);
        } else {
            const vertexCount = meshData.Positions ? meshData.Positions.data.length / 3 : 0;
            renderPass.draw(vertexCount, 1, 0, 0);
        }

        renderPass.end();
    }

    endFrame() {
        this.device.queue.submit([this.cmd.finish()]);
        this.cmd = this.device.createCommandEncoder();
    }

    _getOrCreatePipeline(shader, vertexBufferBits, colorFormat) {
        const cacheKey = JSON.stringify({ shader: shader.basePath, vertexBufferBits, colorFormat });
        if (this.pipelineCache.has(cacheKey)) {
            return this.pipelineCache.get(cacheKey);
        }

        const vertexBuffers = this._createVertexBufferLayouts(vertexBufferBits);

        const pipeline = this.device.createRenderPipeline({
            layout: 'auto',
            vertex: {
                module: shader.vertexModule,
                entryPoint: 'main',
                buffers: vertexBuffers
            },
            fragment: {
                module: shader.fragmentModule,
                entryPoint: 'main',
                targets: [{
                    format: colorFormat,
                    blend: {
                        color: { srcFactor: 'src-alpha', dstFactor: 'one-minus-src-alpha', operation: 'add' },
                        alpha: { srcFactor: 'one', dstFactor: 'one-minus-src-alpha', operation: 'add' }
                    }
                }]
            },
            primitive: {
                topology: 'triangle-list',
                cullMode: 'back'
            },
            depthStencil: {
                format: 'depth24plus',
                depthWriteEnabled: true,
                depthCompare: 'less'
            }
        });

        this.pipelineCache.set(cacheKey, pipeline);
        return pipeline;
    }

    _createVertexBufferLayouts(vertexBufferBits) {
        const buffers = [];
        let location = 0;

        if (vertexBufferBits.Position) {
            buffers.push({
                arrayStride: 12, // 3 * 4 bytes
                attributes: [{
                    shaderLocation: location++,
                    offset: 0,
                    format: 'float32x3'
                }]
            });
        }

        if (vertexBufferBits.Texcoord) {
            buffers.push({
                arrayStride: 8, // 2 * 4 bytes
                attributes: [{
                    shaderLocation: location++,
                    offset: 0,
                    format: 'float32x2'
                }]
            });
        }

        if (vertexBufferBits.Normal) {
            buffers.push({
                arrayStride: 12, // 3 * 4 bytes
                attributes: [{
                    shaderLocation: location++,
                    offset: 0,
                    format: 'float32x3'
                }]
            });
        }

        if (vertexBufferBits.Tangent) {
            buffers.push({
                arrayStride: 16, // 4 * 4 bytes
                attributes: [{
                    shaderLocation: location++,
                    offset: 0,
                    format: 'float32x4'
                }]
            });
        }

        return buffers;
    }

    _createVertexBuffers(meshData, vertexBufferBits) {
        const buffers = [];

        if (vertexBufferBits.Position && meshData.Positions) {
            buffers.push(this._createGPUBuffer(new Float32Array(meshData.Positions.data), GPUBufferUsage.VERTEX));
        }

        if (vertexBufferBits.Texcoord && meshData.Texcoords) {
            buffers.push(this._createGPUBuffer(new Float32Array(meshData.Texcoords.data), GPUBufferUsage.VERTEX));
        }

        if (vertexBufferBits.Normal && meshData.Normals) {
            buffers.push(this._createGPUBuffer(new Float32Array(meshData.Normals.data), GPUBufferUsage.VERTEX));
        }

        if (vertexBufferBits.Tangent && meshData.Tangents) {
            buffers.push(this._createGPUBuffer(new Float32Array(meshData.Tangents.data), GPUBufferUsage.VERTEX));
        }

        return buffers;
    }

    _createIndexBuffer(indices) {
        return this._createGPUBuffer(new Uint32Array(indices), GPUBufferUsage.INDEX);
    }

    _createGPUBuffer(data, usage) {
        const buffer = this.device.createBuffer({
            size: data.byteLength,
            usage: usage | GPUBufferUsage.COPY_DST,
            mappedAtCreation: true
        });
        new data.constructor(buffer.getMappedRange()).set(data);
        buffer.unmap();
        return buffer;
    }

    _createBindGroup(pipeline, bindTable, shader) {
        const entries = [];

        // Textures: assume each has sampler + texture
        if (bindTable.Textures) {
            for (const tex of bindTable.Textures) {
                const sampler = this.device.createSampler({
                    magFilter: 'linear',
                    minFilter: 'linear',
                });
                const textureBindingIndex = parseInt(tex.Name, 10);
                entries.push(
                    { binding: RenderContext.SAMPLER_BINDING_INDEX, resource: sampler },
                    { binding: textureBindingIndex, resource: tex.Value.color.view }
                );
            }
        }

        // Float4x4s: uniform buffers
        if (bindTable.Float4x4s) {
            for (const mat of bindTable.Float4x4s) {
                const buffer = this._createGPUBuffer(mat.Value.toFloat32Array(), GPUBufferUsage.UNIFORM);
                const bindingIndex = shader.bindMapping[mat.Name];
                entries.push({ binding: bindingIndex, resource: { buffer } });
            }
        }

        return this.device.createBindGroup({
            layout: pipeline.getBindGroupLayout(0),
            entries
        });
    }

    async _fetchText(url) {
        const resolvedUrl = new URL(url, window.location.href).href;
        const response = await fetch(resolvedUrl);
        if (!response.ok) {
            throw new Error(`Failed to fetch shader source: ${url} (${response.status})`);
        }
        return await response.text();
    }

    async _validateShaderModule(module, path) {
        if (!module || typeof module.getCompilationInfo !== 'function') {
            return;
        }

        const info = await module.getCompilationInfo();
        const errors = info.messages.filter((m) => m.type === 'error');
        if (errors.length > 0) {
            const msgText = errors.map((e) => `${e.lineNum}:${e.linePos} ${e.message}`).join('\n');
            throw new Error(`Shader compile errors in ${path}:\n${msgText}`);
        }
    }

    _loadGltfBuffer(uri, baseUrl) {
        if (!uri) {
            return Promise.reject(new Error('glTF buffer missing URI'));
        }

        if (uri.startsWith('data:')) {
            const comma = uri.indexOf(',');
            const meta = uri.substring(5, comma);
            const dataPart = uri.substring(comma + 1);
            if (meta.includes('base64')) {
                const binary = atob(dataPart);
                const array = new Uint8Array(binary.length);
                for (let i = 0; i < binary.length; ++i) {
                    array[i] = binary.charCodeAt(i);
                }
                return Promise.resolve(array.buffer);
            }
            return Promise.reject(new Error('Unsupported data URI encoding for glTF buffer'));
        }

        const resolved = new URL(uri, baseUrl).href;

        return fetch(resolved).then((r) => {
            if (r.ok) {
                return r.arrayBuffer();
            }

            // try with path from current document location (fallback if base wasn't right)
            const alt = new URL(uri, window.location.href).href;
            if (alt !== resolved) {
                return fetch(alt).then((r2) => {
                    if (r2.ok) {
                        return r2.arrayBuffer();
                    }
                    throw new Error(`Failed to load glTF buffer:
  primary: ${resolved} (${r.status})
  fallback: ${alt} (${r2.status})`);
                });
            }

            throw new Error(`Failed to load glTF buffer: ${resolved} (${r.status})`);
        });
    }

    _float4x4FromFlatArray(arr) {
        const m = new Float4x4();
        for (let row = 0; row < 4; row++) {
            for (let col = 0; col < 4; col++) {
                m.data[col][row] = arr[row * 4 + col];
            }
        }
        return m;
    }

    _identityMatrix() {
        return Float4x4.identity();
    }

    _matrixToArray4x4(m) {
        if (m instanceof Float4x4) {
            return [
                [m.data[0][0], m.data[0][1], m.data[0][2], m.data[0][3]],
                [m.data[1][0], m.data[1][1], m.data[1][2], m.data[1][3]],
                [m.data[2][0], m.data[2][1], m.data[2][2], m.data[2][3]],
                [m.data[3][0], m.data[3][1], m.data[3][2], m.data[3][3]]
            ];
        }

        const flat = (Array.isArray(m) ? m : []);
        return [
            [flat[0] || 0, flat[1] || 0, flat[2] || 0, flat[3] || 0],
            [flat[4] || 0, flat[5] || 0, flat[6] || 0, flat[7] || 0],
            [flat[8] || 0, flat[9] || 0, flat[10] || 0, flat[11] || 0],
            [flat[12] || 0, flat[13] || 0, flat[14] || 0, flat[15] || 0]
        ];
    }

    _getNodeLocalMatrix(node) {
        if (node.matrix && node.matrix.length === 16) {
            return this._float4x4FromFlatArray(node.matrix);
        }

        const t = node.translation || [0, 0, 0];
        const r = node.rotation || [0, 0, 0, 1];
        const s = node.scale || [1, 1, 1];

        const tm = this._translationMatrix(t);
        const rm = this._rotationMatrixFromQuaternion(r);
        const sm = this._scaleMatrix(s);

        const rs = MathTypeOps.MultiplyFloat4x4(rm, sm);
        return MathTypeOps.MultiplyFloat4x4(tm, rs);
    }

    _translationMatrix(t) {
        const m = new Float4x4();
        m.data[3][0] = t[0];
        m.data[3][1] = t[1];
        m.data[3][2] = t[2];
        return m;
    }

    _scaleMatrix(s) {
        const m = new Float4x4();
        m.data[0][0] = s[0];
        m.data[1][1] = s[1];
        m.data[2][2] = s[2];
        return m;
    }

    _rotationMatrixFromQuaternion(q) {
        const [x, y, z, w] = q;
        const xx = x * x;
        const yy = y * y;
        const zz = z * z;
        const xy = x * y;
        const xz = x * z;
        const yz = y * z;
        const wx = w * x;
        const wy = w * y;
        const wz = w * z;

        const flat = [
            1 - 2 * (yy + zz), 2 * (xy + wz), 2 * (xz - wy), 0,
            2 * (xy - wz), 1 - 2 * (xx + zz), 2 * (yz + wx), 0,
            2 * (xz + wy), 2 * (yz - wx), 1 - 2 * (xx + yy), 0,
            0, 0, 0, 1
        ];

        return this._float4x4FromFlatArray(flat);
    }

    _extractPrimitiveMeshData(primitive, gltf, buffers) {
        if (!uri) {
            return Promise.reject(new Error('glTF buffer missing URI'));
        }

        if (uri.startsWith('data:')) {
            const comma = uri.indexOf(',');
            const meta = uri.substring(5, comma);
            const dataPart = uri.substring(comma + 1);
            if (meta.includes('base64')) {
                const binary = atob(dataPart);
                const array = new Uint8Array(binary.length);
                for (let i = 0; i < binary.length; ++i) {
                    array[i] = binary.charCodeAt(i);
                }
                return Promise.resolve(array.buffer);
            }
            return Promise.reject(new Error('Unsupported data URI encoding for glTF buffer'));
        }

        const resolved = new URL(uri, baseUrl).href;
        return fetch(resolved).then((r) => {
            if (!r.ok) {
                throw new Error(`Failed to load glTF buffer: ${resolved}`);
            }
            return r.arrayBuffer();
        });
    }


    _extractPrimitiveMeshData(primitive, gltf, buffers) {
        const attributeGet = (attrName) => {
            if (!primitive.attributes || primitive.attributes[attrName] === undefined) return null;
            const accessorIndex = primitive.attributes[attrName];
            const data = this._readAccessorData(accessorIndex, gltf, buffers);
            return { data };
        };

        const positions = attributeGet('POSITION');
        const normals = attributeGet('NORMAL');
        const texcoords = attributeGet('TEXCOORD_0');
        const tangents = attributeGet('TANGENT');

        let indices = null;
        if (primitive.indices !== undefined) {
            const indexArray = this._readAccessorData(primitive.indices, gltf, buffers);
            indices = { data: indexArray };
        } else if (positions) {
            const count = positions.data.length / 3;
            indices = { data: Array.from({ length: count }, (_, i) => i) };
        }

        return {
            PrimitiveCount: 1,
            Positions: positions || { data: [] },
            Texcoords: texcoords || { data: [] },
            Normals: normals || { data: [] },
            Tangents: tangents || { data: [] },
            Indices: indices || { data: [] }
        };
    }

    _readAccessorData(accessorIndex, gltf, buffers) {
        const accessor = gltf.accessors[accessorIndex];
        if (!accessor || accessor.bufferView === undefined) {
            return [];
        }

        const bufferView = gltf.bufferViews[accessor.bufferView];
        if (!bufferView) {
            return [];
        }

        const buffer = buffers[bufferView.buffer];
        if (!buffer) {
            return [];
        }

        const componentSize = this._componentTypeSize(accessor.componentType);
        const numComponents = this._typeNumComponents(accessor.type);
        const byteOffset = (bufferView.byteOffset || 0) + (accessor.byteOffset || 0);
        const byteStride = bufferView.byteStride || (componentSize * numComponents);
        const count = accessor.count || 0;
        const data = [];
        const view = new DataView(buffer);

        for (let i = 0; i < count; i++) {
            const elementBase = byteOffset + i * byteStride;
            for (let c = 0; c < numComponents; c++) {
                const componentOffset = elementBase + c * componentSize;
                data.push(this._readComponent(view, componentOffset, accessor.componentType));
            }
        }

        return data;
    }

    _componentTypeSize(type) {
        switch (type) {
            case 5120: return 1;
            case 5121: return 1;
            case 5122: return 2;
            case 5123: return 2;
            case 5125: return 4;
            case 5126: return 4;
            default: throw new Error(`Unsupported accessor componentType ${type}`);
        }
    }

    _typeNumComponents(type) {
        switch (type) {
            case 'SCALAR': return 1;
            case 'VEC2': return 2;
            case 'VEC3': return 3;
            case 'VEC4': return 4;
            case 'MAT2': return 4;
            case 'MAT3': return 9;
            case 'MAT4': return 16;
            default: throw new Error(`Unsupported accessor type ${type}`);
        }
    }

    _readComponent(view, byteOffset, componentType) {
        switch (componentType) {
            case 5120: return view.getInt8(byteOffset);
            case 5121: return view.getUint8(byteOffset);
            case 5122: return view.getInt16(byteOffset, true);
            case 5123: return view.getUint16(byteOffset, true);
            case 5125: return view.getUint32(byteOffset, true);
            case 5126: return view.getFloat32(byteOffset, true);
            default: throw new Error(`Unsupported componentType ${componentType}`);
        }
    }

    _extractPrimitiveMaterial(primitive, gltf, baseUrl) {
        const defaultMaterial = {
            Type: 'Opaque',
            AlbedoFactor: [1.0, 1.0, 1.0],
            MetallicFactor: 1.0,
            RoughnessFactor: 1.0,
            Albedo: null,
            Normal: null,
            MetallicRoughness: null
        };

        const materialIndex = primitive.material;
        const materialDef = (gltf.materials || [])[materialIndex];
        if (!materialDef) {
            return defaultMaterial;
        }

        const pbr = materialDef.pbrMetallicRoughness || {};
        const baseColor = pbr.baseColorFactor || [1, 1, 1, 1];

        const getTexturePath = (textureInfo) => {
            if (!textureInfo || textureInfo.index === undefined) {
                return null;
            }
            const texture = (gltf.textures || [])[textureInfo.index];
            if (!texture) {
                return null;
            }
            const image = (gltf.images || [])[texture.source];
            if (!image) {
                return null;
            }
            if (image.uri && !image.uri.startsWith('data:')) {
                return new URL(image.uri, baseUrl).href;
            }
            return image.uri || null;
        };

        const alphaMode = (materialDef.alphaMode || 'OPAQUE').toUpperCase();
        return {
            Type: alphaMode === 'MASK' ? 'AlphaDiscard' : alphaMode === 'BLEND' ? 'AlphaBlend' : 'Opaque',
            AlbedoFactor: [baseColor[0], baseColor[1], baseColor[2]],
            MetallicFactor: pbr.metallicFactor !== undefined ? pbr.metallicFactor : 1.0,
            RoughnessFactor: pbr.roughnessFactor !== undefined ? pbr.roughnessFactor : 1.0,
            Albedo: { path: getTexturePath(pbr.baseColorTexture) },
            Normal: { path: getTexturePath(materialDef.normalTexture) },
            MetallicRoughness: { path: getTexturePath(pbr.metallicRoughnessTexture) }
        };
    }

    _createTexture(width, height, format, textureUsage) {
        const gpuTexture = this.device.createTexture({
            size: [width, height],
            format: format,
            usage: textureUsage
        });

        return {
            gpuTexture,
            view: gpuTexture.createView(),
            width,
            height,
            format: format
        };
    }

    _presentTexture(texture) {
        if (!texture || !texture.gpuTexture) {
            console.warn('PresentTexture: Invalid texture');
            return;
        }

        const swapChainFormat = this.context.getCurrentTexture().format;
        if (texture.gpuTexture.format === swapChainFormat) {
            
            this.cmd.copyTextureToTexture(
                { texture: texture.gpuTexture },
                { texture: this.context.getCurrentTexture() },
                [texture.width, texture.height]
            );
        } else {
            const pipeline = this._getOrCreateBlitPipeline(this.device, this.context.getCurrentTexture().format);
            const renderPass = this.cmd.beginRenderPass({
                colorAttachments: [{
                    view: this.context.getCurrentTexture().createView(),
                    loadOp: 'clear',
                    storeOp: 'store',
                    clearValue: { r: 0, g: 0, b: 0, a: 1 }
                }]
            });
            renderPass.setPipeline(pipeline);
            const sampler = this.device.createSampler({
                magFilter: 'linear',
                minFilter: 'linear',
            });

            const bindGroup = this.device.createBindGroup({
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
    }

    _getOrCreateBlitPipeline(device, targetFormat) {
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

        const pipeline = this.device.createRenderPipeline({
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
}
