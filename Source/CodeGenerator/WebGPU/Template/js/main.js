import { RenderNodesEngine } from './engine.js';
import { NodeGraphCodeGen } from './nodegraph-codegen.js';

async function main() {
    try {
        const codeGen = new NodeGraphCodeGen();
        const engine = new RenderNodesEngine('canvas', codeGen);
        
        const initialized = await engine.initialize();
        if (!initialized) {
            console.error('Failed to initialize engine');
            return;
        }
        
        engine.start();
        
        window.engine = engine;
        window.codegen = codeGen;
        
    } catch (error) {
        console.error('Application error:', error);
    }
}

if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', main);
} else {
    main();
}
