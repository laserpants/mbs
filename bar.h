#ifndef BAR_H
#define BAR_H

#include "stash.h"
#include <stdbool.h>

void bar_draw_box (void);
void bar_render (struct stash *s, bool tx, bool rx);
void bar_restore_term (void);

#endif 
