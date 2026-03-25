export class InputManager {

    constructor() {
        this.inputState = {
            keysPressed : new Set(),
            keysReleased : new Set(),
            keysDown : new Set(),

            clear : function() {
                this.keysPressed.clear();
                this.keysReleased.clear();
                this.keysDown.clear();
            }
        };

        this.inputCallbacks = {
            keyPressed : new Map(),
            keyReleased : new Map(),
            keyDown : new Map(),
        };
    }

    init() {
        document.addEventListener('keydown', (event) => {
            const keyString = this._getKeyString(event);
            
            this.inputState.keysDown.add(keyString);

            if (!event.repeat) {
                 this.inputState.keysPressed.add(keyString);
            }
        });
        
        document.addEventListener('keyup', (event) => {
            const keyString = this._getKeyString(event);

            this.inputState.keysDown.delete(keyString);
            this.inputState.keysReleased.add(keyString);
        });
        
        window.addEventListener('blur', () => {
            this.inputState.clear();
        });
    }

    registerKeyDown(keyString, callback) {
        if (!this.inputCallbacks.keyDown.has(keyString)) {
            this.inputCallbacks.keyDown.set(keyString, []);
        }
        this.inputCallbacks.keyDown.get(keyString).push(callback);
    }

    registerKeyReleased(keyString, callback) {
        if (!this.inputCallbacks.keyReleased.has(keyString)) {
            this.inputCallbacks.keyReleased.set(keyString, []);
        }
        this.inputCallbacks.keyReleased.get(keyString).push(callback);
    }

    registerKeyPressed(keyString, callback) {
        if (!this.inputCallbacks.keyPressed.has(keyString)) {
            this.inputCallbacks.keyPressed.set(keyString, []);
        }
        this.inputCallbacks.keyPressed.get(keyString).push(callback);
    }

    processInputs() {
        for (const keyString of this.inputState.keysDown) {
            if (this.inputCallbacks.keyDown.has(keyString)) {
                for (const callback of this.inputCallbacks.keyDown.get(keyString)) {
                    callback(keyString);
                }
            }
        }

        for (const keyString of this.inputState.keysReleased) {
            if (this.inputCallbacks.keyReleased.has(keyString)) {
                for (const callback of this.inputCallbacks.keyReleased.get(keyString)) {
                    callback(keyString);
                }
            }
        }

        for (const keyString of this.inputState.keysPressed) {
            if (this.inputCallbacks.keyPressed.has(keyString)) {
                for (const callback of this.inputCallbacks.keyPressed.get(keyString)) {
                    callback(keyString);
                }
            }
        }

        this.inputState.keysPressed.clear();
        this.inputState.keysReleased.clear();
    }

    // Create a key string that includes modifiers
    // e.g., "KeyW", "Ctrl+KeyS", "Ctrl+Shift+KeyZ"
    _getKeyString(event) {
        const parts = [];
        
        if (event.ctrlKey) parts.push('Ctrl');
        if (event.shiftKey) parts.push('Shift');
        if (event.altKey) parts.push('Alt');
        if (event.metaKey) parts.push('Meta');
        
        parts.push(event.code);
        
        return parts.join('+');
    }
}