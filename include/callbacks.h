/*
 * include/callbacks.h
 */

#ifndef CALLBACKS_H
#define CALLBACKS_H

gboolean    cb_blank();
gboolean    cb_destroy(GtkWidget*, gpointer);
gboolean    cb_download_progress(WebKitDownload*, GParamSpec*);

gboolean    cb_inputbar_activate(GtkEntry*, gpointer);
gboolean    cb_inputbar_kb_pressed(GtkWidget*, GdkEventKey*, gpointer);

gboolean    cb_wv_button_release_event(GtkWidget*, GdkEvent*, gpointer);
gboolean    cb_wv_console_message(WebKitWebView*, char*, int, char*, gpointer);
GtkWidget*  cb_wv_create_webview(WebKitWebView*, WebKitWebFrame*, gpointer);
gboolean    cb_wv_download_request(WebKitWebView*, WebKitDownload*, gpointer);
gboolean    cb_wv_event(GtkWidget *widget, GdkEvent *event, gpointer user_data); 
gboolean    cb_wv_hover_link(WebKitWebView*, char*, char*, gpointer);
gboolean    cb_wv_kb_pressed(WebKitWebView*, GdkEventKey*); 
gboolean    cb_tab_kb_pressed(WebKitWebView*, GdkEventKey*); 
gboolean    cb_wv_mimetype_policy_decision(WebKitWebView*, WebKitWebFrame*, WebKitNetworkRequest*, char*, WebKitWebPolicyDecision*, gpointer);
gboolean    cb_wv_nav_policy_decision(WebKitWebView*, WebKitWebFrame*, WebKitNetworkRequest*, WebKitWebNavigationAction*, WebKitWebPolicyDecision*, gpointer);
gboolean    cb_wv_notify_progress(WebKitWebView*, GParamSpec*, gpointer);
gboolean    cb_wv_notify_title(WebKitWebView*, GParamSpec*, gpointer);
gboolean    cb_wv_window_object_cleared(WebKitWebView*, WebKitWebFrame*, gpointer, gpointer, gpointer);
gboolean    cb_wv_window_policy_decision(WebKitWebView*, WebKitWebFrame*, WebKitNetworkRequest*, WebKitWebNavigationAction*, WebKitWebPolicyDecision*, gpointer);
gboolean    cb_wv_scrolled(GtkAdjustment*, gpointer);

#endif
