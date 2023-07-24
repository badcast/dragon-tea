#include "tea.h"

// Функция обработки нажатия кнопки отправить
void on_send_button_clicked(GtkWidget *widget, gpointer data)
{
    GtkEntry *entry = GTK_ENTRY(data);
    const gchar *text = gtk_entry_get_text(entry);
    g_print("Текст: %s\n", text);
}

GtkWidget *create_main_widget()
{
    char buf[TEA_MAXLEN_USERNAME + 128];

    // Создание верхнего уровня окна
    GtkWidget *place_widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    // Создание вертикального контейнера (GtkBox)
    GtkWidget *vertical_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    // Создание текстового поля (GtkTextView) и установка его в вертикальный контейнер
    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    // set text
    GtkTextBuffer *text_view_buff = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    //  char a[32];
    //   char b[32];

    //    struct tm *time_info = localtime(&app_tea_settings.user.creation_date);
    //    strftime(a, sizeof(a), "%d.%m.%Y %H:%M:%S", time_info);
    //    time_info = localtime(&app_tea_settings.user.last_login);
    //    strftime(b, sizeof(b), "%d.%m.%Y %H:%M:%S", time_info);
    //    sprintf(buf, "Вошли как - %s\nДата создания: %s\nПоследний вход: %s", app_tea_settings.user.user_nickname, a, b);
    sprintf(buf, "Не выполнен вход на сервер");
    gtk_text_buffer_set_text(text_view_buff, buf, -1);
    gtk_box_pack_start(GTK_BOX(vertical_box), text_view, TRUE, TRUE, 2);

    // Создание горизонтального контейнера (GtkBox)
    GtkWidget *horizontal_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    // Создание поля ввода текста (GtkEntry)
    GtkWidget *entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(horizontal_box), entry, TRUE, TRUE, 0);
    gtk_entry_set_max_length(GTK_ENTRY(entry), TEA_MAXLEN_MESSAGE);

    // Создание кнопки для отправки
    GtkWidget *button = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(button), gtk_image_new_from_icon_name("mail-send-symbolic", GTK_ICON_SIZE_BUTTON));
    gtk_box_pack_start(GTK_BOX(horizontal_box), button, FALSE, FALSE, 0);

    // Подключение обработчика сигнала "clicked" (нажатие кнопки)
    g_signal_connect(button, "clicked", G_CALLBACK(on_send_button_clicked), entry);

    // Добавление горизонтального контейнера в вертикальный контейнер
    gtk_box_pack_start(GTK_BOX(vertical_box), horizontal_box, FALSE, FALSE, 0);

    // Добавление вертикального контейнера в окно
    gtk_box_pack_start(GTK_BOX(place_widget), vertical_box, TRUE, TRUE, 0);
    gtk_widget_show(place_widget);
    return place_widget;
}
