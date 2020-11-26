#include <stdio.h>
#include <stdlib.h>

#include "material.h"

void material_init (struct material *material)
{
    for (size_t i = 0; i < NR_PLAYERS; i++)
    {
        material->score[i] = 0;
    }
}

