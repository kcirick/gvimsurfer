/*
 * src/callbacks.c
 */
 
#include <ctype.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include <webkit/webkit.h>
#include <gdk/gdkkeysyms.h>
#include <JavaScriptCore/JavaScript.h>

#include "include/gvimsurfer.h"
#include "include/client.h"
#include "include/shortcuts.h"
#include "include/utilities.h"
#include "include/completion.h"
#include "include/commands.h"

#include "config/config.h"
#include "config/commands.h"
#include "config/binds.h"

gboolean cb_blank() { return TRUE; }

gboolean cb_button_close_tab(GtkButton *button, GtkNotebook *notebook) {
  gint page_id = GPOINTER_TO_INT( g_object_get_data( G_OBJECT( button ), "page" ) );
  close_tab(page_id);
  
  return TRUE;
}

gboolean cb_destroy(GtkWidget* widget, gpointer data) {
   pango_font_description_free(Client.Style.font);

   // write bookmarks and history
   if(!private_browsing)   cmd_write(0, NULL);

   // clear bookmarks
   for(GList* list = Client.Global.bookmarks; list; list = g_list_next(list))
      free(list->data);

   g_list_free(Client.Global.bookmarks);

   // clear history
   for(GList* list = Client.Global.history; list; list = g_list_next(list))
      free(list->data);

   g_list_free(Client.Global.history);

   // clean search engines 
   for(GList* list = Client.Global.search_engines; list; list = g_list_next(list))
      free(list->data);

   g_list_free(Client.Global.search_engines);

   // clean quickmarks
   for(GList* list = Client.Global.quickmarks; list; list = g_list_next(list))
      free(list->data);

   g_list_free(Client.Global.quickmarks);

   // clean pagemarks
   for(GList* list = Client.Global.pagemarks; list; list = g_list_next(list))
      free(list->data);

   g_list_free(Client.Global.pagemarks);

   gtk_main_quit();

   return TRUE;
}

gboolean cb_download_progress(WebKitDownload* d, GParamSpec* pspec){
   WebKitDownloadStatus status = webkit_download_get_status(d);

   if (status != WEBKIT_DOWNLOAD_STATUS_STARTED && status != WEBKIT_DOWNLOAD_STATUS_CREATED) {
      if (status != WEBKIT_DOWNLOAD_STATUS_FINISHED)
         notify(ERROR, g_strdup_printf("Error while downloading %s", webkit_download_get_suggested_filename(d)), -1);
      else
         notify(INFO, g_strdup_printf("Download %s finished", webkit_download_get_suggested_filename(d)), -1);
      Client.Global.active_downloads = g_list_remove(Client.Global.active_downloads, d);
   }
   update_statusbar_info();
   return TRUE;
}

gboolean cb_inputbar_activate(GtkEntry* entry, gpointer data) {
   gchar  *input  = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, -1);
   char identifier = input[0];
   gboolean  retv = FALSE;
   gboolean  succ = FALSE;
   Argument arg;

   // no input 
   if(strlen(input) <= 1) {
      isc_abort(NULL);
      g_free(input);
      return FALSE;
   }

   // search command 
   if(identifier == '/'){
      arg.n = FORWARD;
      arg.data = (char*)input+1;
      search_and_highlight(&arg);

      isc_abort(NULL);
      gtk_widget_grab_focus(GTK_WIDGET(GET_CURRENT_TAB_WIDGET()));
      g_free(input);
      return TRUE;
   }

   gchar **tokens = g_strsplit(input + 1, " ", -1);
   g_free(input);
   gchar *command = tokens[0];
   int     length = g_strv_length(tokens);

   // search commands
   for(unsigned int i = 0; i < LENGTH(commands); i++) {
      Command* c = (Command*)&commands[i];
      if((g_strcmp0(command, c->command) == 0)) {
         retv = c->function(length - 1, tokens + 1);
         succ = TRUE;
         break;
      }
   }

   if(!succ) notify(ERROR, "Unknown command.", -1);

   if(retv) isc_abort(NULL);
   else     set_inputbar_visibility(HIDE); 

   gtk_widget_grab_focus(GTK_WIDGET(GET_CURRENT_TAB_WIDGET()));
   //change_mode(NORMAL);
   g_strfreev(tokens);
   
   return TRUE;
}

gboolean cb_inputbar_kb_pressed(GtkWidget* widget, GdkEventKey* event, gpointer data) {
   guint keyval;
   GdkModifierType irrelevant;

   gdk_keymap_translate_keyboard_state(Client.Global.keymap, event->hardware_keycode, 
         event->state, event->group, &keyval, NULL, NULL, &irrelevant);

   gchar  *input  = gtk_editable_get_chars(GTK_EDITABLE(Client.UI.inputbar), 0, -1);
   if(strlen(input)<=1 && keyval==GDK_BackSpace)
      isc_abort(NULL);
   
   /* inputbar shortcuts */
   for(unsigned int i=0; i<LENGTH(inputbar_shortcuts); i++) {
      InputbarShortcut* isc = (InputbarShortcut*)&inputbar_shortcuts[i];
      if (keyval == isc->key                                         // test key
            && (event->state & ~irrelevant & ALL_MASK) == isc->mask) // test mask 
      {
         isc->function(&(isc->argument));
         return TRUE;
      }
   }
   return FALSE;
}

gboolean cb_wv_button_release_event(GtkWidget* widget, GdkEvent* event, gpointer data) {
   for(unsigned int i = 0; i < LENGTH(mouse); i++) {
      if( event->button.button == mouse[i].button              // test button
            && (event->button.state & ALL_MASK) == mouse[i].mask // test mask
            && Client.Global.mode & mouse[i].mode               // test mode
            && mouse[i].function // a function have to be declared
        ) {
         mouse[i].function(&(mouse[i].argument));
         return TRUE;
      }
   }

   // Clicking editable triggers insert mode
   WebKitHitTestResult *result;
   WebKitHitTestResultContext context;

   result = webkit_web_view_get_hit_test_result(WEBKIT_WEB_VIEW(widget), (GdkEventButton*)event);
   g_object_get(result, "context", &context, NULL);

   if (Client.Global.mode == NORMAL && event->type == GDK_BUTTON_RELEASE) {
      if (context & WEBKIT_HIT_TEST_RESULT_CONTEXT_EDITABLE)
         change_mode(INSERT);
   } else if (Client.Global.mode == INSERT && event->type == GDK_BUTTON_RELEASE) {
      if (!(context & WEBKIT_HIT_TEST_RESULT_CONTEXT_EDITABLE)) 
         change_mode(NORMAL);
   }
   return FALSE;
}

gboolean cb_wv_console_message(WebKitWebView* wv, char* message, int line, char* source, gpointer data) {

   if(!strcmp(message, "hintmode_off") || !strcmp(message, "insertmode_off"))
      change_mode(NORMAL);
   else if(!strcmp(message, "insertmode_on"))
      change_mode(INSERT);

   return TRUE;
}

GtkWidget* cb_wv_create_webview(WebKitWebView* wv, WebKitWebFrame* frame, gpointer data) {
   return create_tab((char*)webkit_web_view_get_uri(wv), TRUE);
}

gboolean cb_wv_download_request(WebKitWebView* wv, WebKitDownload* download, gpointer data) {

   download_content(download, (char*)webkit_download_get_suggested_filename(download));
   return TRUE;
}

gboolean cb_wv_hover_link(WebKitWebView* wv, char* title, char* link, gpointer data) {
   if(link) {
      link = g_strconcat("Link: ", link, NULL);
      gtk_label_set_text((GtkLabel*) Client.Statusbar.uri, link);
      g_free(link);
   } else
      gtk_label_set_text((GtkLabel*) Client.Statusbar.uri, webkit_web_view_get_uri(GET_CURRENT_TAB()));

   return TRUE;
}

gboolean cb_wv_kb_pressed(WebKitWebView *wv, GdkEventKey *event) {

   guint keyval;
   GdkModifierType irrelevant;
   gdk_keymap_translate_keyboard_state(Client.Global.keymap, event->hardware_keycode,
         event->state, event->group, &keyval, NULL, NULL, &irrelevant);

   if(keyval == GDK_Escape){
      Argument arg={0, NULL};
      sc_abort(&arg);
   } else {
      // Check the window element (likely editable if being typed...)
      gchar *value = NULL, *message = NULL;
      run_script("window.getSelection().focusNode", &value, &message);
      if (value && g_strrstr(value, "Element"))
         change_mode(INSERT);

      g_free(value);
      g_free(message);
   }

   return FALSE;
}

gboolean cb_tab_kb_pressed(WebKitWebView *wv, GdkEventKey *event) {

   guint keyval;
   GdkModifierType irrelevant;
   gdk_keymap_translate_keyboard_state(Client.Global.keymap, event->hardware_keycode,
         event->state, event->group, &keyval, NULL, NULL, &irrelevant);

   for(unsigned int i=0; i<LENGTH(shortcuts); i++){
      regex_t regex;
      Shortcut* sc = (Shortcut*)&shortcuts[i];

      int     status=1;
      if(sc->regex==NULL) status = Client.Global.buffer==NULL ? 0 : 1;
      else {
         regcomp(&regex, sc->regex, REG_EXTENDED);
         if(Client.Global.buffer!=NULL)
            status = regexec(&regex, Client.Global.buffer->str, (size_t) 0, NULL, 0);
         regfree(&regex);
      }

      if( keyval == sc->key                                          // test key
            && (event->state & ~irrelevant & ALL_MASK) == sc->mask   // test mask
            && (status==0 || keyval == GDK_Escape) 
            && Client.Global.mode!=INSERT
            && sc->function                     // a function have to be defined
        ) {
         sc->function(&(sc->argument));
         if(Client.Global.buffer!=NULL){
            g_string_free(Client.Global.buffer, TRUE);
            Client.Global.buffer = NULL;
            gtk_label_set_text((GtkLabel*) Client.Statusbar.message, "");
         }
         return TRUE;
      }
   }

   switch(Client.Global.mode) {
      case PASS_THROUGH :
         return FALSE;
      case PASS_THROUGH_NEXT :
         change_mode(NORMAL);
         return FALSE;
   }

   // append only numbers and characters to buffer
   if(Client.Global.mode==NORMAL && isascii(keyval)) {
      if(!Client.Global.buffer)
         Client.Global.buffer = g_string_new("");

      Client.Global.buffer = g_string_append_c(Client.Global.buffer, keyval);
      gtk_label_set_text((GtkLabel*) Client.Statusbar.message, Client.Global.buffer->str);
   }

   // follow hints
   if(Client.Global.mode == FOLLOW) {
      Argument argument = {0, event};
      sc_follow_link(&argument);
      return TRUE;
   }
   return FALSE;
}

gboolean cb_wv_mimetype_policy_decision(WebKitWebView* wv, WebKitWebFrame* frame,
      WebKitNetworkRequest* request, char* mimetype, WebKitWebPolicyDecision* decision, gpointer data) {
   if(!webkit_web_view_can_show_mime_type(wv, mimetype)) {
      webkit_web_policy_decision_download(decision);
      return TRUE;
   }
   return FALSE;
}

gboolean cb_wv_nav_policy_decision(WebKitWebView* wv, WebKitWebFrame* frame,
      WebKitNetworkRequest* request, WebKitWebNavigationAction* action,
      WebKitWebPolicyDecision* decision, gpointer data) {
   switch(webkit_web_navigation_action_get_button(action)) {
      case 1: /* left mouse button */
         return FALSE;
      case 2: /* middle mouse button */
         create_tab((char*) webkit_network_request_get_uri(request), TRUE);
         webkit_web_policy_decision_ignore(decision);
         return TRUE;
      case 3: /* right mouse button */
         return FALSE;
      default:
         return FALSE;;
   }
}

gboolean cb_wv_notify_progress(WebKitWebView* wv, GParamSpec* pspec, gpointer data) {
   if(wv == GET_CURRENT_TAB() && gtk_notebook_get_current_page(Client.UI.webview) != -1)
      update_statusbar_info();

   return TRUE;
}

gboolean cb_wv_notify_title(WebKitWebView* wv, GParamSpec* pspec, gpointer data) {
   if(webkit_web_view_get_title(wv)) 
      update_client();

   return TRUE;
}

gboolean cb_wv_window_object_cleared(WebKitWebView* wv, WebKitWebFrame* frame, gpointer context,
      gpointer window_object, gpointer data) {

   /* load all added scripts */
   JSStringRef script;
   JSValueRef exc;
   gchar *buffer = Client.Global.user_script->content;

   script = JSStringCreateWithUTF8CString(buffer);
   JSEvaluateScript((JSContextRef)context, script, JSContextGetGlobalObject((JSContextRef)context), NULL, 0, &exc);
   JSStringRelease(script);

   if(!g_object_get_data(G_OBJECT(GET_CURRENT_TAB()), "loaded_scripts")) 
      run_script(buffer, NULL, NULL);

   g_object_set_data(G_OBJECT(GET_CURRENT_TAB()), "loaded_scripts",  (gpointer) 1);

   return TRUE;
}

gboolean cb_wv_window_policy_decision(WebKitWebView* wv, WebKitWebFrame* frame, WebKitNetworkRequest* request,
      WebKitWebNavigationAction* action, WebKitWebPolicyDecision* decision, gpointer data) {
   if(webkit_web_navigation_action_get_reason(action) == WEBKIT_WEB_NAVIGATION_REASON_LINK_CLICKED) {
      webkit_web_policy_decision_ignore(decision);
      //new_window((char*) webkit_network_request_get_uri(request));
      create_tab((char*) webkit_network_request_get_uri(request), TRUE);
      return TRUE;
   }
   return FALSE;
}

gboolean cb_wv_scrolled(GtkAdjustment* adjustment, gpointer data) {
   update_statusbar_info();
   return TRUE;
}

