#include <emscripten.h>
#include <emscripten/wasm_worker.h>

#include "wisdom-chess/engine/logger.hpp"
#include "wisdom-chess/ui/wasm/web_logger.hpp"

emscripten_wasm_worker_t engine_thread;

int main()
{
    wisdom::setEmergencyLogger (wisdom::worker::makeLogger());
    wisdom::installEmergencyTerminateHandler();

    // Initialize worker thread:
    engine_thread = emscripten_malloc_wasm_worker (/*stack size: */ 8192 * 1024);
}
