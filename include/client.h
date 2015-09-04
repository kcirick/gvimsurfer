/*
 * include/client.h
 */

#ifndef CLIENT_H
#define CLIENT_H

//--- The main client -----
struct {
   struct {
      GtkWidget       *window;
      GtkWidget       *add_button;
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
      GList       *pages;
      GList       *active_downloads;
      GList       *quickmarks;
      GList       *bookmarks;
      GList       *sessions;
      GList       *history;
      gchar       *last_closed;
      GList       *search_engines;
      gchar       *search_handle;
      Script      *user_script;
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

void        close_tab(gint);
GtkWidget*  create_tab(const gchar*, gboolean);
void        new_window(const gchar*);
void        set_inputbar_visibility(gint);
void        update_client(gint);
void        update_statusbar_info(gint);

#endif
