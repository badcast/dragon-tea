#include "ui_callbacks.h"

struct tea_app_widgets widgets;

GThread *thread_sending = NULL;
GThread *thread_reply_msg = NULL;

guint check_chance_logouting = CHANGE_TO_LOGOUT;

struct tea_message_send_result last_sended_result;
struct tea_message_read_result last_read_result;

gpointer async_send(const char *text)
{
    // thread
    struct tea_message_id message;

    int len = strlen(text);
    // send message to
    int net = net_api_write_message(&app_settings.id_info, -1, text, len, &last_sended_result);
    // set work complete
    thread_sending = NULL;

    return NULL;
}

gpointer async_reply(gpointer)
{
    // thread

    // Последнее сообщение на локальной машине (когда на сервере она устарело)
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
    int net = net_api_read_messages(&app_settings.id_info, -1, last_local_msg_id, 16, &last_read_result);

    thread_reply_msg = NULL;

    return NULL;
}

gboolean on_chat_sending_async(const gchar *text)
{
    int worked = thread_sending == NULL;
    if(worked)
    {
        if(last_sended_result.msg_id && last_sended_result.status == TEA_STATUS_OK)
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
            char buffer[255];
            const char format[] = "%H:%M";
            gchar *sended_time, *received_time;
            GDateTime *datet = g_date_time_new_from_unix_local(last_sended_result.time_saved);
            GDateTime *date2 = g_date_time_new_from_unix_local(last_sended_result.time_received);
            sended_time = g_date_time_format(datet, format);
            received_time = g_date_time_format(date2, format);
            snprintf(buffer, sizeof(buffer), "Сообщение отправлено. Время отправки %s, получено %s", sended_time, received_time);
            g_free(sended_time);
            g_free(received_time);
            g_date_time_unref(datet);
            g_date_time_unref(date2);

            tea_ui_chat_status_text(buffer);
            gtk_entry_set_text(GTK_ENTRY(widgets.chat_tab.entry_message_set), "");
            tea_ui_chat_interactable(TRUE);
            tea_ui_chat_vscroll_max();

            // Require chat sync
            tea_ui_chat_sync();
        }
        else
        {
            const char *msg = "Ошибка отправки.";
            tea_ui_chat_status_text(msg);
            error(msg);
        }
    }
    return worked == FALSE;
}

gboolean on_chat_message_handler_async(gpointer)
{
    const char *error_string = NULL;
    int worked = thread_reply_msg == NULL;

    if(worked)
    {
        char buffer[64];
        if(last_read_result.message_length > 0)
        {
            snprintf(buffer, sizeof(buffer), "Синхронизация %ld сообщений", last_read_result.message_length);
            tea_ui_chat_status_text(buffer);
            widgets.chat_tab.chat_synched = FALSE;
        }
        switch(last_read_result.status)
        {
            // Это серверная ошибка, значит можно продолжать
            case TEA_STATUS_INTERNAL_SERVER_ERROR:
            // Успех при чтений данных
            case TEA_STATUS_OK:

                check_chance_logouting = CHANGE_TO_LOGOUT;

                if(widgets.chat_tab.chat_synched == TRUE)
                {
                    tea_ui_chat_status_text("Вы в сети.");
                    break;
                }

                if(last_read_result.message_length < 1)
                {
                    gtk_widget_grab_focus(widgets.chat_tab.entry_message_set);
                    widgets.chat_tab.chat_synched = TRUE;
                    break;
                }

                // Добавление сообщений из загруженных в локальные
                g_array_append_vals(app_settings.local_msg_db, last_read_result.messages->data, last_read_result.messages->len);

                // Отображение собщений
                for(int x = 0; x < last_read_result.messages->len; ++x)
                {
                    tea_ui_chat_push_block(&g_array_index(last_read_result.messages, struct tea_message_id, x));
                }

                // Освобождение старого массива
                g_array_free(last_read_result.messages, TRUE);
                last_read_result.messages = NULL;

                // VScroll to max
                tea_ui_chat_vscroll_max();
                break;
                // Аккаунт удален на стороне серевера, выводим предупреждение и выходим с сервера
            case TEA_STATUS_ID_NO_EXIST:
                if(error_string == NULL)
                {
                    error_string = "Ваш аккаунт был удален.";
                    check_chance_logouting = 0;
                }
            // Сетевая ошибка, отменяем вход и выходим от прослушивания
            case TEA_STATUS_NETWORK_ERROR:

                if(error_string == NULL)
                    error_string = "Сетевая ошибка. Ваше соединение было сброшено.";
            default:

                if(error_string)
                    tea_ui_chat_status_text(error_string);

                // Account has logouting in chances = 0
                if(--check_chance_logouting <= 0)
                {
                    tea_logout();
                    check_chance_logouting = CHANGE_TO_LOGOUT; // revert to default
                }

                // wait
                sleep(1);

                if(error_string)
                    error(error_string);
                // уничтожение Taimer ID
                break;
        }
        // Resend
        thread_reply_msg = g_thread_new(NULL, G_CALLBACK(async_reply), NULL);
    }
    return TRUE; // EVERYTHING
}

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

    if(len < 1 && thread_sending != NULL)
        return;

    // disable Chat widget
    gtk_widget_set_sensitive(widgets.widget_main, FALSE);
    tea_ui_chat_status_text("Отправка...");

    thread_sending = g_thread_new(NULL, G_CALLBACK(async_send), text);

    g_timeout_add(INTERVAL_SEND, G_CALLBACK(on_chat_sending_async), text);
}
