#include <stdio.h>
#include <stdlib.h>

#include "material.h"

struct material *material_new (void)
{
	return calloc (1, sizeof (struct material));
}

void material_free (struct material *material)
{
	free (material);
}
