/*
 * include/client.h
 */

#ifndef CLIENT_H
#define CLIENT_H

//--- The main client -----
struct {
   struct {
      GtkWidget       *window;
      GtkBox          *box;
      GtkWidget       *statusbar;
      GtkBox          *statusbar_box;
      GtkEntry        *inputbar;
      GtkNotebook     *webview;
   } UI;

   struct {
      GdkColor statusbar_bg;
      GdkColor statusbar_fg;
      GdkColor statusbar_ssl_fg;
      GdkColor inputbar_fg;
      GdkColor completion_fg;
      GdkColor completion_bg;
      GdkColor completion_g_bg;
      GdkColor completion_g_fg;
      GdkColor completion_hl_fg;
      GdkColor completion_hl_bg;
      GdkColor notification_fg;
      GdkColor notification_bg;
      PangoFontDescription *font;
   } Style;

   struct {
      GString     *buffer;
      gint        mode;
      GList       *active_downloads;
      GList       *pagemarks;
      GList       *quickmarks;
      GList       *bookmarks;
      GList       *sessions;
      GList       *history;
      GList       *last_closed;
      GList       *search_engines;
      GList       *scripts;
      GdkKeymap   *keymap;
      SoupSession *soup_session;
   } Global;

   struct {
      GtkLabel *message;
      GtkLabel *uri;
      GtkLabel *info;
   } Statusbar;

} Client;

//--- Function declarations -----
void init_client();
void init_client_data();
void init_client_settings();

GtkWidget*  create_tab(char*, gboolean);
void        close_tab(int);
void        new_window(char*);

void update_client();
void update_statusbar_info();
void update_statusbar_uri();

#endif
