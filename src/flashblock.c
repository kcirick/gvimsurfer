/*
 * src/flashblock.c
 */

#include <stdlib.h>

#include <gtk/gtk.h>
#include <webkit/webkit.h>

#include "include/gvimsurfer.h"
#include "include/utilities.h"
#include "include/flashblock.h"

//--- Function implementations -----
void flashblock_create_click_element(WebKitDOMElement *element){

   WebKitDOMNode *parent = webkit_dom_node_get_parent_node(WEBKIT_DOM_NODE(element));
   WebKitDOMElement *div;

   if(!parent) return;

   WebKitDOMDocument *doc = webkit_dom_node_get_owner_document(WEBKIT_DOM_NODE(element));
   WebKitDOMDOMWindow *win = webkit_dom_document_get_default_view(doc);

   WebKitDOMCSSStyleDeclaration *style = webkit_dom_dom_window_get_computed_style(win, element, "");
   gchar *width = webkit_dom_css_style_declaration_get_property_value(style, "width");
   gchar *height = webkit_dom_css_style_declaration_get_property_value(style, "height");
   gchar *top = webkit_dom_css_style_declaration_get_property_value(style, "top");
   gchar *left = webkit_dom_css_style_declaration_get_property_value(style, "left");
   gchar *position = webkit_dom_css_style_declaration_get_property_value(style, "position");
   
   gint w, h;
   if (*width == '\0' || (sscanf(width, "%dpx", &w) == 1 && w<72))      w = 72;
   if (*height == '\0' || (sscanf(height, "%dpx", &h) == 1 && h<24))    h = 24;

   gboolean found=FALSE;
   FlashBlockFrame* frame;
   for(GList* list=FBFrames; list; list=list->next){
      frame = (FlashBlockFrame*)list->data;
      if(frame->element == element) found=TRUE;
   }

   if(!found){
      div = webkit_dom_document_create_element(doc, "div", NULL);

      frame = malloc(sizeof(FlashBlockFrame));
      frame->div = div;
      frame->element = element;
      frame->isVisible = FALSE;
      FBFrames = g_list_prepend(FBFrames, frame); 
   } else
      div = frame->div;

   webkit_dom_html_element_set_inner_html(WEBKIT_DOM_HTML_ELEMENT(div), 
         "<div style='display:table-cell;vertical-align:middle;text-align:center;color:#fff;background:#000;border:1px solid #666;font:11px monospace bold'>click to enable flash</div>", 
         NULL);

    gchar *new_style = g_strdup_printf("position:%s;width:%dpx;height:%dpx;top:%s;left:%s;display:table;z-index:37000;", position, w, h, top, left);
    webkit_dom_element_set_attribute(div, "style", new_style, NULL);
    g_free(new_style);

    webkit_dom_element_set_attribute(div, "onclick", "return", NULL);

    g_object_set_data((gpointer)div, "flashblock-plugin-element", element);

    webkit_dom_node_remove_child(parent, WEBKIT_DOM_NODE(element), NULL);
    webkit_dom_node_append_child(parent, WEBKIT_DOM_NODE(div), NULL);

    webkit_dom_event_target_add_event_listener(WEBKIT_DOM_EVENT_TARGET(div), "click", G_CALLBACK(cb_flashblock_onclick), TRUE, NULL);
    g_object_unref(style);
    g_object_unref(parent);
}

//--- Callback functions -----
void cb_flashblock_onclick(WebKitDOMElement *element, WebKitDOMEvent *event, gpointer user_data) {
   
   for(GList* list=FBFrames; list; list=list->next){
      FlashBlockFrame* frame = (FlashBlockFrame*)list->data;
      if(frame->div == element) frame->isVisible=TRUE;
   }
   
   WebKitDOMElement *e = g_object_get_data((gpointer)element, "flashblock-plugin-element");
   WebKitDOMNode *parent = webkit_dom_node_get_parent_node(WEBKIT_DOM_NODE(element));
   WebKitDOMNode *child = webkit_dom_node_get_first_child(WEBKIT_DOM_NODE(element));
   webkit_dom_node_remove_child(WEBKIT_DOM_NODE(parent), child, NULL);
   webkit_dom_node_remove_child(parent, WEBKIT_DOM_NODE(element), NULL);
   webkit_dom_node_append_child(parent, WEBKIT_DOM_NODE(e), NULL);
   g_object_unref(parent);
   
}

gboolean cb_flashblock_before_load(WebKitDOMDOMWindow *win, WebKitDOMEvent *event, gpointer user_data){
   
   WebKitDOMElement *element = (void*)webkit_dom_event_get_src_element(event);
   gchar *tagname = webkit_dom_element_get_tag_name(element);
   gchar *type = webkit_dom_element_get_attribute(element, "type");

   gboolean visible = FALSE;
   for(GList* list=FBFrames; list; list=list->next){
      FlashBlockFrame* frame = (FlashBlockFrame*)list->data;
      if(frame->element == element) visible = frame->isVisible;
   }

   if ( (!g_strcmp0(type, "application/x-shockwave-flash") 
            && (! g_ascii_strcasecmp(tagname, "object") || ! g_ascii_strcasecmp(tagname, "embed")) )
            && !visible ) 
   {
      webkit_dom_event_prevent_default(event);
      webkit_dom_event_stop_propagation(event);

      flashblock_create_click_element(element);
   }
   g_object_unref(element);
   g_free(tagname);
   g_free(type);
    
   return TRUE;
}

gboolean cb_flashblock_before_unload(WebKitDOMDOMWindow *win, WebKitDOMEvent *event, gpointer user_data){
   
   if(FBFrames){
      for(GList* list = FBFrames; list; list = g_list_next(list))
         g_free(list->data);

      g_list_free(FBFrames);
   }

   return TRUE;
}

