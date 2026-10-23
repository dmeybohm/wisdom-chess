// Every spelling of an include names the same header to the preprocessor.
#  include "wisdom-chess/engine/generate.hpp"
#include <wisdom-chess/engine/board.hpp>
  #include   <wisdom-chess/engine/threats.hpp>
#include /* not "wisdom-chess/engine/board.hpp" */ "wisdom-chess/engine/game.hpp"
/* about "wisdom-chess/engine/board.hpp" */
#include "wisdom-chess/engine/game.hpp"
#include \
    "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/history.hpp" // not "wisdom-chess/engine/game.hpp"
#define ENGINE_HEADER "wisdom-chess/engine/board.hpp"
#include ENGINE_HEADER
#if 0
#include "wisdom-chess/engine/search.hpp"
#endif
#pragma message("wisdom-chess/engine/board.hpp")
