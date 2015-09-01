/*
 * include/flashblock.h
 */

#ifndef FLASHBLOCK_H
#define FLASHBLOCK_H

typedef struct{
   WebKitDOMElement* element;
   WebKitDOMElement* div;
   gboolean          isVisible;
} FlashBlockFrame;

GList* FBFrames;

void     flashblock_create_click_element(WebKitDOMElement*);

void     cb_flashblock_onclick(WebKitDOMElement*, WebKitDOMEvent*, gpointer); 
gboolean cb_flashblock_before_load(WebKitDOMDOMWindow*, WebKitDOMEvent*, gpointer);
gboolean cb_flashblock_before_unload(WebKitDOMDOMWindow*, WebKitDOMEvent*, gpointer);


#endif
