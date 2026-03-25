export class Float2 {
    constructor(x = 0, y = 0) {
        this.x = x;
        this.y = y;
    }

    static zero() {
        return new Float2(0, 0);
    }

    static one() {
        return new Float2(1, 1);
    }
}

export class Float3 {
    constructor(x = 0, y = 0, z = 0) {
        this.x = x;
        this.y = y;
        this.z = z;
    }

    static zero() {
        return new Float3(0, 0, 0);
    }

    static one() {
        return new Float3(1, 1, 1);
    }

    static up() {
        return new Float3(0, 1, 0);
    }

    static forward() {
        return new Float3(0, 0, 1);
    }

    static right() {
        return new Float3(1, 0, 0);
    }
}

export class Float4 {
    constructor(x = 0, y = 0, z = 0, w = 0) {
        this.x = x;
        this.y = y;
        this.z = z;
        this.w = w;
    }

    static zero() {
        return new Float4(0, 0, 0, 0);
    }

    static one() {
        return new Float4(1, 1, 1, 1);
    }
}

export class Float4x4 {
    constructor() {
        // Column-major identity matrix
        this.data = [
            [1, 0, 0, 0],
            [0, 1, 0, 0],
            [0, 0, 1, 0],
            [0, 0, 0, 1]
        ];
    }

    static identity() {
        return new Float4x4();
    }

    toFloat32Array() {
        const result = new Float32Array(16);
        let index = 0;
        for (let col = 0; col < 4; col++) {
            for (let row = 0; row < 4; row++) {
                result[index++] = this.data[row][col];
            }
        }
        return result;
    }
}

export const DepthTest = {
    NEVER: 'never',
    LESS: 'less',
    EQUAL: 'equal',
    LEQUAL: 'less-equal',
    GREATER: 'greater',
    NOTEQUAL: 'not-equal',
    GEQUAL: 'greater-equal',
    ALWAYS: 'always'
};

export class RenderState {
    constructor() {
        this.depthTest = DepthTest.LESS;
        this.depthWrite = true;
        this.cullMode = 'back';
        this.blendEnabled = false;
    }
}

export class MathTypeOps {
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

    static TranslateFloat4x4(baseMatrix, translation) {
        const matrix = baseMatrix ? JSON.parse(JSON.stringify(baseMatrix)) : new Float4x4();
        
        matrix.data[3][0] = translation.x;
        matrix.data[3][1] = translation.y;
        matrix.data[3][2] = translation.z;

        return matrix;
    }

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

    static ScaleFloat4x4(baseMatrix, scale) {
        const matrix = baseMatrix ? JSON.parse(JSON.stringify(baseMatrix)) : new Float4x4();
        
        matrix.data[0][0] *= scale.x;
        matrix.data[1][1] *= scale.y;
        matrix.data[2][2] *= scale.z;

        return matrix;
    }

    static VectorNormalize(vec) {
        if (!vec) return vec;

        if (!(vec instanceof Float2 || vec instanceof Float3 || vec instanceof Float4)) {
            throw new Error('VectorNormalize expects Float2, Float3 or Float4');
        }

        const x = vec.x;
        const y = vec.y;
        const z = vec instanceof Float3 || vec instanceof Float4 ? vec.z : 0;
        const w = vec instanceof Float4 ? vec.w : 0;

        const length = Math.hypot(x, y, z, w);

        if (length === 0) {
            if (vec instanceof Float2) return Float2.zero();
            if (vec instanceof Float3) return Float3.zero();
            if (vec instanceof Float4) return Float4.zero();
        }

        if (vec instanceof Float2) return new Float2(x / length, y / length);
        if (vec instanceof Float3) return new Float3(x / length, y / length, z / length);
        if (vec instanceof Float4) return new Float4(x / length, y / length, z / length, w / length);

        return vec;
    }

    static VectorCross(a, b) {
        if (!a || !b) return null;

        if (a instanceof Float2 && b instanceof Float2) {
            // 2D cross product returns scalar (z-component)
            return a.x * b.y - a.y * b.x;
        }

        if (a instanceof Float3 && b instanceof Float3) {
            // 3D cross product
            return new Float3(
                a.y * b.z - a.z * b.y,
                a.z * b.x - a.x * b.z,
                a.x * b.y - a.y * b.x
            );
        }

        if (a instanceof Float4 && b instanceof Float4) {
            // 4D cross product (use x, y, z components)
            return new Float4(
                a.y * b.z - a.z * b.y,
                a.z * b.x - a.x * b.z,
                a.x * b.y - a.y * b.x,
                0
            );
        }

        throw new Error('VectorCross: both vectors must be the same type (Float2, Float3, or Float4)');
    }

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

    static VectorAdd(a, b) {
        if (!a || !b) return null;

        if (a.constructor !== b.constructor) {
            throw new Error('VectorAdd: both vectors must be the same type (Float2, Float3, or Float4)');
        }

        if (a instanceof Float2) {
            return new Float2(a.x + b.x, a.y + b.y);
        }

        if (a instanceof Float3) {
            return new Float3(a.x + b.x, a.y + b.y, a.z + b.z);
        }

        if (a instanceof Float4) {
            return new Float4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
        }

        throw new Error('VectorAdd: unsupported vector type');
    }

    static VectorSubtract(a, b) {
        if (!a || !b) return null;

        if (a.constructor !== b.constructor) {
            throw new Error('VectorSubtract: both vectors must be the same type (Float2, Float3, or Float4)');
        }

        if (a instanceof Float2) {
            return new Float2(a.x - b.x, a.y - b.y);
        }

        if (a instanceof Float3) {
            return new Float3(a.x - b.x, a.y - b.y, a.z - b.z);
        }

        if (a instanceof Float4) {
            return new Float4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
        }

        throw new Error('VectorSubtract: unsupported vector type');
    }

    static VectorMultiply(a, b) {
        if (!a || !b) return null;

        if (a.constructor !== b.constructor) {
            throw new Error('VectorMultiply: both vectors must be the same type (Float2, Float3, or Float4)');
        }

        if (a instanceof Float2) {
            return new Float2(a.x * b.x, a.y * b.y);
        }

        if (a instanceof Float3) {
            return new Float3(a.x * b.x, a.y * b.y, a.z * b.z);
        }

        if (a instanceof Float4) {
            return new Float4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
        }

        throw new Error('VectorMultiply: unsupported vector type');
    }

    static VectorDivide(a, b) {
        if (!a || !b) return null;

        if (a.constructor !== b.constructor) {
            throw new Error('VectorDivide: both vectors must be the same type (Float2, Float3, or Float4)');
        }

        if (a instanceof Float2) {
            return new Float2(a.x / b.x, a.y / b.y);
        }

        if (a instanceof Float3) {
            return new Float3(a.x / b.x, a.y / b.y, a.z / b.z);
        }

        if (a instanceof Float4) {
            return new Float4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
        }

        throw new Error('VectorDivide: unsupported vector type');
    }
}