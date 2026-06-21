/**
 * crypt_of_the_pixel.h — Master Include Header
 * 
 * Includes all subsystems. Include this single header in any file
 * that needs access to the full engine + game.
 * 
 * Version 0.1.0 — Initial implementation
 */

#ifndef CRYPT_OF_THE_PIXEL_H
#define CRYPT_OF_THE_PIXEL_H

/* ---- Version ---- */
#define COTP_VERSION_MAJOR   0
#define COTP_VERSION_MINOR   1
#define COTP_VERSION_PATCH   0

/* ---- Game constants ---- */
#define GAME_TITLE        "Crypt of the Pixel"
#define GAME_TITLE_SHORT  "CryptPixel"
#define GAME_CODE         "CTPX"
#define GAME Maker        "Z"

/* ---- Map constants ---- */
#define MAP_SIZE          64        /* 64×64 tile grid */
#define MAP_BYTES         (MAP_SIZE * MAP_SIZE)

/* ---- Entity limits ---- */
#define MAX_ENEMIES       32
#define MAX_ITEMS_INV     16        /* Inventory slots */
#define MAX_GROUND_ITEMS  16
#define MAX_SPRITES       64
#define MAX_ROOMS         12
#define MAX_FLOOR         10

/* ---- Difficulty constants ---- */
#define DETECT_RANGE      8
#define FOG_RADIUS        7
#define BOSS_FLOOR_START  1
#define ENEMY_SCALE_RATE  3         /* HP/ATK gain per floor */

/* ---- Turn constants ---- */
#define TURN_DELAY_FRAMES 5

/* ---- Save system ---- */
#define SAVE_MAGIC        0x4354    /* "CT" */
#define SAVE_VERSION      1
#define SAVE_SIZE         64        /* bytes */

/* ---- Engine includes ---- */
#include "gba_types.h"
#include "gba_header.h"
#include "system.h"
#include "input.h"
#include "dma.h"
#include "random.h"
#include "timer.h"
#include "iso_math.h"
#include "tilemap.h"
#include "sprite_manager.h"
#include "text_renderer.h"

/* ---- Game includes ---- */
#include "dungeon_gen.h"
#include "player.h"
#include "enemy.h"
#include "combat.h"
#include "items.h"
#include "hud.h"
#include "game_state.h"

/* ---- Asset includes ---- */
#include "palettes.h"
#include "tiles.h"
#include "sprites.h"
#include "font.h"
#include "enemy_data.h"
#include "item_data.h"

#endif /* CRYPT_OF_THE_PIXEL_H */
