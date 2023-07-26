#include "ui_callbacks.h"

struct tea_app_widgets widgets;

void ui_on_close_window(GtkWidget *window, gpointer data)
{
    gtk_main_quit();
    tea_save();
}

void ui_on_notebook_switch_page(GtkNotebook *notebook, GtkWidget *page, guint page_num, gpointer user_data)
{
    if(page_num == gtk_notebook_get_n_pages(notebook) - 1)
    {
        show_about_dialog();
        tea_ui_focus_tab(UI_TAB_CHAT);
    }
}

// Функция обработки нажатия кнопки отправить
void on_chat_send_button(GtkWidget *widget, gpointer data)
{
    GtkEntry *entry = GTK_ENTRY(data);
    const gchar *text = gtk_entry_get_text(entry);
    int len = gtk_entry_get_text_length(entry);

    // Отправка доступна, только когда она не пуста
    if(len < 1)
        return;

    // disable Chat widget
    gtk_widget_set_sensitive(widgets.widget_main, FALSE);
    tea_ui_chat_status_text("Отправка...");

    g_timeout_add(100, G_CALLBACK(on_chat_sending_async), text);
}

gboolean on_chat_sending_async(const gchar *text)
{
    struct tea_message_id message;
    struct tea_message_send_result output_result;

    // enable Chat widget
    gtk_widget_set_sensitive(widgets.widget_main, TRUE);

    int len = strlen(text);
    // send message to
    net_api_write_message(&app_settings.id_info, -1, text, len, &output_result);
    if(output_result.status == TEA_STATUS_OK)
    {
        /*
            // set message id as offload
            message.msg_id = output_result.msg_id;
            // time received on server
            message.time_received = output_result.time_received;
            // time saved on DB server
            message.time_saved = output_result.time_saved;
            // set sender user id
            message.sent_user_id = app_settings.id_info.user_id;
            // copy sender user name
            strncpy(message.sent_user_name, app_settings.id_info.user_nickname, sizeof(message.sent_user_name));
            // copy sender user message
            strncpy(message.message_text, text, sizeof(message.message_text));
        */
        // set VScroll to max
        tea_ui_chat_vscroll_max();

        tea_ui_chat_status_text("Сообщение отправлено.");
        gtk_entry_set_text(GTK_ENTRY(widgets.chat_tab.entry_message_set), "");
        gtk_widget_grab_focus(widgets.chat_tab.entry_message_set);
    }
    else
    {
        const char *msg = "Ошибка отправки.";
        tea_ui_chat_status_text(msg);
        error(msg);
    }
    return FALSE;
}

gboolean last_synched = 0;
gboolean on_chat_message_handler_async(gpointer)
{
    struct tea_message_read_result result;
    const char *error_string = NULL;
    // Последнее сообщение на локальной машине (когда на сервере оно устарело)
    int last_local_msg_id;

    if(app_settings.local_msg_db->len == 0)
    {
        last_local_msg_id = 0;
    }
    else
    {
        last_local_msg_id = (&g_array_index(app_settings.local_msg_db, struct tea_message_id, app_settings.local_msg_db->len - 1))->msg_id;
    }

    // Читаем сообщение других пользователей и поддерживаем коммуникацию
    net_api_read_messages(&app_settings.id_info, -1, last_local_msg_id, 255, &result);
    if(result.message_length > 0)
    {
        tea_ui_chat_status_text("Синхронизация...");
        last_synched = 0;
    }
    switch(result.status)
    {
        // Это серверная ошибка, значит можно продолжать
        case TEA_STATUS_INTERNAL_SERVER_ERROR:
        // Успех при чтений данных
        case TEA_STATUS_OK:
            if(last_synched == 1)
                break;

            if(result.message_length < 1)
            {
                tea_ui_chat_status_text("Вы в сети.");
                tea_ui_chat_interactable(TRUE);
                gtk_widget_grab_focus(widgets.chat_tab.entry_message_set);
                last_synched = 1;
                break;
            }

            tea_ui_chat_interactable(FALSE);

            // Добавление сообшщений из загруженных в локальный
            g_array_append_vals(app_settings.local_msg_db, result.messages->data, result.messages->len);

            // Отображение собщений
            for(int x = 0; x < result.messages->len; ++x)
            {
                tea_ui_chat_push_block(&g_array_index(result.messages, struct tea_message_id, x));
            }

            // Освобождение старого массива
            g_array_free(result.messages, TRUE);
            // VScroll to max
            tea_ui_chat_vscroll_max();
            last_synched = 0;
            break;
            // Аккаунт удален на стороне, серевера, значит выводи предупреждение и выходим с сервера
        case TEA_STATUS_ID_NO_EXIST:
            if(error_string == NULL)
                error_string = "Ваш аккаунт был удален. Вам придеться перезайти.";
        // Сетевая ошибка, отменяем вход и выходим от прослушивания
        case TEA_STATUS_NETWORK_ERROR:
            if(error_string == NULL)
                error_string = "Сетевая ошибка. Ваше соединение было сброшено.";
        default:

            if(error_string)
                tea_ui_chat_status_text(error_string);

            tea_logout();

            // wait
            sleep(2);

            if(error_string)
                error(error_string);
            // уничтожение Taimer ID
            break;
    }

    return TRUE; // EVERYTHING
}
