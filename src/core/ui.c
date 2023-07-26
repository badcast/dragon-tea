#include "ui_callbacks.h"

void tea_ui_init()
{
    // Инициализация окна
    GtkWidget *main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "Драконий Чай");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 500, 400);
    gtk_window_set_position(main_window, GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(main_window), FALSE);
    gtk_window_set_icon_name(GTK_WINDOW(main_window), "mail-message-new-symbolic");
    widgets.main_window = GTK_WINDOW(main_window);
    g_signal_connect(main_window, "destroy", G_CALLBACK(ui_on_close_window), NULL);

    GtkWidget *notebook = widgets.notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(main_window), notebook);

    // Инициализация виджетов

    widgets.widget_auth = create_auth_widget();
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widgets.widget_auth, gtk_label_new("Аутентификация"));
    widgets.widget_main = create_chat_widget();
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widgets.widget_main, gtk_label_new("Драконий чат"));
    gtk_notebook_append_page(
        GTK_NOTEBOOK(notebook), gtk_image_new_from_icon_name("help-about", GTK_ICON_SIZE_LARGE_TOOLBAR), gtk_label_new("О программе"));
    g_signal_connect(notebook, "switch-page", G_CALLBACK(ui_on_notebook_switch_page), NULL);

    // Show window
    gtk_widget_show_all(main_window);

    tea_ui_auth_lock(FALSE);
    tea_ui_reg_lock(FALSE);

    // disable chat as default
    tea_ui_chat_enable(FALSE);

    // Попытка автоматический войти, если настройки успешно загружены
    tea_ui_focus_tab(UI_TAB_AUTH);

    tea_try_login();
}

void tea_ui_focus_tab(enum UiTabs tabIndex)
{
    gtk_notebook_set_current_page(GTK_NOTEBOOK(widgets.notebook), (int) tabIndex);
}

void tea_ui_chat_enable(int value)
{
    tea_ui_chat_status_text("");
    gtk_entry_set_text(GTK_ENTRY(widgets.chat_tab.entry_message_set), "");

    if(value)
        tea_ui_chat_clear();
    else
        tea_ui_chat_set_text("Не выполнен вход на сервер");
    // interactable
    tea_ui_chat_interactable(value);
}

void tea_ui_chat_interactable(int value)
{
    gtk_widget_set_sensitive(widgets.chat_tab.entry_message_set, value);
    gtk_widget_set_sensitive(widgets.widget_main, value);
}

void tea_ui_chat_status_text(const char *status_text)
{
    gtk_label_set_text(GTK_LABEL(widgets.chat_tab.label_status), status_text);
}

void tea_ui_chat_clear()
{
    tea_ui_chat_set_text("");
}

void tea_ui_chat_set_text(const char *text)
{
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets.chat_tab.textview_chat));
    gtk_text_buffer_set_text(buffer, text, -1);
}

void tea_ui_chat_vscroll_max()
{
    // GtkAdjustment *vadj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(widgets.chat_tab.scrolled_window_chat));
    // gtk_adjustment_set_value(vadj, gtk_adjustment_get_upper(vadj));

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets.chat_tab.textview_chat));
    GtkTextIter endIter;
    gtk_text_buffer_get_end_iter(buffer, &endIter);
    GtkTextMark *end_mark = gtk_text_buffer_create_mark(buffer, "end", &endIter, FALSE);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(widgets.chat_tab.textview_chat), end_mark, 0.0, TRUE, 0, 1.0);
    gtk_text_buffer_delete_mark(buffer, end_mark);

}

void tea_ui_chat_push_block(struct tea_message_id *message)
{
    /*
     *  FORMAT VIEW for Message Block
     *   bad@(21:16): Hello
     */
    static const char format_message_block[] = "\n%s@(%s): %s\n";
    static char text_buffer[TEA_MAXLEN_MESSAGE + TEA_MAXLEN_USERNAME + 32];
    GDateTime *_gdatetime = g_date_time_new_from_unix_local(message->time_received);
    gchar *date_time = g_date_time_format(_gdatetime, "%H:%M");
    snprintf(text_buffer, sizeof(text_buffer), format_message_block, message->sent_user_name, date_time, message->message_text);
    g_free(_gdatetime);
    g_free(date_time);

    int len = strlen(text_buffer);
    len = len < sizeof(text_buffer) ? len : sizeof(text_buffer);
    text_buffer[len] = '\0';
    tea_ui_chat_push_text_raw(text_buffer, len);
}

void tea_ui_chat_push_text_raw(const char *text, int len)
{
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets.chat_tab.textview_chat));
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
   // gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, len, "red_foreground");
    gtk_text_buffer_insert(buffer, &iter, text, len);
}
