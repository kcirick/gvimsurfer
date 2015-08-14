/*
 * src/client.c
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gtk/gtk.h>
#include <webkit/webkit.h>

#include "include/gvimsurfer.h"
#include "include/client.h"
#include "include/utilities.h"
#include "include/callbacks.h"
#include "include/completion.h"
#include "include/commands.h"
#include "include/shortcuts.h"

#include "config/config.h"

//--- Function declarations -----
GtkWidget * create_notebook_label(const gchar*, GtkWidget*, gint);
void        init_client_data();


void init_client() {

   Client.Global.mode         = NORMAL;
   Client.Global.keymap       = gdk_keymap_get_default();      // GDK keymap
   Client.Global.soup_session = webkit_get_default_session();  // libsoup session

   //--- Init UI -----
   Client.UI.window        = gtk_window_new(GTK_WINDOW_TOPLEVEL);

   Client.UI.box           = GTK_BOX(gtk_vbox_new(FALSE, 0));
   Client.UI.statusbar     = gtk_event_box_new();
   Client.UI.statusbar_box = GTK_BOX(gtk_hbox_new(FALSE, 0));
   Client.UI.inputbar      = GTK_ENTRY(gtk_entry_new());
   Client.UI.webview       = GTK_NOTEBOOK(gtk_notebook_new());

   // Window
   gtk_window_set_wmclass(GTK_WINDOW(Client.UI.window), TARGET, NAME);
   GdkGeometry hints = { 1, 1 };
   gtk_window_set_geometry_hints(GTK_WINDOW(Client.UI.window), NULL, &hints, GDK_HINT_MIN_SIZE);
   g_signal_connect(G_OBJECT(Client.UI.window), "destroy", G_CALLBACK(cb_destroy), NULL);

   // Box
   gtk_box_set_spacing(Client.UI.box, 0);
   gtk_container_add(GTK_CONTAINER(Client.UI.window), GTK_WIDGET(Client.UI.box));

   // Statusbar
   Client.Statusbar.message   = GTK_LABEL(gtk_label_new(NULL));
   Client.Statusbar.uri       = GTK_LABEL(gtk_label_new(NULL));
   Client.Statusbar.info      = GTK_LABEL(gtk_label_new(NULL));

   gtk_misc_set_alignment(GTK_MISC(Client.Statusbar.message),  0.0, 0.0);
   gtk_misc_set_alignment(GTK_MISC(Client.Statusbar.uri),      1.0, 0.0);
   gtk_misc_set_alignment(GTK_MISC(Client.Statusbar.info),     1.0, 0.0);

   gtk_misc_set_padding(GTK_MISC(Client.Statusbar.message), 2.0, 2.0);
   gtk_misc_set_padding(GTK_MISC(Client.Statusbar.uri),     2.0, 2.0);
   gtk_misc_set_padding(GTK_MISC(Client.Statusbar.info),    2.0, 2.0);

   gtk_box_pack_start(Client.UI.statusbar_box, GTK_WIDGET(Client.Statusbar.message),   FALSE,   TRUE,  0);
   gtk_box_pack_start(Client.UI.statusbar_box, GTK_WIDGET(Client.Statusbar.uri),       TRUE,    TRUE,  0);
   gtk_box_pack_start(Client.UI.statusbar_box, GTK_WIDGET(Client.Statusbar.info),      FALSE,   FALSE, 0);

   gtk_container_add(GTK_CONTAINER(Client.UI.statusbar), GTK_WIDGET(Client.UI.statusbar_box));
 
   // Inputbar
   gtk_entry_set_has_frame(Client.UI.inputbar, FALSE);
   gtk_editable_set_editable(GTK_EDITABLE(Client.UI.inputbar), TRUE);
   gtk_entry_set_inner_border(Client.UI.inputbar, NULL);

   g_signal_connect(G_OBJECT(Client.UI.inputbar), "key-press-event", G_CALLBACK(cb_inputbar_kb_pressed), NULL);
   g_signal_connect(G_OBJECT(Client.UI.inputbar), "activate",        G_CALLBACK(cb_inputbar_activate),   NULL);

   // Webview
   gtk_notebook_set_show_tabs(Client.UI.webview,   show_tabbar?TRUE:FALSE);
   gtk_notebook_set_scrollable(Client.UI.webview,  TRUE);
   gtk_notebook_set_show_border(Client.UI.webview, FALSE);

   // packing
   gtk_box_pack_start(Client.UI.box, GTK_WIDGET(Client.UI.webview),  TRUE,    TRUE,    0);
   gtk_box_pack_end(Client.UI.box, GTK_WIDGET(Client.UI.inputbar), FALSE,   FALSE,   0);
   gtk_box_pack_end(Client.UI.box, GTK_WIDGET(Client.UI.statusbar),  FALSE,   FALSE,   0);

   // set statusbar height
   gint w=0, h=0;
   PangoContext *pctx = gtk_widget_get_pango_context(GTK_WIDGET(Client.UI.inputbar));
   PangoLayout *layout = pango_layout_new(pctx);
   pango_layout_set_text(layout, "a", -1);
   pango_layout_set_font_description(layout, Client.Style.font);
   pango_layout_get_size(layout, &w, &h);
   Client.Style.statusbar_height = h/PANGO_SCALE+2;

   // Set statusbar styles
   gtk_widget_modify_bg(GTK_WIDGET(Client.UI.statusbar),       GTK_STATE_NORMAL, &(Client.Style.statusbar_bg));
   gtk_widget_modify_fg(GTK_WIDGET(Client.Statusbar.message),  GTK_STATE_NORMAL, &(Client.Style.statusbar_fg));
   gtk_widget_modify_fg(GTK_WIDGET(Client.Statusbar.uri) ,     GTK_STATE_NORMAL, &(Client.Style.statusbar_fg));
   gtk_widget_modify_fg(GTK_WIDGET(Client.Statusbar.info),     GTK_STATE_NORMAL, &(Client.Style.statusbar_fg));

   gtk_widget_modify_font(GTK_WIDGET(Client.Statusbar.message),  Client.Style.font);
   gtk_widget_modify_font(GTK_WIDGET(Client.Statusbar.uri),      Client.Style.font);
   gtk_widget_modify_font(GTK_WIDGET(Client.Statusbar.info),     Client.Style.font);

   gtk_widget_set_size_request(GTK_WIDGET(Client.UI.statusbar),   -1, Client.Style.statusbar_height);

   // inputbar styles
   gtk_widget_modify_base(GTK_WIDGET(Client.UI.inputbar), GTK_STATE_NORMAL, &(Client.Style.statusbar_bg));
   gtk_widget_modify_text(GTK_WIDGET(Client.UI.inputbar), GTK_STATE_NORMAL, &(Client.Style.inputbar_fg));
   gtk_widget_modify_font(GTK_WIDGET(Client.UI.inputbar),                     Client.Style.font);
   
   gtk_widget_set_size_request(GTK_WIDGET(Client.UI.inputbar),    -1, Client.Style.statusbar_height);

   // set window size
   gtk_window_set_default_size(GTK_WINDOW(Client.UI.window), default_width, default_height);

   init_client_data();
}

void init_client_data(){
   // read bookmarks
   gchar* bookmark_file = g_build_filename(g_get_home_dir(), config_dir, bookmarks, NULL);
   if(!bookmark_file)    return;

   if(g_file_test(bookmark_file, G_FILE_TEST_IS_REGULAR)){
      gchar* content = NULL;

      if(g_file_get_contents(bookmark_file, &content, NULL, NULL)) {
         gchar **lines = g_strsplit(content, "\n", -1);
         gint    n     = g_strv_length(lines) - 1;

         for(gint i = 0; i < n; i++) {
            if(!strlen(lines[i]))    continue;

            Client.Global.bookmarks = g_list_append(Client.Global.bookmarks, lines[i]);
         }
         g_free(content);
         g_free(lines);
      }
   }
   g_free(bookmark_file);

   // read history
   gchar* history_file = g_build_filename(g_get_home_dir(), config_dir, history, NULL);
   if(!history_file)     return;

   if(g_file_test(history_file, G_FILE_TEST_IS_REGULAR)) {
      gchar* content = NULL;

      if(g_file_get_contents(history_file, &content, NULL, NULL)) {
         gchar **lines = g_strsplit(content, "\n", -1);
         gint    n     = g_strv_length(lines) - 1;

         for(gint i = 0; i < n; i++) {
            if(!strlen(lines[i]))   continue;

            Client.Global.history = g_list_prepend(Client.Global.history, lines[i]);
         }
         Client.Global.history = g_list_reverse(Client.Global.history);

         g_free(content);
         g_free(lines);
      }
   }
   g_free(history_file);

   // read sessions
   gchar* sessions_file = g_build_filename(g_get_home_dir(), config_dir, sessions, NULL);
   if(!sessions_file)    return;

   if(g_file_test(sessions_file, G_FILE_TEST_IS_REGULAR)) {
      gchar* content = NULL;

      if(g_file_get_contents(sessions_file, &content, NULL, NULL)) {
         gchar **lines = g_strsplit(content, "\n", -1);
         gint    n     = g_strv_length(lines) - 1;

         for(gint i=0; i<n; i+=2) {
            if(!strlen(lines[i]) || !strlen(lines[i+1]))  continue;

            Session* se = malloc(sizeof(Session));
            se->name = lines[i];
            se->uris = lines[i+1];

            Client.Global.sessions = g_list_prepend(Client.Global.sessions, se);
         }
         g_free(content);
         g_free(lines);
      }
   }
   g_free(sessions_file);

   // load cookies
   gchar* cookie_file       = g_build_filename(g_get_home_dir(), config_dir, cookies, NULL);
   SoupCookieJar *cookiejar = soup_cookie_jar_text_new(cookie_file, FALSE);

   soup_session_add_feature(Client.Global.soup_session, (SoupSessionFeature*) cookiejar);
   g_free(cookie_file);
}

GtkWidget* create_tab(char* uri, gboolean background) {
   if(!uri)           uri = home_page;

   GtkWidget *tab = gtk_scrolled_window_new(NULL, NULL);
   WebKitWebView *wv  = (WebKitWebView*)webkit_web_view_new();

   int number_of_tabs = gtk_notebook_get_current_page(Client.UI.webview);
   int position       = number_of_tabs + 1;

   if(show_scrollbars)
      gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(tab), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
   else {
      WebKitWebFrame* mf = webkit_web_view_get_main_frame(WEBKIT_WEB_VIEW(wv));
      g_signal_connect(G_OBJECT(mf),  "scrollbars-policy-changed", G_CALLBACK(cb_blank), NULL);

      gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(tab), GTK_POLICY_NEVER, GTK_POLICY_NEVER);
   }

   /* connect webview callbacks */
   g_signal_connect(G_OBJECT(wv),  "console-message",                      G_CALLBACK(cb_wv_console_message),            NULL);
   g_signal_connect(G_OBJECT(wv),  "create-web-view",                      G_CALLBACK(cb_wv_create_webview),          NULL);
   g_signal_connect(G_OBJECT(wv),  "download-requested",                   G_CALLBACK(cb_wv_download_request),         NULL);
   g_signal_connect(G_OBJECT(wv),  "button-release-event",                 G_CALLBACK(cb_wv_button_release_event),     NULL);
   g_signal_connect(G_OBJECT(wv),  "hovering-over-link",                   G_CALLBACK(cb_wv_hover_link),               NULL);
   g_signal_connect(G_OBJECT(wv),  "mime-type-policy-decision-requested",  G_CALLBACK(cb_wv_mimetype_policy_decision), NULL);
   g_signal_connect(G_OBJECT(wv),  "navigation-policy-decision-requested", G_CALLBACK(cb_wv_nav_policy_decision),      NULL);
   g_signal_connect(G_OBJECT(wv),  "new-window-policy-decision-requested", G_CALLBACK(cb_wv_window_policy_decision),   NULL);
   g_signal_connect(G_OBJECT(wv),  "notify::progress",                     G_CALLBACK(cb_wv_notify_progress),          NULL);
   g_signal_connect(G_OBJECT(wv),  "notify::title",                        G_CALLBACK(cb_wv_notify_title),             NULL);
   g_signal_connect(G_OBJECT(wv),  "window-object-cleared",                G_CALLBACK(cb_wv_window_object_cleared),    NULL);
   g_signal_connect(G_OBJECT(wv),  "key-press-event",                      G_CALLBACK(cb_wv_kb_pressed),               NULL);

   // connect tab callbacks
   GtkAdjustment* adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(tab));
   g_signal_connect(G_OBJECT(tab),        "key-press-event", G_CALLBACK(cb_tab_kb_pressed), NULL);
   g_signal_connect(G_OBJECT(adjustment), "value-changed",   G_CALLBACK(cb_wv_scrolled),    NULL);

   /* set default values */
   g_object_set_data(G_OBJECT(wv), "loaded_scripts", 0);
   g_object_set(G_OBJECT(wv), "full-content-zoom", full_content_zoom, NULL);

   /////
   /* apply browser setting */
   WebKitWebSettings *settings = (WebKitWebSettings*)webkit_web_settings_new();
   char *filename, *file_url;

   gboolean enablePlugins = TRUE;
   gboolean enableScripts = TRUE;
   gboolean enableJava = TRUE;
   gboolean enablePagecache = FALSE;

   Client.Global.soup_session = webkit_get_default_session();
   g_object_set(G_OBJECT(Client.Global.soup_session), "ssl-ca-file", ca_bundle, NULL);
   g_object_set(G_OBJECT(Client.Global.soup_session), "ssl-strict", strict_ssl, NULL);
   g_object_set(G_OBJECT(settings), "enable-scripts", enableScripts, NULL);
   g_object_set(G_OBJECT(settings), "enable-plugins", enablePlugins, NULL);
   g_object_set(G_OBJECT(settings), "enable-java-applet", enableJava, NULL);
   g_object_set(G_OBJECT(settings), "enable-page-cache", enablePagecache, NULL);
   filename = g_build_filename(g_get_home_dir(), config_dir, stylesheet, NULL);    
   file_url = g_strdup_printf("file://%s", filename);
   g_object_set(G_OBJECT(settings), "user-stylesheet-uri", file_url, NULL);
   g_object_set(G_OBJECT(settings), "user-agent", user_agent, NULL);
   //g_object_get(G_OBJECT(settings), "zoom-step", &client.config.zoomstep, NULL);
   webkit_web_view_set_settings(wv, settings);

   GtkWidget* label = create_notebook_label("", GTK_WIDGET(Client.UI.webview), position);

   gtk_container_add(GTK_CONTAINER(tab), GTK_WIDGET(wv));
   gtk_widget_show_all(tab);
   gtk_notebook_insert_page(Client.UI.webview, tab, label, position);

   if(!background)
      gtk_notebook_set_current_page(Client.UI.webview, position);

   gtk_widget_grab_focus(GTK_WIDGET(GET_CURRENT_TAB_WIDGET()));

   // open uri 
   open_uri(WEBKIT_WEB_VIEW(wv), uri);
   change_mode(NORMAL);

   return GTK_WIDGET(wv);
}

GtkWidget * create_notebook_label( const gchar *text, GtkWidget *notebook, gint page) {

   GtkWidget *image;
   GtkWidget *hbox     = gtk_hbox_new(FALSE, 3);
   GtkWidget *label    = gtk_label_new(text);
   GtkWidget *button   = gtk_event_box_new();

   g_object_set_data( G_OBJECT(button), "page", GINT_TO_POINTER(page) );
   g_signal_connect( G_OBJECT(button), "button_press_event", G_CALLBACK(cb_button_close_tab), GTK_NOTEBOOK(notebook) );

   gtk_box_pack_start(GTK_BOX(hbox),   label,  TRUE,  TRUE,    0);
   gtk_box_pack_end(GTK_BOX(hbox),     button, FALSE, FALSE,   0);

   image = gtk_image_new_from_stock (GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
   gtk_event_box_set_visible_window(GTK_EVENT_BOX(button), FALSE);
   gtk_container_add (GTK_CONTAINER(button), image);

   gtk_widget_show_all( hbox );

   return( hbox );
}

void close_tab(int tab_id) {

   // remove markers for this tab and update the others
   GList* list = Client.Global.pagemarks;
   while (list) {
      PMark* pmark = (PMark*) list->data;
      if (pmark->tab_id == tab_id) {
         Client.Global.pagemarks = g_list_delete_link(Client.Global.pagemarks, list);
         free(pmark);
      } else if (pmark->tab_id > tab_id) {
         pmark->tab_id -= 1;
      }
      list = g_list_next(list);
   }

   Client.Global.last_closed = g_strdup((gchar *) webkit_web_view_get_uri(GET_CURRENT_TAB()));

   if (gtk_notebook_get_n_pages(Client.UI.webview) > 1) {
      gtk_notebook_remove_page(Client.UI.webview, tab_id);
      update_client();
   } else 
      cb_destroy(NULL, NULL);
}

void new_window(char* uri) {
   if(!uri)     return;

   char* nargv[3] = { TARGET, uri, NULL };

   g_spawn_async(NULL, nargv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL);
}

void update_client(){
   if(!Client.UI.webview || !gtk_notebook_get_n_pages(Client.UI.webview)) return;

   update_statusbar_info();
   update_statusbar_uri();

   // update title
   gchar* title = g_strdup_printf("%s %s: %s", NAME, private_browsing ? "(PRIVATE)" : "", webkit_web_view_get_title(GET_CURRENT_TAB()));
   if(title)    gtk_window_set_title(GTK_WINDOW(Client.UI.window), title);

   // update tabbar
   int number_of_tabs  = gtk_notebook_get_n_pages(Client.UI.webview);
   for(int tc = 0; tc < number_of_tabs; tc++) {
      GtkWidget* tab       = GTK_WIDGET(GET_NTH_TAB_WIDGET(tc));

      const gchar* tab_title = webkit_web_view_get_title(GET_WEBVIEW(tab));
      int progress = webkit_web_view_get_progress(GET_WEBVIEW(tab)) * 100;
      gchar* n_tab_title = g_strdup_printf("%d: %s", tc + 1, tab_title ? tab_title : ((progress == 100) ? "Loading..." : "(Untitled)"));

      // shorten title if needed
      gchar* new_tab_title = g_strdup_printf("%s", n_tab_title);
      if(strlen(n_tab_title)> max_title_length){
         g_strlcpy(new_tab_title, n_tab_title, max_title_length-3);
         g_strlcat(new_tab_title, "...", max_title_length);
      }

      
      GtkWidget* label = gtk_notebook_get_tab_label(Client.UI.webview, tab);
      label = NULL;
      label = create_notebook_label(new_tab_title, GTK_WIDGET(Client.UI.webview), tc);
      //gtk_notebook_set_tab_label_text(Client.UI.webview, tab, new_tab_title);
      gtk_notebook_set_tab_label(Client.UI.webview, tab, label);
      g_free(n_tab_title);
      g_free(new_tab_title);
   }
}

void update_statusbar_info() {
   if (gtk_notebook_get_current_page(Client.UI.webview) == -1)
      return;

   GString *status = g_string_new("");

   // update page load and download status
   {
      // check for possible navigation
      GString* navigation = g_string_new("");
      g_string_append_c(navigation, webkit_web_view_can_go_back(GET_CURRENT_TAB())?'<':'-');
      g_string_append_c(navigation, webkit_web_view_can_go_forward(GET_CURRENT_TAB())?'>':'-');

      g_string_append_printf(status, " [%s]\t", navigation->str);
      g_string_free(navigation, TRUE);

      //page load status
      gint progress = -1;
      progress = webkit_web_view_get_progress(GET_CURRENT_TAB())*100;
      if(progress>0)
         g_string_append_printf(status, "[%d%%]\t", progress);

      // download status
      if(Client.Global.active_downloads){
         gint download_count = g_list_length(Client.Global.active_downloads);

         progress=0;
         for(GList *ptr = Client.Global.active_downloads; ptr; ptr= g_list_next(ptr))
            progress += 100*webkit_download_get_progress(ptr->data);

         progress /= download_count;

         g_string_append_printf(status, "[%d DL: %d%%]\t", download_count, progress);
      }
   }

   // update position
   GtkAdjustment* adjustment = gtk_scrolled_window_get_vadjustment(GET_CURRENT_TAB_WIDGET());
   gdouble view_size         = gtk_adjustment_get_page_size(adjustment);
   gdouble value             = gtk_adjustment_get_value(adjustment);
   gdouble max               = gtk_adjustment_get_upper(adjustment) - view_size;

   gchar* position;
   if(max == 0)            position = g_strdup("All");
   else if(value == max)   position = g_strdup("Bot");
   else if(value == 0)     position = g_strdup("Top");
   else
      position = g_strdup_printf("%2d%%", (int) ceil(((value / max) * 100)));

   g_string_append_printf(status, "%s", position);

   gtk_label_set_text((GtkLabel*) Client.Statusbar.info, status->str);

   g_string_free(status, TRUE);
   g_free(position);
}

void update_statusbar_uri() {
   gchar* link  = (gchar*) webkit_web_view_get_uri(GET_CURRENT_TAB());
   gchar* uri = link?g_strdup_printf("%s", link):"[NULL]";

   /* check for https */
   gboolean ssl = link ? g_str_has_prefix(link, "https://") : FALSE;
   GdkColor* fg = ssl ? &(Client.Style.statusbar_ssl_fg) : &(Client.Style.statusbar_fg);

   gtk_widget_modify_fg(GTK_WIDGET(Client.Statusbar.uri),      GTK_STATE_NORMAL, fg);
   gtk_label_set_text((GtkLabel*) Client.Statusbar.uri, uri);
   //g_free(uri);
}

void set_inputbar_visibility(gint visibility){

   gboolean is_visible = GTK_WIDGET_VISIBLE(GTK_WIDGET(Client.UI.inputbar));

   if(visibility==HIDE || (visibility==TOGGLE && is_visible)){
      gtk_widget_hide(GTK_WIDGET(Client.UI.inputbar));
      if(show_statusbar)
         gtk_widget_show(GTK_WIDGET(Client.UI.statusbar));
      else
         gtk_widget_hide(GTK_WIDGET(Client.UI.statusbar));
   }else if(visibility==SHOW || (visibility==TOGGLE && !is_visible)){
      gtk_widget_show(GTK_WIDGET(Client.UI.inputbar));
      gtk_widget_hide(GTK_WIDGET(Client.UI.statusbar));
   }
}

