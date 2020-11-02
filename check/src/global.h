#ifndef WIZDUMB_GLOBAL_H
#define WIZDUMB_GLOBAL_H

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#define NR_PLAYERS                (2U)

#define NR_ROWS                   (8U)
#define NR_COLUMNS                (8U)

#define LAST_ROW                  (NR_ROWS-1)
#define LAST_COLUMN               (NR_COLUMNS-1)

#define KING_COLUMN               (4U)

#define KING_ROOK_COLUMN          (7U)
#define QUEEN_ROOK_COLUMN         (0U)
#define KING_CASTLED_ROOK_COLUMN  (5U)
#define QUEEN_CASTLED_ROOK_COLUMN (3U)


#endif //WIZDUMB_GLOBAL_H
