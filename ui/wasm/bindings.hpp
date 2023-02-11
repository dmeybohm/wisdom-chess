//
// Created by dmeybohm on 2/11/2023.
//

#ifndef WISDOMCHESS_BINDINGS_HPP
#define WISDOMCHESS_BINDINGS_HPP

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

#endif // WISDOMCHESS_BINDINGS_HPP
