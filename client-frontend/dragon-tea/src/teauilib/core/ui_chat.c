#include "tea.h"
#include "ui_callbacks.h"

void on_entry_edit_message(GtkEntry *entry, gpointer data)
{
    gtk_widget_set_sensitive(widgets.chat_tab.button_send, gtk_entry_get_text_length(entry) > 0);
}

void on_button_logout(GtkButton *button, gpointer data)
{
    tea_logout();
}

GtkWidget *create_chat_widget()
{
    GtkWidget *place_widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *top_horz_box = widgets.chat_tab.top_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *vertical_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *horizontal_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *status_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_box_pack_start(
        GTK_BOX(top_horz_box),
        widgets.chat_tab.top_button_logout = gtk_button_new_from_icon_name("system-log-out", GTK_ICON_SIZE_BUTTON),
        FALSE,
        FALSE,
        0);
    g_signal_connect(widgets.chat_tab.top_button_logout, "clicked", G_CALLBACK(on_button_logout), NULL);

    gtk_box_pack_start(GTK_BOX(top_horz_box), widgets.chat_tab.top_label_user_state = gtk_label_new(NULL), FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(place_widget), top_horz_box, FALSE, TRUE, 0);

    GtkWidget *scrolled_text = widgets.chat_tab.scrolled_window_chat = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_text), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_text), GTK_SHADOW_IN);
    gtk_scrolled_window_set_kinetic_scrolling(GTK_SCROLLED_WINDOW(scrolled_text), TRUE);

    GtkWidget *text_view = widgets.chat_tab.textview_chat = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_CHAR);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);

    gtk_container_add(GTK_CONTAINER(scrolled_text), text_view);

    gtk_box_pack_start(GTK_BOX(vertical_box), scrolled_text, TRUE, TRUE, 10);

    GtkWidget *entry = widgets.chat_tab.entry_message_set = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(horizontal_box), entry, TRUE, TRUE, 0);
    gtk_entry_set_max_length(GTK_ENTRY(entry), TEA_MAXLEN_MESSAGE - 1);
    g_signal_connect(entry, "activate", G_CALLBACK(on_chat_send_button), entry);
    g_signal_connect(entry, "changed", G_CALLBACK(on_entry_edit_message), NULL);

    GtkWidget *button = widgets.chat_tab.button_send = gtk_button_new();
    gtk_widget_set_sensitive(button, FALSE);
    gtk_button_set_image(GTK_BUTTON(button), gtk_image_new_from_icon_name("mail-send-symbolic", GTK_ICON_SIZE_BUTTON));

    gtk_box_pack_start(GTK_BOX(horizontal_box), button, FALSE, FALSE, 0);

    g_signal_connect(button, "clicked", G_CALLBACK(on_chat_send_button), entry);

    gtk_box_pack_end(GTK_BOX(place_widget), vertical_box, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(vertical_box), horizontal_box, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(vertical_box), status_box, FALSE, FALSE, 0);

    gtk_widget_show(place_widget);
    return place_widget;
}
