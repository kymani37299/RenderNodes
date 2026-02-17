/**
 * Static Resources - Built-in meshes and resources for codegen
 */

import { RenderContext } from './render-context.js';

class StaticResources {
    constructor() {
        this._cubeMesh = null;
        this._quadMesh = null;
    }

    get CubeMesh() {
        if (!this._cubeMesh) {
            this._cubeMesh = this._createCubeMesh();
        }
        return this._cubeMesh;
    }

    get QuadMesh() {
        if (!this._quadMesh) {
            this._quadMesh = this._createQuadMesh();
        }
        return this._quadMesh;
    }

    _createCubeMesh() {
        const ctx = RenderContext.getInstance();
        const device = ctx.getDevice();

        // Cube vertices (position only, for skybox)
        const positions = new Float32Array([
            // Front face
            -1, -1,  1,  1, -1,  1,  1,  1,  1,
            -1, -1,  1,  1,  1,  1, -1,  1,  1,
            // Back face
            -1, -1, -1, -1,  1, -1,  1,  1, -1,
            -1, -1, -1,  1,  1, -1,  1, -1, -1,
            // Top face
            -1,  1, -1, -1,  1,  1,  1,  1,  1,
            -1,  1, -1,  1,  1,  1,  1,  1, -1,
            // Bottom face
            -1, -1, -1,  1, -1, -1,  1, -1,  1,
            -1, -1, -1,  1, -1,  1, -1, -1,  1,
            // Right face
             1, -1, -1,  1,  1, -1,  1,  1,  1,
             1, -1, -1,  1,  1,  1,  1, -1,  1,
            // Left face
            -1, -1, -1, -1, -1,  1, -1,  1,  1,
            -1, -1, -1, -1,  1,  1, -1,  1, -1
        ]);

        // Generate UVs (for cube mapping)
        const texcoords = new Float32Array(positions.length / 3 * 2);
        for (let i = 0; i < positions.length / 3; i++) {
            texcoords[i * 2] = 0;
            texcoords[i * 2 + 1] = 0;
        }

        // Generate normals
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

        const positionBuffer = this._createBuffer(device, positions, GPUBufferUsage.VERTEX);
        const texcoordBuffer = this._createBuffer(device, texcoords, GPUBufferUsage.VERTEX);
        const normalBuffer = this._createBuffer(device, normals, GPUBufferUsage.VERTEX);

        return {
            positions: positionBuffer,
            texcoords: texcoordBuffer,
            normals: normalBuffer,
            indices: null,
            indexCount: 0,
            vertexCount: positions.length / 3,
            indexFormat: 'uint16'
        };
    }

    _createQuadMesh() {
        const ctx = RenderContext.getInstance();
        const device = ctx.getDevice();

        const positions = new Float32Array([
            -1, -1, 0,  1, -1, 0,  1,  1, 0,
            -1, -1, 0,  1,  1, 0, -1,  1, 0
        ]);

        const texcoords = new Float32Array([
            0, 1,  1, 1,  1, 0,
            0, 1,  1, 0,  0, 0
        ]);

        const normals = new Float32Array([
            0, 0, 1,  0, 0, 1,  0, 0, 1,
            0, 0, 1,  0, 0, 1,  0, 0, 1
        ]);

        const positionBuffer = this._createBuffer(device, positions, GPUBufferUsage.VERTEX);
        const texcoordBuffer = this._createBuffer(device, texcoords, GPUBufferUsage.VERTEX);
        const normalBuffer = this._createBuffer(device, normals, GPUBufferUsage.VERTEX);

        return {
            positions: positionBuffer,
            texcoords: texcoordBuffer,
            normals: normalBuffer,
            indices: null,
            indexCount: 0,
            vertexCount: positions.length / 3,
            indexFormat: 'uint16'
        };
    }

    _createBuffer(device, data, usage) {
        const buffer = device.createBuffer({
            size: data.byteLength,
            usage,
            mappedAtCreation: true
        });

        new Float32Array(buffer.getMappedRange()).set(data);
        buffer.unmap();
        return buffer;
    }
}

// Export singleton instance
export const staticResources = new StaticResources();
