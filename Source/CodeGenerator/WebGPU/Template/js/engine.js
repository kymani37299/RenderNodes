import { InputManager } from "./input-manager.js"
import { RenderContext } from "./render-context.js";

export class RenderNodesEngine {
    static instance = null;

    constructor(canvasId, codeGenClass) {
        if (RenderNodesEngine.instance) {
            return RenderNodesEngine.instance;
        }

        this.canvas = document.getElementById(canvasId);
        if (!this.canvas) {
            throw new Error(`Canvas element with id "${canvasId}" not found`);
        }

        this.renderer = null;
        this.codeGenClass = codeGenClass;
        this.isRunning = false;
        this.inputManager = new InputManager();

        RenderNodesEngine.instance = this;
    }

    async initialize() {
        try {
            this.renderer = new RenderContext();
            await this.renderer.initialize(this.canvas);
            
            window.addEventListener('resize', () => this._handleResize());
            this._handleResize();
            this.inputManager.init();
            await this._initializeCodeGen();
            return true;
        } catch (error) {
            console.error(error);
            return false;
        }
    }

    start() {
        if (this.isRunning) return;
        this.isRunning = true;
        this._renderLoop();
    }

    stop() {
        this.isRunning = false;
    }

    async _initializeCodeGen() {
        try {
            this.codeGenClass.RegisterInputs();
            await this.codeGenClass.InitializeVariables();
            await this.codeGenClass.OnStart();
            this.renderer.endFrame();
        } catch (error) {
            console.error('CodeGen initialization failed:', error);
            throw error;
        }
    }

    _renderLoop() {
        if (!this.isRunning) return;

        try {
            this.inputManager.processInputs();
            this.codeGenClass.OnUpdate();
            this.renderer.endFrame();
        } catch (error) {
            console.error('CodeGen OnUpdate error:', error);
        }
        requestAnimationFrame(() => this._renderLoop());
    }

    _handleResize() {
        const width = window.innerWidth;
        const height = window.innerHeight;
        this.renderer.resize(width, height);
    }
}
