/**
 * @file lv_theme_material.h
 *
 */

#ifndef THEME_H_INCLUDED
#define THEME_H_INCLUDED

#include "lvgl.h"

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize the default
 * @param color_primary the primary color of the theme
 * @param color_secondary the secondary color for the theme
 * @param flags ORed flags starting with `LV_THEME_DEF_FLAG_...`
 * @param font_small pointer to a small font
 * @param font_normal pointer to a normal font
 * @param font_subtitle pointer to a large font
 * @param font_title pointer to a extra large font
 * @return a pointer to reference this theme later
 */
lv_theme_t *theme_init(lv_color_t color_primary, lv_color_t color_secondary, uint32_t flags,
                       const lv_font_t *font_small, const lv_font_t *font_normal, const lv_font_t *font_subtitle,
                       const lv_font_t *font_title);
/**********************
 *      MACROS
 **********************/


#endif /*LV_THEME_MATERIAL_H*/