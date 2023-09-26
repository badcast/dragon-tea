#include "tea.h"
#include "ui_callbacks.h"

void on_entry_edit_message(GtkEntry *entry, gpointer data)
{
    // Кнопка отправить доступна, когда длина текста сообщений больше 0
    gtk_widget_set_sensitive(widgets.chat_tab.button_send, gtk_entry_get_text_length(entry) > 0);
}

void on_button_logout(GtkButton *button, gpointer data)
{
    tea_logout();
}

char *expower_bytesISO(char *str, int len, uintmax_t bytes)
{
    static const int iso = 1024;
    static const char data_kind[5][6] = {"bytes", "KiB", "MiB", "GiB", "TiB"};

    /*
    ASSIGN: bytes < 1024 = bytes
           bytes < 1024^2 = KiB
           bytes < 1024^3 = MiB
           bytes < 1024^4 = GiB
           bytes < 1024^5 = TiB
           ~ BigData
    */
    int power = 0;

    while(bytes >= iso)
    {
        bytes /= iso;
        ++power;
    }
    snprintf(str, len, "%d %s", bytes, data_kind[MIN(power, 5)]);
    return str;
}

size_t last_sent = 0, last_recv = 0;
gboolean on_refresh_traffic(gpointer)
{
    char buffer[78], bufA[24];
    buffer[0] = bufA[0] = 0;

    strncat(buffer, expower_bytesISO(bufA, sizeof(bufA), net_stats.transmitted_bytes-last_sent), sizeof(buffer));
    strncat(buffer, "/s / ", sizeof(buffer));
    strncat(buffer, expower_bytesISO(bufA, sizeof(bufA), net_stats.received_bytes-last_recv), sizeof(buffer));
    strncat(buffer, "/s", sizeof(buffer));

    strncat(buffer, " | ", sizeof(buffer));
    strncat(buffer, expower_bytesISO(bufA, sizeof(bufA), net_stats.transmitted_bytes), sizeof(buffer));
    strncat(buffer, " / ", sizeof(buffer));
    strncat(buffer, expower_bytesISO(bufA, sizeof(bufA), net_stats.received_bytes), sizeof(buffer));

    last_sent = net_stats.transmitted_bytes;
    last_recv = net_stats.received_bytes;

    gtk_label_set_text(GTK_LABEL(widgets.chat_tab.label_traffics), buffer);
    return 1;
}

GtkTextTag *red_tag;
GtkWidget *create_chat_widget()
{
    // Создание верхнего уровня окна
    GtkWidget *place_widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    // Создание верхнего статуса
    GtkWidget *top_horz_box = widgets.chat_tab.top_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    // Создание вертикального контейнера (GtkBox)
    GtkWidget *vertical_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    // Создание горизонтального контейнера (GtkBox)
    GtkWidget *horizontal_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    // Создание контейнер статуса
    GtkWidget *status_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    // Создание верхнего поля отображение информаций
    gtk_box_pack_start(
        GTK_BOX(top_horz_box),
        widgets.chat_tab.top_button_logout = gtk_button_new_from_icon_name("system-log-out", GTK_ICON_SIZE_BUTTON),
        FALSE,
        FALSE,
        0);
    g_signal_connect(widgets.chat_tab.top_button_logout, "clicked", G_CALLBACK(on_button_logout), NULL);

    gtk_box_pack_start(GTK_BOX(top_horz_box), widgets.chat_tab.top_label_user_state = gtk_label_new("Hello"), FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(place_widget), top_horz_box, FALSE, TRUE, 0);

    // Создание текстового поля (GtkTextView) и установка его в вертикальный контейнер
    GtkWidget *scrolled_text = widgets.chat_tab.scrolled_window_chat = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_text), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

    GtkWidget *text_view = widgets.chat_tab.textview_chat = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_CHAR);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_widget_set_margin_top(text_view, 10);
    gtk_widget_set_margin_start(text_view, 10);
    gtk_widget_set_margin_end(text_view, 10);
    gtk_widget_set_margin_bottom(text_view, 10);

    // init tags
    GtkTextBuffer *textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    red_tag = gtk_text_buffer_create_tag(textbuffer, "red_foreground", "foreground", "red", NULL);

    gtk_container_add(GTK_CONTAINER(scrolled_text), text_view);
    // Запрещаем GtkTextView изменять размеры
    gtk_widget_set_size_request(text_view, 200, 100);

    gtk_box_pack_start(GTK_BOX(vertical_box), scrolled_text, TRUE, TRUE, 0);

    // Создание поля ввода текста (GtkEntry)
    GtkWidget *entry = widgets.chat_tab.entry_message_set = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(horizontal_box), entry, TRUE, TRUE, 0);
    gtk_entry_set_max_length(GTK_ENTRY(entry), TEA_MAXLEN_MESSAGE - 1);
    g_signal_connect(entry, "activate", G_CALLBACK(on_chat_send_button), entry);
    g_signal_connect(entry, "changed", G_CALLBACK(on_entry_edit_message), NULL);

    // Создание кнопки для отправки
    GtkWidget *button = widgets.chat_tab.button_send = gtk_button_new();
    gtk_widget_set_sensitive(button, FALSE);
    gtk_button_set_image(GTK_BUTTON(button), gtk_image_new_from_icon_name("mail-send-symbolic", GTK_ICON_SIZE_BUTTON));

    gtk_box_pack_start(GTK_BOX(horizontal_box), button, FALSE, FALSE, 0);

    // Подключение обработчика сигнала "clicked" (нажатие кнопки)
    g_signal_connect(button, "clicked", G_CALLBACK(on_chat_send_button), entry);

    // Статус
    GtkWidget *label = widgets.chat_tab.label_status = gtk_label_new(NULL);
    gtk_box_pack_start(GTK_BOX(status_box), label, TRUE, TRUE, 0);
    label = widgets.chat_tab.label_traffics = gtk_label_new(NULL);
    gtk_box_pack_end(GTK_BOX(status_box), label, FALSE, TRUE, 25);
    gtk_box_pack_end(GTK_BOX(status_box), gtk_image_new_from_icon_name("mail-send-receive", GTK_ICON_SIZE_SMALL_TOOLBAR), FALSE, TRUE, 0);

    g_timeout_add(1000, on_refresh_traffic, NULL);

    // Добавление вертикального контейнера в окно
    gtk_box_pack_end(GTK_BOX(place_widget), vertical_box, TRUE, TRUE, 0);

    // Добавление горизонтального контейнера в вертикальный контейнер
    gtk_box_pack_start(GTK_BOX(vertical_box), horizontal_box, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(vertical_box), status_box, FALSE, FALSE, 0);

    gtk_widget_show(place_widget);
    return place_widget;
}
