#ifndef WIZDUMB_VALIDATE_H
#define WIZDUMB_VALIDATE_H

#include "move.h"

struct Board;

void validate_castle_state (Board &board, struct move move);

#endif //WIZDUMB_VALIDATE_H
