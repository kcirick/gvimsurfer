/*
 * config/config.h
 */

#ifndef CONFIG_H
#define CONFIG_H

// All variables are initialized in config file
gint   default_width;
gint   default_height;
gint   max_title_length;

gboolean fb_enabled;
gboolean full_content_zoom;
gboolean show_scrollbars;
gboolean show_statusbar;
gboolean show_tabbar;
gboolean strict_ssl; 

gchar* user_agent;
gchar* home_page;
gchar* external_editor;

gint n_completion_items;
gint history_limit;
gfloat zoom_step;
gfloat scroll_step;

gchar* download_dir;
gchar* config_dir;
gchar* bookmarks;
gchar* history;
gchar* cookies;
gchar* sessions;
gchar* stylesheet;
gchar* scriptfile;

#endif
