/*
 * include/callbacks.h
 */

#ifndef CALLBACKS_H
#define CALLBACKS_H

gboolean    cb_blank();
gboolean    cb_destroy(GtkWidget*, gpointer);
gboolean    cb_download_progress(WebKitDownload*, GParamSpec*);

gboolean    cb_button_add_tab(GtkButton*, GtkNotebook*);
gboolean    cb_button_close_tab(GtkButton*, GtkNotebook*);

gboolean    cb_notebook_switch_page(GtkNotebook*, gpointer, guint, gpointer);
gboolean    cb_notebook_switch_page_after(GtkNotebook*, gpointer, guint, gpointer);

gboolean    cb_inputbar_activate(GtkEntry*, gpointer);
gboolean    cb_inputbar_kb_pressed(GtkWidget*, GdkEventKey*, gpointer);

gboolean    cb_wv_button_release(GtkWidget*, GdkEvent*, gpointer);
gboolean    cb_wv_console_message(WebKitWebView*, gchar*, gint, gchar*, gpointer);
GtkWidget*  cb_wv_create_webview(WebKitWebView*, WebKitWebFrame*, gpointer);
gboolean    cb_wv_download_request(WebKitWebView*, WebKitDownload*, gpointer);
gboolean    cb_wv_hover_link(WebKitWebView*, gchar*, gchar*, gpointer);
gboolean    cb_wv_kb_pressed(WebKitWebView*, GdkEventKey*); 
gboolean    cb_tab_kb_pressed(WebKitWebView*, GdkEventKey*); 
gboolean    cb_wv_mime_type(WebKitWebView*, WebKitWebFrame*, WebKitNetworkRequest*, 
      gchar*, WebKitWebPolicyDecision*, gpointer);
gboolean    cb_wv_navigation(WebKitWebView*, WebKitWebFrame*, WebKitNetworkRequest*, 
      WebKitWebNavigationAction*, WebKitWebPolicyDecision*, gpointer);
gboolean    cb_wv_notify_progress(WebKitWebView*, GParamSpec*, gpointer);
gboolean    cb_wv_notify_title(WebKitWebView*, GParamSpec*, gpointer);
gboolean    cb_wv_new_window(WebKitWebView*, WebKitWebFrame*, WebKitNetworkRequest*, 
      WebKitWebNavigationAction*, WebKitWebPolicyDecision*, gpointer);
gboolean    cb_wv_scrolled(GtkAdjustment*, gpointer);
gboolean    cb_wv_load_status(WebKitWebView*, GParamSpec*, gpointer);

#endif
