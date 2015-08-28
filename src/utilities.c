/*
 * src/utilities.c
 */

#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <webkit/webkit.h>
#include <JavaScriptCore/JavaScript.h>

#include "include/gvimsurfer.h"
#include "include/utilities.h"
#include "include/client.h"
#include "include/callbacks.h"
#include "include/completion.h"

#include "config/config.h"

#define COLOUR_RED     "\x1b[31m"
#define COLOUR_GREEN   "\x1b[32m"
#define COLOUR_YELLOW  "\x1b[33m"
#define COLOUR_PURPLE  "\x1b[35m"
#define COLOUR_RESET   "\x1b[0m"

//--- Function declaration -----
char* reference_to_string(JSContextRef, JSValueRef);


void notify(gint level, gchar* message) {
   gboolean output_stderr=FALSE; // set to TRUE for debugging

   if(Client.UI.window){
      if(level==ERROR || level==WARNING)
         gtk_widget_modify_fg(GTK_WIDGET(Client.Statusbar.message), GTK_STATE_NORMAL, &(Client.Style.notification_fg));
      else 
         gtk_widget_modify_fg(GTK_WIDGET(Client.Statusbar.message), GTK_STATE_NORMAL, &(Client.Style.statusbar_fg));

      gtk_label_set_text((GtkLabel*) Client.Statusbar.message, message);
   }

   if(!Client.UI.window || output_stderr){
      gchar* coloured_type;
      if(level==ERROR)   
         coloured_type=g_strdup(COLOUR_RED "ERROR" COLOUR_RESET);
      else if(level==WARNING)
         coloured_type=g_strdup(COLOUR_YELLOW "WARNING" COLOUR_RESET);
      else if(level==DEBUG)
         coloured_type=g_strdup(COLOUR_PURPLE "DEBUG" COLOUR_RESET);
      else
         coloured_type=g_strdup(COLOUR_GREEN "INFO" COLOUR_RESET);

      g_printf("%s [%s]:\t%s\n", NAME, coloured_type, message);
   }
}

void die(gint level, gchar* message, gint exit_type) {
   gchar* coloured_type;
   if(level==ERROR)   
      coloured_type=g_strdup(COLOUR_RED "ERROR" COLOUR_RESET);
   else if(level==WARNING)
      coloured_type=g_strdup(COLOUR_YELLOW "WARNING" COLOUR_RESET);
   else if(level==DEBUG)
      coloured_type=g_strdup(COLOUR_PURPLE "DEBUG" COLOUR_RESET);
   else
      coloured_type=g_strdup(COLOUR_GREEN "INFO" COLOUR_RESET);

   g_printf("%s [%s]:\t%s\n", NAME, coloured_type, message);

   if(exit_type==EXIT_SUCCESS || exit_type==EXIT_FAILURE ) 
      exit(exit_type);
}

void open_uri(WebKitWebView* web_view, const gchar* uri) {
   if(!uri || strlen(uri)==0) return;

   gchar* new_uri = NULL;

   gchar **args = g_strsplit(uri, " ", -1);
   gint   nargs = g_strv_length(args);

   if(nargs==1){  // only one argument given

      // file path
      if(uri[0] == '/' || strncmp(uri, "./", 2) == 0) 
         new_uri = g_strconcat("file://", uri, NULL);
      // uri does contain any ".", ":" or "/" nor does it start with "localhost"
      else if(!strpbrk(uri, ".:/") && strncmp(uri, "localhost", 9)){
         //new_uri = g_strconcat("http://", uri, NULL);
         SearchEngine* se = (SearchEngine*)Client.Global.search_engines->data;
         new_uri = g_strdup_printf(se->uri, uri);
      } else
         new_uri = strstr(uri, "://") ? g_strdup(uri) : g_strconcat("http://", uri, NULL);
   
   } else {       // multiple arguments given

      /* first agrument contain "://" -> it's a bookmark with tag */
      if(g_strrstr(args[0], "://")) {
         new_uri = g_strdup(args[0]);
      }
      /* first agrument doesn't contain "://" -> use search engine */
      else {
         SearchEngine* se;
         gboolean matched=FALSE;
         for(GList* list = Client.Global.search_engines; list; list = g_list_next(list)){
            se = (SearchEngine*)list->data;
            if(g_strcmp0(args[0], se->name)==0){ matched=TRUE; break; }
         }

         if(!matched){
            notify(WARNING, g_strdup_printf("Search engine %s doesn't exist", args[0]));
            se = (SearchEngine*)Client.Global.search_engines->data;
            uri = g_strjoinv(" ", args);
            //return;
         } else 
            uri = g_strjoinv(" ", &args[1]);

         new_uri = g_strdup_printf(se->uri, uri);
      }

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
   g_free(args);
   g_free(new_uri);

   update_client(gtk_notebook_get_current_page(Client.UI.webview));
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
         notify(WARNING, "No proxy defined");
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

   JSGlobalContextRef context = webkit_web_frame_get_global_context(frame);
   JSStringRef sc             = JSStringCreateWithUTF8CString(script);
   if(!context || !sc)      return;

   JSValueRef exception, val; 
   val = JSEvaluateScript(context, sc, JSContextGetGlobalObject(context), NULL, 0, &exception);
   JSStringRelease(sc);

   if(!val && error)
      *error = reference_to_string(context, exception);
   else if(value)
      *value = reference_to_string(context, val);
}

void download_content(WebKitDownload* download, const gchar* filename){

   WebKitDownloadStatus status;

   gchar* download_path_uri   = g_strconcat("file://", download_dir, filename?filename:"download", NULL);

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
}

gchar* build_proper_path(gchar* path){

   gchar* proper_path;
   if(path[0]=='~')        proper_path = g_build_filename(g_get_home_dir(), path+1, NULL);
   else if(path[0]=='/')   proper_path = g_strdup(path);
   else {
      notify(WARNING, g_strdup_printf("Path %s is vague", path));
      proper_path = g_strdup(path);
   }

   return proper_path;
}

gboolean read_configuration(gchar* configrc) {
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

      if(!strcmp(id, "full_content_zoom"))   full_content_zoom = strcmp(value, "false") ? TRUE : FALSE;
      if(!strcmp(id, "show_scrollbars"))     show_scrollbars = strcmp(value, "false") ? TRUE : FALSE;
      if(!strcmp(id, "show_statusbar"))      show_statusbar = strcmp(value, "false") ? TRUE : FALSE;
      if(!strcmp(id, "show_tabbar"))         show_tabbar = strcmp(value, "false") ? TRUE : FALSE;
      if(!strcmp(id, "strict_ssl"))          strict_ssl = strcmp(value, "false") ? TRUE : FALSE;

      if(!strcmp(id, "home_page"))        home_page = value;
      if(!strcmp(id, "user_agent"))       user_agent = value;
      if(!strcmp(id, "external_editor"))  external_editor = value;

      if(!strcmp(id, "n_completion_items"))    n_completion_items = atoi(value);
      if(!strcmp(id, "history_limit"))         history_limit = atoi(value);
      if(!strcmp(id, "zoom_step"))             zoom_step = atof(value);
      if(!strcmp(id, "scroll_step"))           scroll_step = atof(value);

      if(!strcmp(id, "download_dir")) download_dir = build_proper_path(value);
      if(!strcmp(id, "config_dir"))   config_dir   = build_proper_path(value);
      if(!strcmp(id, "bookmarks"))    bookmarks    = g_strconcat(config_dir, value, NULL);
      if(!strcmp(id, "history"))      history      = g_strconcat(config_dir, value, NULL);
      if(!strcmp(id, "cookies"))      cookies      = g_strconcat(config_dir, value, NULL);
      if(!strcmp(id, "sessions"))     sessions     = g_strconcat(config_dir, value, NULL);
      if(!strcmp(id, "stylesheet"))   stylesheet   = g_strconcat(config_dir, value, NULL);
      if(!strcmp(id, "scriptfile"))   load_script(g_strconcat(config_dir, value, NULL));

      // Search Engines
      if(!strcmp(id, "search_engine")){
         gchar **entries = g_strsplit_set(value, " ", -1);
         gint    num_entries = g_strv_length(entries);
         if(num_entries !=2) notify(WARNING, "Numbers of entries is not 2!");
         
         SearchEngine* sengine = malloc(sizeof(SearchEngine));
         sengine->name = entries[0];
         sengine->uri  = entries[1];

         Client.Global.search_engines = g_list_append(Client.Global.search_engines, sengine);
         g_free(entries);
      }

      // Appearance
      if(!strcmp(id, "font"))   
         Client.Style.font = pango_font_description_from_string(value);
      if(!strcmp(id, "statusbar_color")){
         gchar   **colors  = g_strsplit_set(value, " ", -1);
         gint   num_colors = g_strv_length(colors);
         if(num_colors !=5) notify(WARNING, "Numbers of colors is not 5!");
         
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
         if(num_colors !=3) notify(WARNING, "Numbers of colors is not 3!");
         
         gdk_color_parse(colors[0],    &(Client.Style.completion_bg));
         gdk_color_parse(colors[1],    &(Client.Style.completion_fg));
         gdk_color_parse(colors[2],    &(Client.Style.completion_hl));
         g_free(colors);
      }
   }
   g_free(lines);

   return TRUE;
}

gboolean load_script(gchar* path){

   gchar* file = build_proper_path(path);
   if(!g_file_test(file, G_FILE_TEST_IS_REGULAR)) return FALSE;

   char* content = NULL;
   if(!g_file_get_contents(file, &content, NULL, NULL)){
      notify(ERROR, g_strdup_printf("Could not open or read file '%s'", path));
      return FALSE;
   }

   // search for existing script to overwrite or reread it
   Script* scr = Client.Global.user_script;
   if(scr && !strcmp(scr->path, path)) {
      scr->path    = path;
      scr->content = content;
      return TRUE;
   }

   // load new script
   Client.Global.user_script = malloc(sizeof(Script));
   if(!Client.Global.user_script) die(ERROR, "Out of memory", EXIT_FAILURE);

   Client.Global.user_script->path    = path;
   Client.Global.user_script->content = content;

   return TRUE;
}

void search_and_highlight(gboolean direction, gchar* token) {
   static WebKitWebView* last_wv = NULL;
   gboolean search_handle_changed = FALSE;

   if(!Client.Global.search_handle || !strlen(Client.Global.search_handle)){
      if(token)
         Client.Global.search_handle = g_strdup(token);
      else
         return;
   }

   if(token && strcmp(Client.Global.search_handle, token)!=0){
      Client.Global.search_handle = g_strdup(token);
      search_handle_changed = TRUE;
   }

   WebKitWebView* current_wv = GET_CURRENT_TAB();
   
   if(search_handle_changed || last_wv != current_wv) {
      webkit_web_view_unmark_text_matches(current_wv);
      webkit_web_view_mark_text_matches(current_wv, Client.Global.search_handle, FALSE, 0);
      webkit_web_view_set_highlight_text_matches(current_wv, TRUE);

      last_wv = current_wv;
   }
   webkit_web_view_search_text(current_wv, Client.Global.search_handle, FALSE, direction, TRUE);

}

void clear_input() {
   run_completion(HIDE);

   gtk_label_set_text((GtkLabel*) Client.Statusbar.message, "");
   change_mode(NORMAL);
   gtk_widget_grab_focus(GTK_WIDGET(GET_CURRENT_TAB_WIDGET()));
   set_inputbar_visibility(HIDE);
}

gint get_int_from_buffer(gchar* buffer){

   gint digit_end = 0;
   while(g_ascii_isdigit(buffer[digit_end]))
      digit_end = digit_end + 1;

   gchar* number = g_strndup(buffer, digit_end);
   gint id = atoi(number);
   g_free(number);

   return id;
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

