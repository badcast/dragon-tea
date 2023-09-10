#ifndef UI_CALLBACKS_H
#define UI_CALLBACKS_H

#include "tea.h"

void tea_on_authenticate(struct tea_id_info *user_info);

void tea_on_logouted();

void ui_on_close_window(GtkWidget *window, gpointer data);

void ui_on_notebook_switch_page(GtkNotebook *notebook, GtkWidget *page, guint page_num, gpointer user_data);

void on_chat_send_button(GtkWidget *widget, gpointer data);

gboolean on_chat_sending_async(const gchar *text);

gboolean on_chat_message_handler_async(gpointer);

#endif
