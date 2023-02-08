#include <emscripten.h>
#include <emscripten/wasm_worker.h>

extern "C"
{
  EM_JS(void, console_log, (const char* str), {
    console.log(UTF8ToString(str))
  })

  EMSCRIPTEN_KEEPALIVE void run_in_worker ()
  {
    console_log ("Hello from a worker!\n");
  }
}
// The engine thread manager receives messages from the main thread
emscripten_wasm_worker_t engine_thread_manager;
emscripten_wasm_worker_t engine_thread;

int main()
{
  // Initialize the worker threads:
  engine_thread_manager = emscripten_malloc_wasm_worker (/*stack size: */1024);
  engine_thread = emscripten_malloc_wasm_worker (/*stack size: */1024);
}

