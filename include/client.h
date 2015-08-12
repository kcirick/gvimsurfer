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
      gint                 statusbar_height;
      GdkColor             statusbar_bg;
      GdkColor             statusbar_fg;
      GdkColor             statusbar_ssl_fg;
      GdkColor             inputbar_fg;
      GdkColor             notification_fg;
      GdkColor             completion_fg;
      GdkColor             completion_bg;
      GdkColor             completion_hl;
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
      gchar       *last_closed;
      gchar       *search_handle;
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
void        init_client();
void        init_client_data();

void        close_tab(int);
GtkWidget*  create_tab(char*, gboolean);
void        new_window(char*);
void        set_inputbar_visibility(gint);
void        update_client();
void        update_statusbar_info();
void        update_statusbar_uri();

#endif
