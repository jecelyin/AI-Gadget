#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <themes/lv_theme.h>


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

lv_theme_t * app_default_theme_init(lv_display_t * disp);
/**
 * Initialize the theme
 * @param disp pointer to display
 * @param color_primary the primary color of the theme
 * @param color_secondary the secondary color for the theme
 * @param dark
 * @param font pointer to a font to use.
 * @return a pointer to reference this theme later
 */
lv_theme_t * app_theme_init(lv_display_t * disp, lv_color_t color_primary, lv_color_t color_secondary, bool dark,
                                   const lv_font_t * font);

/**
 * Get default theme
 * @return a pointer to default theme, or NULL if this is not initialized
 */
lv_theme_t * app_theme_get(void);

/**
 * Check if default theme is initialized
 * @return true if default theme is initialized, false otherwise
 */
bool app_theme_is_inited(void);

/**
 * Deinitialize the default theme
 */
void app_theme_deinit(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif
