#include <stdio.h>
#include <stdlib.h>

#include "material.h"

void material_init (struct material *material)
{
    // TODO: Handle non-defaul starting positions and initialize from the pieces
    // on the board instead
    for (int i = 0; i < NR_PLAYERS; i++)
    {
        material->score[i] = 0;
    }
}

