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
#include "include/flashblock.h"

#include "config/config.h"
#include "config/commands.h"
#include "config/binds.h"

static gint page_id;

gboolean cb_blank() { return TRUE; }

gboolean cb_destroy(GtkWidget* widget, gpointer data) {
   cmd_quitall(0, NULL);
   return TRUE;
}

gboolean cb_download_progress(WebKitDownload* d, GParamSpec* pspec){
   WebKitDownloadStatus status = webkit_download_get_status(d);
   const gchar* filename = webkit_download_get_suggested_filename(d);

   if (status != WEBKIT_DOWNLOAD_STATUS_STARTED && status != WEBKIT_DOWNLOAD_STATUS_CREATED) {
      if (status == WEBKIT_DOWNLOAD_STATUS_FINISHED)
         notify(INFO, g_strdup_printf("Download %s finished", filename));
      if (status == WEBKIT_DOWNLOAD_STATUS_ERROR)
         notify(ERROR, g_strdup_printf("Error while downloading %s", filename));
      if (status == WEBKIT_DOWNLOAD_STATUS_CANCELLED)
         notify(ERROR, g_strdup_printf("Cancelled downloading %s", filename));

      Client.Global.active_downloads = g_list_remove(Client.Global.active_downloads, d);
   }

   update_statusbar_info(gtk_notebook_get_current_page(Client.UI.webview));
   return TRUE;
}

gboolean cb_button_add_tab(GtkButton *button, GtkNotebook *notebook) {
  gint page_id = GPOINTER_TO_INT( g_object_get_data( G_OBJECT( button ), "page" ) );
  create_tab(NULL, page_id);
  
  return TRUE;
}

gboolean cb_button_close_tab(GtkButton *button, GtkNotebook *notebook) {
  gint page_id = GPOINTER_TO_INT( g_object_get_data( G_OBJECT( button ), "page" ) );
  close_tab(page_id);
  
  return TRUE;
}

gboolean cb_notebook_switch_page(GtkNotebook *notebook, gpointer page, guint page_num, gpointer user_data) {
   page_id = gtk_notebook_get_current_page(notebook);
   return TRUE;
}


gboolean cb_notebook_switch_page_after(GtkNotebook *notebook, gpointer page, guint page_num, gpointer user_data) {
   GtkWidget* addpage = g_object_get_data(G_OBJECT(notebook), "addtab");
   if(GTK_WIDGET(page) == addpage){
      gtk_notebook_set_current_page(notebook, page_id);
      return FALSE;
   } 
   
   update_client(page_num);

   return TRUE;
}

gboolean cb_inputbar_activate(GtkEntry* entry, gpointer data) {
   gchar  *input  = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, -1);
   char identifier = input[0];
   gboolean  retv = FALSE;
   gboolean  succ = FALSE;

   // no input 
   if(strlen(input) <= 1) {
      clear_input();
      g_free(input);
      return FALSE;
   }

   // search command 
   if(identifier == '/'){
      search_and_highlight(FORWARD, (gchar*)input+1);

      clear_input();
      g_free(input);
      return TRUE;
   }

   gchar **tokens = g_strsplit(input + 1, " ", -1);
   g_free(input);
   gchar *command = tokens[0];
   gint    length = g_strv_length(tokens);

   // search commands
   for(unsigned int i = 0; i < LENGTH(commands); i++) {
      Command* c = (Command*)&commands[i];
      if((g_strcmp0(command, c->command) == 0)) {
         retv = c->function(length - 1, tokens + 1);
         succ = TRUE;
         break;
      }
   }

   if(!succ) notify(ERROR, "Unknown command");

   if(retv) clear_input();
   else     set_inputbar_visibility(HIDE); 

   g_strfreev(tokens);

   gtk_widget_grab_focus(GTK_WIDGET(GET_CURRENT_TAB_WIDGET()));
   
   return TRUE;
}

gboolean cb_inputbar_kb_pressed(GtkWidget* widget, GdkEventKey* event, gpointer data) {
   guint keyval;
   GdkModifierType irrelevant;

   gdk_keymap_translate_keyboard_state(Client.Global.keymap, event->hardware_keycode, 
         event->state, event->group, &keyval, NULL, NULL, &irrelevant);

   gchar  *input  = gtk_editable_get_chars(GTK_EDITABLE(Client.UI.inputbar), 0, -1);
   if(keyval==GDK_Escape || (strlen(input)<=1 && keyval==GDK_BackSpace))
      clear_input();
   
   //--- inputbar shortcuts -----
   switch (keyval) {
      case GDK_Tab:
         run_completion(NEXT_GROUP);   return TRUE;
      case GDK_Up:
         run_completion(PREVIOUS);     return TRUE;
      case GDK_Down:
         run_completion(NEXT);         return TRUE;
   }

   return FALSE;
}

gboolean cb_wv_button_release(GtkWidget* widget, GdkEvent* event, gpointer data) {
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

gboolean cb_wv_console_message(WebKitWebView* wv, gchar* message, gint line, gchar* source, gpointer data) {

   if(!strcmp(message, "hintmode_off") || !strcmp(message, "insertmode_off"))
      change_mode(NORMAL);
   else if(!strcmp(message, "insertmode_on"))
      change_mode(INSERT);

   return TRUE;
}

GtkWidget* cb_wv_create_webview(WebKitWebView* wv, WebKitWebFrame* frame, gpointer data) {
   return create_tab(webkit_web_view_get_uri(wv), TRUE);
}

gboolean cb_wv_download_request(WebKitWebView* wv, WebKitDownload* download, gpointer data) {
   return download_content(download, webkit_download_get_suggested_filename(download));
}

gboolean cb_wv_hover_link(WebKitWebView* wv, gchar* title, gchar* link, gpointer data) {
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
      Argument arg={0};
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

   // stop here in case of PASS_THROUGH mode
   if(Client.Global.mode == PASS_THROUGH) return FALSE;

   // append only numbers and characters to buffer
   if(Client.Global.mode==NORMAL && isascii(keyval)) {
      if(!Client.Global.buffer)
         Client.Global.buffer = g_string_new("");

      Client.Global.buffer = g_string_append_c(Client.Global.buffer, keyval);
      gtk_label_set_text((GtkLabel*) Client.Statusbar.message, Client.Global.buffer->str);
   }

   // follow hints
   if(Client.Global.mode == HINTS) {
      Argument argument = {.data=event};
      sc_follow_link(&argument);
      return TRUE;
   }
   return FALSE;
}

gboolean cb_wv_mime_type(WebKitWebView* wv, WebKitWebFrame* frame, WebKitNetworkRequest* request, 
      gchar* mimetype, WebKitWebPolicyDecision* decision, gpointer data) {
   if(!webkit_web_view_can_show_mime_type(wv, mimetype)) {
      webkit_web_policy_decision_download(decision);
      return TRUE;
   }
   return FALSE;
}

gboolean cb_wv_navigation(WebKitWebView* wv, WebKitWebFrame* frame, WebKitNetworkRequest* request, 
      WebKitWebNavigationAction* action, WebKitWebPolicyDecision* decision, gpointer data) {

   switch(webkit_web_navigation_action_get_button(action)) {
      case 1: /* left mouse button */
         return FALSE;
      case 2: /* middle mouse button */
         create_tab(webkit_network_request_get_uri(request), TRUE);
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
      update_statusbar_info(gtk_notebook_get_current_page(Client.UI.webview));

   return TRUE;
}

gboolean cb_wv_notify_title(WebKitWebView* wv, GParamSpec* pspec, gpointer data) {
   if(webkit_web_view_get_title(wv)) 
      update_client(gtk_notebook_get_current_page(Client.UI.webview));

   return TRUE;
}

gboolean cb_wv_load_status(WebKitWebView* wv, GParamSpec *p, gpointer user_data){

      WebKitLoadStatus status = webkit_web_view_get_load_status(wv);

      if(status==WEBKIT_LOAD_COMMITTED){
         gchar *buffer = Client.Global.user_script->content;
         if(!buffer){
            notify(WARNING, "No script loaded");
            return FALSE;
         }
         run_script(buffer, NULL, NULL);

         // Flashblock stuff
         if(fb_enabled){
            FBFrames = NULL;
            WebKitDOMDocument *doc = webkit_web_view_get_dom_document(wv);
            WebKitDOMDOMWindow *win = webkit_dom_document_get_default_view(doc);
            webkit_dom_event_target_add_event_listener(WEBKIT_DOM_EVENT_TARGET(win), "beforeload", G_CALLBACK(cb_flashblock_before_load), TRUE, NULL);
            webkit_dom_event_target_add_event_listener(WEBKIT_DOM_EVENT_TARGET(win), "beforeunload", G_CALLBACK(cb_flashblock_before_unload), TRUE, NULL);
         }

      } else if(status==WEBKIT_LOAD_FINISHED){
      
         update_client(gtk_notebook_get_current_page(Client.UI.webview));

         gchar* uri = (gchar*) webkit_web_view_get_uri(wv);
         if(!uri) return FALSE;

         //--- Update history -----
         if(!private_browsing) {
            // we verify if the new_uri is already present in the list
            GList* l = g_list_find_custom(Client.Global.history, uri, (GCompareFunc)strcmp);
            if (l) {
               // new_uri is already present -> move it to the end of the list
               Client.Global.history = g_list_remove_link(Client.Global.history, l);
               Client.Global.history = g_list_concat(l, Client.Global.history);
            } else 
               Client.Global.history = g_list_prepend(Client.Global.history, g_strdup(uri));
         }
      }

      return TRUE;
}

gboolean cb_wv_new_window(WebKitWebView* wv, WebKitWebFrame* frame, WebKitNetworkRequest* request,
      WebKitWebNavigationAction* action, WebKitWebPolicyDecision* decision, gpointer data) {
   if(webkit_web_navigation_action_get_reason(action) == WEBKIT_WEB_NAVIGATION_REASON_LINK_CLICKED) {
      webkit_web_policy_decision_ignore(decision);
      //new_window(webkit_network_request_get_uri(request));
      create_tab(webkit_network_request_get_uri(request), TRUE);
      return TRUE;
   }
   return FALSE;
}

gboolean cb_wv_scrolled(GtkAdjustment* adjustment, gpointer data) {
   update_statusbar_info(gtk_notebook_get_current_page(Client.UI.webview));
   return TRUE;
}

