export class GLTFLoader {
    
    async loadFromPath(path) {
        try {
            const response = await fetch(path);
            if (!response.ok) {
                throw new Error(`Failed to load file: ${response.statusText}`);
            }

            const arrayBuffer = await response.arrayBuffer();
            const filename = path.split('/').pop();
            
            return await this._parse(arrayBuffer, filename, path);
        } catch (error) {
            throw new Error(`Error loading GLTF from ${path}: ${error.message}`);
        }
    }

    async _parse(arrayBuffer, filename, basePath) {
        let gltf, buffers;

        if (filename.endsWith('.glb')) {
            const result = this._parseGLB(arrayBuffer);
            gltf = result.json;
            buffers = result.buffers;
        } else {
            const result = await this._parseGLTF(arrayBuffer, basePath);
            gltf = result.json;
            buffers = result.buffers;
        }

        return { json: gltf, buffers };
    }

    _parseGLB(arrayBuffer) {
        const view = new DataView(arrayBuffer);
        const magic = view.getUint32(0, true);
        
        if (magic !== 0x46546C67) { // 'glTF'
            throw new Error('Invalid GLB file');
        }

        // Read JSON chunk
        const jsonLength = view.getUint32(12, true);
        const jsonStart = 20;
        const jsonData = new Uint8Array(arrayBuffer, jsonStart, jsonLength);
        const gltf = JSON.parse(new TextDecoder().decode(jsonData));

        // Read binary chunk
        const binStart = jsonStart + jsonLength + 8;
        const binLength = view.getUint32(jsonStart + jsonLength + 4, true);
        const buffers = [arrayBuffer.slice(binStart, binStart + binLength)];

        return { json: gltf, buffers };
    }

    async _parseGLTF(arrayBuffer, basePath) {
        const jsonData = new TextDecoder().decode(arrayBuffer);
        const gltf = JSON.parse(jsonData);
        const buffers = [];
        
        const baseDir = basePath.substring(0, basePath.lastIndexOf('/') + 1);

        if (gltf.buffers) {
            for (const buffer of gltf.buffers) {
                if (buffer.uri) {
                    if (buffer.uri.startsWith('data:')) {
                        // Base64 embedded buffer
                        buffers.push(this._decodeBase64Buffer(buffer.uri));
                    } else {
                        // External buffer file
                        buffers.push(await this._loadExternalBuffer(baseDir + buffer.uri));
                    }
                }
            }
        }

        return { json: gltf, buffers };
    }

    _decodeBase64Buffer(dataUri) {
        const base64Data = dataUri.split(',')[1];
        const binaryString = atob(base64Data);
        const bytes = new Uint8Array(binaryString.length);
        for (let i = 0; i < binaryString.length; i++) {
            bytes[i] = binaryString.charCodeAt(i);
        }
        return bytes.buffer;
    }

    async _loadExternalBuffer(bufferPath) {
        const response = await fetch(bufferPath);
        if (!response.ok) {
            throw new Error(`Failed to load buffer: ${bufferPath}`);
        }
        return await response.arrayBuffer();
    }

    createMeshBuffers(gltfData) {
        const { json, buffers } = gltfData;
        
        if (!json.meshes || json.meshes.length === 0) {
            throw new Error('No meshes found in GLTF file');
        }

        const mesh = json.meshes[0];
        const primitives = [];
        let totalVertices = 0;
        
        // Calculate bounding box
        const bounds = {
            min: [Infinity, Infinity, Infinity],
            max: [-Infinity, -Infinity, -Infinity]
        };

        for (const primitive of mesh.primitives) {
            const meshPrimitive = this._createPrimitive(json, buffers, primitive, bounds);
            if (meshPrimitive) {
                primitives.push(meshPrimitive);
                totalVertices += meshPrimitive.vertexCount;
            }
        }

        // Calculate model info
        const center = [
            (bounds.min[0] + bounds.max[0]) / 2,
            (bounds.min[1] + bounds.max[1]) / 2,
            (bounds.min[2] + bounds.max[2]) / 2
        ];
        
        const size = [
            bounds.max[0] - bounds.min[0],
            bounds.max[1] - bounds.min[1],
            bounds.max[2] - bounds.min[2]
        ];
        
        const maxSize = Math.max(...size);

        return {
            primitives,
            vertexCount: totalVertices,
            center,
            size: maxSize
        };
    }

    _createPrimitive(json, buffers, primitive, bounds) {
        const positions = this._getAccessorData(json, buffers, primitive.attributes.POSITION);
        const normals = this._getAccessorData(json, buffers, primitive.attributes.NORMAL);
        const indices = primitive.indices !== undefined 
            ? this._getAccessorData(json, buffers, primitive.indices)
            : null;

        if (!positions) {
            console.warn('Primitive missing position data, skipping');
            return null;
        }

        // Update bounding box
        this._updateBounds(positions, bounds);

        // Create position buffer
        const positionBuffer = this.renderer.createBuffer(
            new Float32Array(positions),
            GPUBufferUsage.VERTEX
        );

        // Create or generate normal buffer
        const normalBuffer = normals
            ? this.renderer.createBuffer(new Float32Array(normals), GPUBufferUsage.VERTEX)
            : this.renderer.createBuffer(this._generateNormals(positions, indices), GPUBufferUsage.VERTEX);

        // Create index buffer if present
        let indexBuffer = null;
        let indexCount = 0;
        let indexFormat = 'uint16';

        if (indices) {
            indexBuffer = this.renderer.createBuffer(indices, GPUBufferUsage.INDEX);
            indexFormat = indices instanceof Uint16Array ? 'uint16' : 'uint32';
            indexCount = indices.length;
        }

        const vertexCount = positions.length / 3;

        return {
            positionBuffer,
            normalBuffer,
            indexBuffer,
            indexCount,
            vertexCount,
            indexFormat
        };
    }

    _updateBounds(positions, bounds) {
        for (let i = 0; i < positions.length; i += 3) {
            bounds.min[0] = Math.min(bounds.min[0], positions[i]);
            bounds.min[1] = Math.min(bounds.min[1], positions[i + 1]);
            bounds.min[2] = Math.min(bounds.min[2], positions[i + 2]);
            bounds.max[0] = Math.max(bounds.max[0], positions[i]);
            bounds.max[1] = Math.max(bounds.max[1], positions[i + 1]);
            bounds.max[2] = Math.max(bounds.max[2], positions[i + 2]);
        }
    }

    _getAccessorData(gltf, buffers, accessorIndex) {
        if (accessorIndex === undefined) return null;

        const accessor = gltf.accessors[accessorIndex];
        if (!accessor) return null;

        const bufferView = gltf.bufferViews[accessor.bufferView];
        if (!bufferView) return null;

        const buffer = buffers[bufferView.buffer];
        if (!buffer) return null;

        const offset = (bufferView.byteOffset || 0) + (accessor.byteOffset || 0);
        const componentSize = this._getComponentSize(accessor.componentType);
        const componentCount = this._getComponentCount(accessor.type);
        const totalSize = accessor.count * componentCount * componentSize;

        const arrayBuffer = buffer.slice(offset, offset + totalSize);

        // Map component type to TypedArray
        const typeMap = {
            5126: Float32Array,  // FLOAT
            5123: Uint16Array,   // UNSIGNED_SHORT
            5125: Uint32Array,   // UNSIGNED_INT
            5120: Int8Array,     // BYTE
            5121: Uint8Array,    // UNSIGNED_BYTE
            5122: Int16Array     // SHORT
        };

        const ArrayType = typeMap[accessor.componentType];
        if (!ArrayType) {
            throw new Error(`Unsupported component type: ${accessor.componentType}`);
        }

        return new ArrayType(arrayBuffer);
    }

    _getComponentCount(type) {
        const counts = {
            'SCALAR': 1,
            'VEC2': 2,
            'VEC3': 3,
            'VEC4': 4,
            'MAT2': 4,
            'MAT3': 9,
            'MAT4': 16
        };
        return counts[type] || 1;
    }

    _getComponentSize(componentType) {
        const sizes = {
            5120: 1,  // BYTE
            5121: 1,  // UNSIGNED_BYTE
            5122: 2,  // SHORT
            5123: 2,  // UNSIGNED_SHORT
            5125: 4,  // UNSIGNED_INT
            5126: 4   // FLOAT
        };
        return sizes[componentType] || 4;
    }

    _generateNormals(positions, indices) {
        const normals = new Float32Array(positions.length);
        
        // Simple face normal generation
        for (let i = 0; i < positions.length; i += 9) {
            const v0 = [positions[i], positions[i + 1], positions[i + 2]];
            const v1 = [positions[i + 3], positions[i + 4], positions[i + 5]];
            const v2 = [positions[i + 6], positions[i + 7], positions[i + 8]];

            const edge1 = [v1[0] - v0[0], v1[1] - v0[1], v1[2] - v0[2]];
            const edge2 = [v2[0] - v0[0], v2[1] - v0[1], v2[2] - v0[2]];

            // Cross product
            const normal = [
                edge1[1] * edge2[2] - edge1[2] * edge2[1],
                edge1[2] * edge2[0] - edge1[0] * edge2[2],
                edge1[0] * edge2[1] - edge1[1] * edge2[0]
            ];

            // Normalize
            const len = Math.sqrt(normal[0] ** 2 + normal[1] ** 2 + normal[2] ** 2);
            if (len > 0) {
                normal[0] /= len;
                normal[1] /= len;
                normal[2] /= len;
            }

            // Assign to all three vertices
            for (let j = 0; j < 3; j++) {
                normals[i + j * 3] = normal[0];
                normals[i + j * 3 + 1] = normal[1];
                normals[i + j * 3 + 2] = normal[2];
            }
        }

        return normals;
    }
}
