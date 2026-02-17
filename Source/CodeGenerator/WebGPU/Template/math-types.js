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
                result[index++] = this.data[col][row];
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
