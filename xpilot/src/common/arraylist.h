/* 
 * XPilot NG, a multiplayer space war game.
 *
 * Copyright (C) 2005 Kristian Söderblom <kps@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#include "xpcommon.h"

typedef struct {
    void	*elements;
    size_t	element_size;
    size_t	element_padded_size;
    int 	num_elements;
    int 	max_elements;
} arraylist_t;

static inline void *Arraylist_get_element_pointer(arraylist_t *alp, int ind)
{
    return (uint8_t *)alp->elements + alp->element_padded_size * ind;
}

static inline int Arraylist_get_num_elements(arraylist_t *alp)
{
    return alp->num_elements;
}

void *Arraylist_get(arraylist_t *alp, int ind);
void Arraylist_add(arraylist_t *alp, void *element);
arraylist_t *Arraylist_create(size_t element_size);

#endif
