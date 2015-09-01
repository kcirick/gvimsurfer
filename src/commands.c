/*
 * src/commands.c
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <gtk/gtk.h>
#include <webkit/webkit.h>

#include "include/gvimsurfer.h"
#include "include/client.h"
#include "include/utilities.h"
#include "include/completion.h"
#include "include/commands.h"
#include "include/callbacks.h"

#include "config/config.h"
#include "config/settings.h"

gboolean cmd_back(int argc, char** argv) {
   gint increment = argc==0 ? 1 : atoi(argv[0]);
   webkit_web_view_go_back_or_forward(GET_CURRENT_TAB(), -1*increment);

   return TRUE;
}

gboolean cmd_bookmark(int argc, char** argv) {
   char* bookmark = g_strdup(webkit_web_view_get_uri(GET_CURRENT_TAB()));

   /* at first we verify that bookmark (without tag) isn't already in the list */
   unsigned int bookmark_length = strlen(bookmark);
   for(GList* l = Client.Global.bookmarks; l; l = g_list_next(l)) {
      if(!strncmp(bookmark, (char*) l->data, bookmark_length)) {
         g_free(l->data);
         Client.Global.bookmarks = g_list_delete_link(Client.Global.bookmarks, l);
         break;
      }
   }

   if(argc >= 1 && argv[argc] == NULL) {
      char* tags = g_strjoinv(" ", argv);
      char* bookmark_temp = bookmark;
      bookmark = g_strjoin(" ", bookmark, tags, NULL);

      g_free(bookmark_temp);
      g_free(tags);
   }
   Client.Global.bookmarks = g_list_append(Client.Global.bookmarks, bookmark);

   return TRUE;
}

gboolean cmd_cancel_download(int argc, char** argv){

   if(argc<1) return FALSE;

   gint which = atoi(argv[0]);

   WebKitDownload* this_download = (WebKitDownload*)g_list_nth_data(Client.Global.active_downloads, which);

   if(!this_download){
      notify(WARNING, "No such download!");
      return FALSE;
   }

   webkit_download_cancel(this_download);

   notify(INFO, "Download cancelled");

   return TRUE;
}

gboolean cmd_forward(int argc, char** argv) {
   gint increment = argc==0 ? 1 : atoi(argv[0]);
   webkit_web_view_go_back_or_forward(GET_CURRENT_TAB(), increment);

   return TRUE;
}

gboolean cmd_open(int argc, char** argv) {
  if(argc <= 0)            return TRUE;
  if(argv[argc] != NULL)   return TRUE;

  gchar* uri = g_strjoinv(" ", argv);
  if(strlen(uri)>0)
     open_uri(GET_CURRENT_TAB(), uri);

  g_free(uri);

  return TRUE;
}

gboolean cmd_pagemark(int argc, char** argv){
   if(argc<1) return FALSE;

   int id = atoi(argv[0]);
   GtkAdjustment* adjustment;
   adjustment = gtk_scrolled_window_get_vadjustment(GET_CURRENT_TAB_WIDGET());
   gdouble va = gtk_adjustment_get_value(adjustment);
   adjustment = gtk_scrolled_window_get_hadjustment(GET_CURRENT_TAB_WIDGET());
   gdouble ha = gtk_adjustment_get_value(adjustment);
   float zl   = webkit_web_view_get_zoom_level(GET_CURRENT_TAB());

   Page* page = get_current_page();

   // search if entry already exists
   for(GList* list = page->pagemarks; list; list = g_list_next(list)) {
      PMark* pmark = (PMark*) list->data;

      if(pmark->id == id) {
         pmark->vadjustment = va;
         pmark->hadjustment = ha;
         pmark->zoom_level  = zl;
         return TRUE;
      }
   }

   // add new marker
   PMark* pmark = malloc(sizeof(PMark));
   pmark->id          = id;
   pmark->vadjustment = va;
   pmark->hadjustment = ha;
   pmark->zoom_level  = zl;

   page->pagemarks = g_list_append(page->pagemarks, pmark);
   
   return TRUE;
}

gboolean cmd_quickmark(int argc, char** argv){
   if(argc<1) return FALSE;

   int id = atoi(argv[0]);
   char* this_uri = (char*) webkit_web_view_get_uri(GET_CURRENT_TAB());

   // search if entry already exists
   for(GList* list = Client.Global.quickmarks; list; list = g_list_next(list)){
      QMark* qmark = (QMark*) list->data;   

      if(qmark->id == id){
         qmark->uri = this_uri;
         return TRUE;
      }
   }

   // add new qmark
   QMark* qmark = malloc(sizeof(QMark));
   qmark-> id  = id;
   qmark->uri  = this_uri;

   Client.Global.quickmarks = g_list_append(Client.Global.quickmarks, qmark);

   return TRUE;
}

gboolean cmd_quit(int argc, char** argv) {
   close_tab(gtk_notebook_get_current_page(Client.UI.webview));
   return TRUE;
}

gboolean cmd_quitall(int argc, char** argv) {
   cb_destroy(NULL, NULL);
   return TRUE;
}

gboolean cmd_reload(int argc, char** argv) {
   if(argc==0){
      webkit_web_view_reload(GET_CURRENT_TAB());
   } else if (strcmp(argv[0], "all")==0){
      int number_of_tabs = gtk_notebook_get_n_pages(Client.UI.webview);

      for(int i=0; i<number_of_tabs; i++)
         webkit_web_view_reload(GET_NTH_TAB(i));
   }
   return TRUE;
}

gboolean cmd_saveas(int argc, char** argv) {
   
   if(argc < 2) return FALSE;

   const gchar* uri = strcmp(argv[0], "*")==0 ?
      webkit_web_view_get_uri(GET_CURRENT_TAB()) :
      argv[0];

   if(!uri){
      notify(ERROR, "Could not retrieve download uri");
      return FALSE;
   }

   WebKitNetworkRequest* request = webkit_network_request_new(uri);
   WebKitDownload* download = webkit_download_new(request);

   const gchar* filename = strcmp(argv[1], "*")==0 ?
      webkit_download_get_suggested_filename(download) :
      argv[0];

   download_content(download, filename);

   return TRUE;
}

gboolean cmd_load_script(int argc, char** argv) {
   if(argc < 1)    return TRUE;

   return load_script(argv[0]);
}

gboolean cmd_session(int argc, char** argv) {
   if(argc<=1 || argv[argc]!=NULL)  return FALSE;

   gchar* session_name   = argv[1];
   
   gboolean to_return=FALSE;
   if(strcmp(argv[0], "save")==0)
      to_return = sessionsave(session_name);
   else if(strcmp(argv[0], "load")==0)
      to_return = sessionload(session_name);

   g_free(session_name);

   return to_return;
}

gboolean cmd_stop(int argc, char** argv) {
   webkit_web_view_stop_loading(GET_CURRENT_TAB());
   return TRUE;
}

gboolean cmd_settings(int argc, char** argv) {
   if(argc < 2)   return TRUE;

   WebKitWebSettings* browser_settings = (WebKitWebSettings*)webkit_web_view_get_settings(GET_CURRENT_TAB());

   for(unsigned int i = 0; i < LENGTH(settings); i++) {
      if(!strcmp(argv[0], settings[i].name)) {

         //check var type
         if(settings[i].type == 'b') {
            gboolean value = !strcmp(argv[1], "false") ? FALSE : TRUE;

            if(settings[i].variable) {
               gboolean *x = (gboolean*) (settings[i].variable);
               *x = value;
            }

            if(settings[i].webkitvar)
               g_object_set(G_OBJECT(browser_settings), settings[i].webkitvar, value, NULL);
         } else if(settings[i].type == 'i') {
            int id = atoi(argv[1]);

            if(settings[i].variable) {
               int *x = (int*) (settings[i].variable);
               *x = id;
            }

            if(settings[i].webkitvar)
               g_object_set(G_OBJECT(browser_settings), settings[i].webkitvar, id, NULL);
         } else if(settings[i].type == 'f') {
            float value = atof(argv[1]);

            if(settings[i].variable) {
               float *x = (float*) (settings[i].variable);
               *x = value;
            }

            if(settings[i].webkitvar)
               g_object_set(G_OBJECT(browser_settings), settings[i].webkitvar, value, NULL);
         } else if(settings[i].type == 's') {
            gchar* s = g_strjoinv(" ", &(argv[1]));

            if(settings[i].variable) {
               char **x = (char**) settings[i].variable;
               *x = s;
            }

            if(settings[i].webkitvar)
               g_object_set(G_OBJECT(browser_settings), settings[i].webkitvar, s, NULL);
         } else if(settings[i].type == 'c') {
            char value = argv[1][0];

            if(settings[i].variable) {
               char *x = (char*) (settings[i].variable);
               *x = value;
            }

            if(settings[i].webkitvar)
               g_object_set(G_OBJECT(browser_settings), settings[i].webkitvar, value, NULL);
         } else if(settings[i].type == '*'){
            // Special settings
            if(!strcmp(argv[0], "windowsize")){
               if(argc<3) return TRUE;

               int x = atoi(argv[1]);
               if(errno==ERANGE || x <= 0) x = default_width;
               int y = atoi(argv[2]);
               if(errno==ERANGE || y <= 0) y = default_height;

               gtk_window_resize(GTK_WINDOW(Client.UI.window), x, y);
            }
            if(!strcmp(argv[0], "proxy")){
               gboolean value = !strcmp(argv[1], "false") ? FALSE : TRUE;
               set_proxy(value);
            }
         }
      }
   }

   // check specific settings
   if(show_statusbar)   gtk_widget_show(GTK_WIDGET(Client.UI.statusbar));
   else                 gtk_widget_hide(GTK_WIDGET(Client.UI.statusbar));

   gtk_notebook_set_show_tabs(Client.UI.webview,   show_tabbar?TRUE:FALSE);

   update_client(gtk_notebook_get_current_page(Client.UI.webview));
   webkit_web_view_set_settings(GET_CURRENT_TAB(), browser_settings);
   return TRUE;
}

gboolean cmd_print(int argc, char** argv) {
   WebKitWebFrame* frame = webkit_web_view_get_main_frame(GET_CURRENT_TAB());
   if(!frame)      return FALSE;

   webkit_web_frame_print(frame);

   return TRUE;
}

gboolean cmd_tabopen(int argc, char** argv) {
   if(argc <= 0)           return TRUE;
   if(argv[argc] != NULL)  return TRUE;

   gchar* uri = g_strjoinv(" ", argv);
   if(strlen(uri)>0)
      create_tab(uri, FALSE);

   g_free(uri);

   return TRUE;
}

gboolean cmd_winopen(int argc, char** argv) {
   if(argc <= 0)   return TRUE;

   GString *uri = g_string_new("");

   uri = g_string_append(uri, argv[0]);
   for(int i=1; i<argc; i++) {
      uri = g_string_append_c(uri, ' ');
      uri = g_string_append(uri, argv[i]);
   }

   new_window(uri->str);
   g_string_free(uri, FALSE);

   return TRUE;
}

gboolean cmd_write(int argc, char** argv) {
   // save bookmarks
   GString *bookmark_list = g_string_new("");
   for(GList* l = Client.Global.bookmarks; l; l = g_list_next(l)){
      char* bookmark = g_strconcat((char*) l->data, "\n", NULL);
      bookmark_list = g_string_append(bookmark_list, bookmark);
      g_free(bookmark);
   }

   g_file_set_contents(bookmarks, bookmark_list->str, -1, NULL);

   g_string_free(bookmark_list, TRUE);

   // save history 
   int h_counter = 0;

   GString *history_list = g_string_new("");
   for(GList* h=Client.Global.history; h && (!history_limit || h_counter<history_limit); h=g_list_next(h)) {
      gchar* uri = g_strconcat((gchar*) h->data, "\n", NULL);
      history_list = g_string_append(history_list, uri);
      g_free(uri);

      h_counter++;
   }

   g_file_set_contents(history, history_list->str, -1, NULL);

   g_string_free(history_list, TRUE);

   // save session
   sessionsave("last_session");

   GString* session_list = g_string_new("");
   for(GList* se_list = Client.Global.sessions; se_list; se_list = g_list_next(se_list)) {
      Session* se = se_list->data;

      gchar* session_lines = g_strconcat(se->name, "\n", se->uris, "\n", NULL);
      session_list = g_string_append(session_list, session_lines);

      g_free(session_lines);
   }

   g_file_set_contents(sessions, session_list->str, -1, NULL);

   g_string_free(session_list, TRUE);

   return TRUE;
}


