/*
 * config/settings.h
 */

#ifndef CONFIG_SETTINGS_H
#define CONFIG_SETTINGS_H

//--- Settings -----
/*    type can be one of:
 *       i - integer value
 *       b - boolean value (true/false) * case sensitive
 *       f - float value
 *       s - string value
 *       c - character value
 *       * - special setting
 */
static const Setting settings[] = {
   // public name,            internal variable    webkit setting             type     description
   { "font_size",             NULL,                "default-font-size",       'i' },   // Default font size to display text
   { "n_completion_items",    &(n_completion_items), NULL,                    'i' },   // Number of completion items
   { "auto_shrink_images",    NULL,                "auto-shrink-images",      'b' },   // Shrink standalone images to fit
   { "background",            NULL,                "print-backgrounds",       'b' },   // Print background images
   { "full_content_zoom",     &(full_content_zoom), "full-content-zoom",      'b' },   // Scale full content when zooming
   { "flash_block",           &(fb_enabled),       NULL,                      'b' },   // Enable flash block
   { "images",                NULL,                "auto-load-images",        'b' },   // Load images automatically
   { "java_applet",           NULL,                "enable-java-applet",      'b' },   // Enable Java <applet> tag
   { "page_cache",            NULL,                "enable-page-cache",       'b' },   // Enable page cache
   { "private",               &(private_mode),     "enable-private-browsing", 'b' },   // Enable private browsing
   { "resizable_text_areas",  NULL,                "resizable-text-areas",    'b' },   // Resizable text areas
   { "scrollbars",            &(show_scrollbars),  NULL,                      'b' },   // Show scrollbars
   { "statusbar",             &(show_statusbar),   NULL,                      'b' },   // Show statusbar
   { "tabbar",                &(show_tabbar),      NULL,                      'b' },   // Show statusbar
   { "scroll_step",           &(scroll_step),      NULL,                      'f' },   // Scroll step
   { "zoom_step",             &(zoom_step),        "zoom-step",               'f' },   // Zoom step
   { "editor",                &(external_editor),  NULL,                      's' },   // Command to spawn the default editor
   { "encoding",              NULL,                "default-encoding",        's' },   // Default encoding to display text
   { "user_agent",            &(user_agent),       "user-agent",              's' },   // User agent
   { "stylesheet",            NULL,                "user-stylesheet-uri",     's' },   // Custom stylesheet
   { "proxy",                 NULL,                NULL,                      '*' },   // Toggle proxy
   { "windowsize",            NULL,                NULL,                      '*' },   // Set window size
};

#endif
