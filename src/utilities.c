/*
 * src/utilities.c
 */

#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include <webkit/webkit.h>
#include <JavaScriptCore/JavaScript.h>

#include "include/gvimsurfer.h"
#include "include/utilities.h"
#include "include/client.h"
#include "include/callbacks.h"
#include "include/completion.h"
#include "include/commands.h"

#include "config/config.h"

#define COLOUR_RED     "\x1b[31m"
#define COLOUR_GREEN   "\x1b[32m"
#define COLOUR_YELLOW  "\x1b[33m"
#define COLOUR_RESET   "\x1b[0m"

void say(int type, const char *message, int exit_type){
   gchar* coloured_type;
   if(type==ERROR)   
      coloured_type=g_strdup_printf(COLOUR_RED "%s" COLOUR_RESET, "ERROR");
   else if(type==WARNING)
      coloured_type=g_strdup_printf(COLOUR_YELLOW "%s" COLOUR_RESET, "WARNING");
   else
      coloured_type=g_strdup_printf(COLOUR_GREEN "%s" COLOUR_RESET, "INFO");

   fprintf(stderr, "%s [%s]:\t%s\n", NAME, coloured_type, message);
   if(exit_type==EXIT_SUCCESS || exit_type==EXIT_FAILURE ) 
      exit(exit_type);
}

void notify(int level, char* message) {
   if(!message || strlen(message) <= 0) return;

   if(level==ERROR || level==WARNING)
      gtk_widget_modify_fg(GTK_WIDGET(Client.Statusbar.message), GTK_STATE_NORMAL, &(Client.Style.notification_fg));
   else 
      gtk_widget_modify_fg(GTK_WIDGET(Client.Statusbar.message), GTK_STATE_NORMAL, &(Client.Style.statusbar_fg));

   gtk_label_set_text((GtkLabel*) Client.Statusbar.message, message);
}

void open_uri(WebKitWebView* web_view, char* uri) {
   if(!uri)              return;
   while (*uri == ' ')   uri++;

   gchar* new_uri = NULL;

   char* uri_first_space = strchr(uri, ' ');
   if(uri_first_space) {   // multiple argument given
      unsigned int first_arg_length = uri_first_space - uri;

      /* first agrument contain "://" -> it's a bookmark with tag */
      if(strstr(uri, "://")) {
         new_uri = g_strndup(uri, first_arg_length);
      }
      /* first agrument doesn't contain "://" -> use search engine */
      else {
         SearchEngine* se;
         for(GList* list = Client.Global.search_engines; list; list = g_list_next(list)){
            se = (SearchEngine*)list->data;
            if(strlen(se->name) == first_arg_length && !strncmp(uri, se->name, first_arg_length)){
               break;
            }
         }

         if(!se)
            se = (SearchEngine*)g_list_first(Client.Global.search_engines)->data;
         else /* we remove the trailing arg since it's the se name */
            uri = uri + first_arg_length + 1;

         new_uri = g_strdup_printf(se->uri, uri);

         printf("%s\n", new_uri);
         /* we change all the space with '+'
          * -2 for the '%s'
          */
         char* new_uri_it = new_uri + strlen(se->uri) - 2;

         while(*new_uri_it) {
            if(*new_uri_it == ' ') *new_uri_it = '+';

            new_uri_it++;
         }
      }
   } else if(strlen(uri) == 0) {   // no argument given
      new_uri = g_strdup(home_page);
   } else {                        // only one argument given
      // file path
      if(uri[0] == '/' || strncmp(uri, "./", 2) == 0) {
         new_uri = g_strconcat("file://", uri, NULL);
      }
      // uri does contain any ".", ":" or "/" nor does it start with "localhost"
      else if(!strpbrk(uri, ".:/") && strncmp(uri, "localhost", 9)) {
         new_uri = g_strconcat("http://", uri, NULL);
      } else
         new_uri = strstr(uri, "://") ? g_strdup(uri) : g_strconcat("http://", uri, NULL);
   }
   webkit_web_view_load_uri(web_view, new_uri);

   // update history
   if(!private_browsing) {
      /* we verify if the new_uri is already present in the list*/
      GList* l = g_list_find_custom(Client.Global.history, new_uri, (GCompareFunc)strcmp);
      if (l) {
         /* new_uri is already present : move it to the end of the list */
         Client.Global.history = g_list_remove_link(Client.Global.history, l);
         Client.Global.history = g_list_concat(l, Client.Global.history);
      } else 
         Client.Global.history = g_list_prepend(Client.Global.history, g_strdup(new_uri));
   }
   g_free(new_uri);

   update_client();
}

void set_proxy(gboolean onoff) {
   gchar   *filename, *new;

   if(!onoff) {
      g_object_set(Client.Global.soup_session, "proxy-uri", NULL, NULL);

      notify(INFO, "Proxy deactivated");
   } else {
      filename = (char*) g_getenv("http_proxy");
      if(filename==NULL)   filename = (char*) g_getenv("HTTP_PROXY");

      if(filename==NULL) {
         say(WARNING, "No proxy defined", -1);
         return;
      }

      new = g_strrstr(filename, "://") ? g_strdup(filename) : g_strconcat("http://", filename, NULL);
      SoupURI* proxy_uri = soup_uri_new(new);

      g_object_set(Client.Global.soup_session, "proxy-uri", proxy_uri, NULL);

      soup_uri_free(proxy_uri);
      g_free(new);

      notify(INFO, "Proxy activated");
   }
}

void change_mode(int mode) {
   char* mode_text = NULL;

   switch(mode) {
      case INSERT:
         mode_text = "-- INSERT --";
         break;
      case VISUAL:
         mode_text = "-- VISUAL --";
         break;
      case FOLLOW:
         mode_text = "-- FOLLOW -- ";
         break;
      case PASS_THROUGH:
         mode_text = "-- PASS THROUGH --";
         break;
      case PASS_THROUGH_NEXT:
         mode_text = "-- PASS THROUGH (next)--";
         break;
      default:
         mode_text = "";
         mode      = NORMAL;
         //gtk_widget_grab_focus(GTK_WIDGET(GET_CURRENT_TAB()));
         gtk_widget_grab_focus(GTK_WIDGET(GET_CURRENT_TAB_WIDGET()));
         break;
   }
   Client.Global.mode = mode;
   gtk_label_set_text((GtkLabel*) Client.Statusbar.message, mode_text);
}

char* reference_to_string(JSContextRef context, JSValueRef reference) {
   if(!context || !reference)     return NULL;

   JSStringRef ref_st = JSValueToStringCopy(context, reference, NULL);
   size_t      length = JSStringGetMaximumUTF8CStringSize(ref_st);
   gchar*      string = g_new(gchar, length);
   JSStringGetUTF8CString(ref_st, string, length);
   JSStringRelease(ref_st);

   return string;
}

void run_script(char* script, char** value, char** error) {
   if(!script)     return;

   WebKitWebFrame *frame = webkit_web_view_get_main_frame(GET_CURRENT_TAB());
   if(!frame)      return;

   JSContextRef context  = webkit_web_frame_get_global_context(frame);
   JSStringRef sc        = JSStringCreateWithUTF8CString(script);
   if(!context || !sc)      return;

   JSObjectRef ob = JSContextGetGlobalObject(context);
   if(!ob)      return;

   JSValueRef exception = NULL;
   JSValueRef va   = JSEvaluateScript(context, sc, ob, NULL, 0, &exception);
   JSStringRelease(sc);

   if(!va && error)
      *error = reference_to_string(context, exception);
   else if(value)
      *value = reference_to_string(context, va);
}

void download_content(WebKitDownload* download, char* filename){

   WebKitDownloadStatus status;

   gchar* file      = g_build_filename(g_strdup(download_dir), filename ? filename : "vimsurfer_download", NULL);
   gchar* download_path_uri = g_strconcat("file://", file, NULL);

   webkit_download_set_destination_uri(download, download_path_uri);
   g_free(download_path_uri);

   uint32_t size = (uint32_t)webkit_download_get_total_size(download);
   if(size>0)
      notify(INFO, g_strdup_printf("Download %s started (expected size: %u bytes)...", filename, size));
   else
      notify(INFO, g_strdup_printf("Download %s started (unknown size)...", filename));

   Client.Global.active_downloads = g_list_prepend(Client.Global.active_downloads, download);
   g_signal_connect(download, "notify::progress", G_CALLBACK(cb_download_progress), NULL);
   g_signal_connect(download, "notify::status",   G_CALLBACK(cb_download_progress), NULL);
   
   status = webkit_download_get_status(download);
   if(status == WEBKIT_DOWNLOAD_STATUS_CREATED)
      webkit_download_start(download);

   g_free(file);

}

gboolean read_configuration(char* configrc) {
   if(!configrc) return FALSE;
   if(!g_file_test(configrc, G_FILE_TEST_IS_REGULAR)) return FALSE;

   gchar* content = NULL;
   if(!g_file_get_contents(configrc, &content, NULL, NULL)) return FALSE;

   gchar **lines = g_strsplit(content, "\n", -1);
   gint    n     = g_strv_length(lines) - 1;

   for(gint i = 0; i <= n; i++) {
      if(!strlen(lines[i]) || lines[i][0]=='#') continue;

      gchar* id = strtok(lines[i], "=");
      gchar* value = strtok(NULL, "\n");

      // remove whitespaces
      id = g_strstrip(id);
      value = g_strstrip(value);
      if(!strlen(value)) continue;

      if(!strcmp(id, "default_width"))    default_width = atoi(value);
      if(!strcmp(id, "default_height"))   default_height = atoi(value);
      if(!strcmp(id, "max_title_length")) max_title_length = atoi(value);

      if(!strcmp(id, "private_browsing"))    private_browsing = strcmp(value, "false") ? TRUE : FALSE;
      if(!strcmp(id, "full_content_zoom"))   full_content_zoom = strcmp(value, "false") ? TRUE : FALSE;
      if(!strcmp(id, "show_scrollbars"))     show_scrollbars = strcmp(value, "false") ? TRUE : FALSE;
      if(!strcmp(id, "show_statusbar"))      show_statusbar = strcmp(value, "false") ? TRUE : FALSE;
      if(!strcmp(id, "show_tabbar"))         show_tabbar = strcmp(value, "false") ? TRUE : FALSE;
      if(!strcmp(id, "strict_ssl"))          strict_ssl = strcmp(value, "false") ? TRUE : FALSE;

      if(!strcmp(id, "home_page"))        home_page = value;
      if(!strcmp(id, "user_agent"))       user_agent = value;
      if(!strcmp(id, "external_editor"))  external_editor = value;
      if(!strcmp(id, "ca_bundle"))        ca_bundle = value;

      if(!strcmp(id, "n_completion_items"))    n_completion_items = atoi(value);
      if(!strcmp(id, "history_limit"))         history_limit = atoi(value);
      if(!strcmp(id, "zoom_step"))             zoom_step = atof(value);
      if(!strcmp(id, "scroll_step"))           scroll_step = atof(value);

      if(!strcmp(id, "download_dir")) download_dir = value;
      if(!strcmp(id, "config_dir"))   config_dir = value;
      if(!strcmp(id, "bookmarks"))    bookmarks = value;
      if(!strcmp(id, "history"))      history = value;
      if(!strcmp(id, "cookies"))      cookies = value;
      if(!strcmp(id, "sessions"))     sessions = value;
      if(!strcmp(id, "stylesheet"))   stylesheet = value;

      // Search Engines
      if(!strcmp(id, "search_engine")){
         gchar **entries = g_strsplit_set(value, " ", -1);
         gint    num_entries = g_strv_length(entries);
         if(num_entries !=2) say(WARNING, "Numbers of entries is not 2!", -1);
         
         SearchEngine* sengine = malloc(sizeof(SearchEngine));
         sengine->name = entries[0];
         sengine->uri  = entries[1];

         Client.Global.search_engines = g_list_append(Client.Global.search_engines, sengine);
         g_free(entries);
      }

      // Loading Scripts
      if(!strcmp(id, "scriptfile")){
         char* my_argv[1] = { g_strdup_printf("~/%s/%s", config_dir, value) };
         cmd_script(1, my_argv);
      }

      // Appearance
      if(!strcmp(id, "font"))   
         Client.Style.font = pango_font_description_from_string(value);
      if(!strcmp(id, "statusbar_color")){
         gchar   **colors  = g_strsplit_set(value, " ", -1);
         gint   num_colors = g_strv_length(colors);
         if(num_colors !=5) say(WARNING, "Numbers of colors is not 5!", -1);
         
         gdk_color_parse(colors[0],    &(Client.Style.statusbar_bg));
         gdk_color_parse(colors[1],    &(Client.Style.statusbar_fg));
         gdk_color_parse(colors[2],    &(Client.Style.statusbar_ssl_fg));
         gdk_color_parse(colors[3],    &(Client.Style.inputbar_fg));
         gdk_color_parse(colors[4],    &(Client.Style.notification_fg));
         g_free(colors);
      }
      if(!strcmp(id, "completion_color")){
         gchar    **colors = g_strsplit_set(value, " ", -1);
         gint   num_colors = g_strv_length(colors);
         if(num_colors !=3) say(WARNING, "Numbers of colors is not 3!", -1);
         
         gdk_color_parse(colors[0],    &(Client.Style.completion_bg));
         gdk_color_parse(colors[1],    &(Client.Style.completion_fg));
         gdk_color_parse(colors[2],    &(Client.Style.completion_hl));
         g_free(colors);
      }
   }
   g_free(lines);

   return TRUE;
}

void load_all_scripts() {
   int ls = (size_t) g_object_get_data(G_OBJECT(GET_CURRENT_TAB()), "loaded_scripts");

   if(!ls) {
      GList* sl = Client.Global.scripts;
      while(sl) {
         run_script(((Script*)sl)->content, NULL, NULL);
         sl = g_list_next(sl);
      }
   }
   g_object_set_data(G_OBJECT(GET_CURRENT_TAB()), "loaded_scripts",  (gpointer) 1);
}

gboolean search_and_highlight(Argument* argument) {
   static WebKitWebView* last_wv = NULL;
   gboolean search_handle_changed = FALSE;

   if(!Client.Global.search_handle || !strlen(Client.Global.search_handle)){
      if(argument->data)
         Client.Global.search_handle = g_strdup(argument->data);
      else
         return FALSE;
   }

   if(argument->data && strcmp(Client.Global.search_handle, (gchar*)argument->data)!=0){
      Client.Global.search_handle = g_strdup(argument->data);
      search_handle_changed = TRUE;
   }

   WebKitWebView* current_wv = GET_CURRENT_TAB();
   
   if(search_handle_changed || last_wv != current_wv) {
      webkit_web_view_unmark_text_matches(current_wv);
      webkit_web_view_mark_text_matches(current_wv, Client.Global.search_handle, FALSE, 0);
      webkit_web_view_set_highlight_text_matches(current_wv, TRUE);

      last_wv = current_wv;
   }
   gboolean direction = (argument->n == BACKWARD) ? FALSE : TRUE;
   webkit_web_view_search_text(current_wv, Client.Global.search_handle, FALSE, direction, TRUE);

   return FALSE;
}

gboolean sessionsave(char* session_name) {
   GString* session_uris = g_string_new("");

   for (int i = 0; i < gtk_notebook_get_n_pages(Client.UI.webview); i++) {
      gchar* tab_uri   = g_strconcat(webkit_web_view_get_uri(GET_NTH_TAB(i)), " ", NULL);
      session_uris     = g_string_append(session_uris, tab_uri);

      g_free(tab_uri);
   }

   GList* se_list = Client.Global.sessions;
   while(se_list) {
      Session* se = se_list->data;

      if(g_strcmp0(se->name, session_name) == 0) {
         g_free(se->uris);
         se->uris = session_uris->str;

         break;
      }
      se_list = g_list_next(se_list);
   }

   if(!se_list) {
      Session* se = malloc(sizeof(Session));
      se->name = g_strdup(session_name);
      se->uris = session_uris->str;

      Client.Global.sessions = g_list_prepend(Client.Global.sessions, se);
   }

   g_string_free(session_uris, FALSE);

   return TRUE;
}

gboolean sessionload(char* session_name) {
   GList* se_list = Client.Global.sessions;
   while(se_list) {
      Session* se = se_list->data;

      if(g_strcmp0(se->name, session_name) == 0) {
         gchar** uris = g_strsplit(se->uris, " ", -1);
         int     n    = g_strv_length(uris) - 1;

         if(n <= 0)  return FALSE;

         for(int i = 0; i < n; i++)
            create_tab(uris[i], TRUE);

         g_strfreev(uris);
         return TRUE;
      }
      se_list = g_list_next(se_list);
   }

   return FALSE;
}


