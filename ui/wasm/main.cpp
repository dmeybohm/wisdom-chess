#include <emscripten.h>
#include <emscripten/wasm_worker.h>

emscripten_wasm_worker_t engine_thread;

int main()
{
    // Initialize worker thread:
    engine_thread = emscripten_malloc_wasm_worker (/*stack size: */ 8192 * 1024);
}

