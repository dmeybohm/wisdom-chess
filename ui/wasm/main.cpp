#include <emscripten.h>
#include <emscripten/wasm_worker.h>

// The engine thread manager receives messages from the main thread
emscripten_wasm_worker_t engine_thread_manager;
emscripten_wasm_worker_t engine_thread;

int main()
{
  // Initialize the worker threads:
  engine_thread_manager = emscripten_malloc_wasm_worker (/*stack size: */8192 * 1024);
  engine_thread = emscripten_malloc_wasm_worker (/*stack size: */8192 * 1024);
}

