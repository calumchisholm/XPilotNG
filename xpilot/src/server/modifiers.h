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

#ifndef MODIFIERS_H
#define MODIFIERS_H

/*
 * Weapons modifiers.
 */

typedef uint16_t	modifiers_t;

#define MODS_BIT0	(1<<0)
#define MODS_BIT1	(1<<1)

#define MODS_N_BIT0	(1<<0)		/* nuclear */
#define MODS_N_BIT1	(1<<1)		/* fullnuclear */
#define MODS_C_BIT	(1<<2)		/* cluster */
#define MODS_I_BIT	(1<<3)		/* implosion */
#define MODS_V_BIT0	(1<<4)		/* velocity */
#define MODS_V_BIT1	(1<<5)
#define MODS_X_BIT0	(1<<6)		/* mini */
#define MODS_X_BIT1	(1<<7)
#define MODS_Z_BIT0	(1<<8)		/* spread */
#define MODS_Z_BIT1	(1<<9)
#define MODS_B_BIT0	(1<<10)		/* power */
#define MODS_B_BIT1	(1<<11)
#define MODS_LS_BIT	(1<<12)		/* stun laser */
#define MODS_LB_BIT	(1<<13)		/* blinding laser */

#define MODS_NUCLEAR_MAX	3	/* - N FN */
#define MODS_VELOCITY_MAX	3	/* - V1 V2 V3 */
#define MODS_MINI_MAX		3	/* - X2 X3 X4 */
#define MODS_SPREAD_MAX		3	/* - Z1 Z2 Z3 */
#define MODS_POWER_MAX		3	/* - B1 B2 B3 */
#define MODS_LASER_MAX		2	/* - LS LB */

#define MODS_NUCLEAR		(1<<0)
#define MODS_FULLNUCLEAR	(1<<1)
#define MODS_LASER_STUN		(1<<0)
#define MODS_LASER_BLIND	(1<<1)

#define CLEAR_MODS(mods)	memset(&(mods), 0, sizeof(modifiers_t))

void Modifiers_to_string(modifiers_t mods, char *dst, size_t size);

static inline int Get_nuclear_modifier(modifiers_t mods)
{
    int n0, n1;

    n0 = BIT(mods, MODS_N_BIT0) ? 1 : 0;
    n1 = BIT(mods, MODS_N_BIT1) ? 1 : 0;

    return (n1 << 1) + n0;
}
static inline void Set_nuclear_modifier(modifiers_t *mods, int value)
{
    LIMIT(value, 0, MODS_NUCLEAR_MAX);
    if (BIT(value, MODS_BIT0))
	SET_BIT(*mods, MODS_N_BIT0);
    else
	CLR_BIT(*mods, MODS_N_BIT0);
    if (BIT(value, MODS_BIT1))
	SET_BIT(*mods, MODS_N_BIT1);
    else
	CLR_BIT(*mods, MODS_N_BIT1);
}

static inline int Get_cluster_modifier(modifiers_t mods)
{
    return (int) BIT(mods, MODS_C_BIT) ? 1 : 0;
}
static inline void Set_cluster_modifier(modifiers_t *mods, int value)
{
    LIMIT(value, 0, 1);
    if (value)
	SET_BIT(*mods, MODS_C_BIT);
    else
	CLR_BIT(*mods, MODS_C_BIT);
}

static inline int Get_implosion_modifier(modifiers_t mods)
{
    return (int) BIT(mods, MODS_I_BIT) ? 1 : 0;
}
static inline void Set_implosion_modifier(modifiers_t *mods, int value)
{
    LIMIT(value, 0, 1);
    if (value)
	SET_BIT(*mods, MODS_I_BIT);
    else
	CLR_BIT(*mods, MODS_I_BIT);
}

static inline int Get_velocity_modifier(modifiers_t mods)
{
    int v0, v1;

    v0 = BIT(mods, MODS_V_BIT0) ? 1 : 0;
    v1 = BIT(mods, MODS_V_BIT1) ? 1 : 0;

    return (v1 << 1) + v0;
}
static inline void Set_velocity_modifier(modifiers_t *mods, int value)
{
    LIMIT(value, 0, MODS_VELOCITY_MAX);
    if (BIT(value, MODS_BIT0))
	SET_BIT(*mods, MODS_V_BIT0);
    else
	CLR_BIT(*mods, MODS_V_BIT0);
    if (BIT(value, MODS_BIT1))
	SET_BIT(*mods, MODS_V_BIT1);
    else
	CLR_BIT(*mods, MODS_V_BIT1);
}

static inline int Get_mini_modifier(modifiers_t mods)
{
    int x0, x1;

    x0 = BIT(mods, MODS_X_BIT0) ? 1 : 0;
    x1 = BIT(mods, MODS_X_BIT1) ? 1 : 0;

    return (x1 << 1) + x0;
}
static inline void Set_mini_modifier(modifiers_t *mods, int value)
{
    LIMIT(value, 0, MODS_MINI_MAX);
    if (BIT(value, MODS_BIT0))
	SET_BIT(*mods, MODS_X_BIT0);
    else
	CLR_BIT(*mods, MODS_X_BIT0);
    if (BIT(value, MODS_BIT1))
	SET_BIT(*mods, MODS_X_BIT1);
    else
	CLR_BIT(*mods, MODS_X_BIT1);
}

static inline int Get_spread_modifier(modifiers_t mods)
{
    int z0, z1;

    z0 = BIT(mods, MODS_Z_BIT0) ? 1 : 0;
    z1 = BIT(mods, MODS_Z_BIT1) ? 1 : 0;

    return (z1 << 1) + z0;
}
static inline void Set_spread_modifier(modifiers_t *mods, int value)
{
    LIMIT(value, 0, MODS_SPREAD_MAX);
    if (BIT(value, MODS_BIT0))
	SET_BIT(*mods, MODS_Z_BIT0);
    else
	CLR_BIT(*mods, MODS_Z_BIT0);
    if (BIT(value, MODS_BIT1))
	SET_BIT(*mods, MODS_Z_BIT1);
    else
	CLR_BIT(*mods, MODS_Z_BIT1);
}

static inline int Get_power_modifier(modifiers_t mods)
{
    int b0, b1;

    b0 = BIT(mods, MODS_B_BIT0) ? 1 : 0;
    b1 = BIT(mods, MODS_B_BIT1) ? 1 : 0;

    return (b1 << 1) + b0;
}
static inline void Set_power_modifier(modifiers_t *mods, int value)
{
    LIMIT(value, 0, MODS_POWER_MAX);
    if (BIT(value, MODS_BIT0))
	SET_BIT(*mods, MODS_B_BIT0);
    else
	CLR_BIT(*mods, MODS_B_BIT0);
    if (BIT(value, MODS_BIT1))
	SET_BIT(*mods, MODS_B_BIT1);
    else
	CLR_BIT(*mods, MODS_B_BIT1);
}

static inline int Get_laser_modifier(modifiers_t mods)
{
    int ls, lb;

    ls = BIT(mods, MODS_LS_BIT) ? 1 : 0;
    lb = BIT(mods, MODS_LB_BIT) ? 1 : 0;

    return (lb << 1) + ls;
}
static inline void Set_laser_modifier(modifiers_t *mods, int value)
{
    LIMIT(value, 0, MODS_LASER_MAX);
    if (BIT(value, MODS_BIT0))
	SET_BIT(*mods, MODS_LS_BIT);
    else
	CLR_BIT(*mods, MODS_LS_BIT);
    if (BIT(value, MODS_BIT1))
	SET_BIT(*mods, MODS_LB_BIT);
    else
	CLR_BIT(*mods, MODS_LB_BIT);
}

#endif
