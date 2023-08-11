#include "ui_callbacks.h"

struct tea_app_widgets widgets;

GThread *thread_sending = NULL;
GThread *thread_reply_msg = NULL;

guint check_chance_logouting = CHANCE_TO_LOGOUT;

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
            // Clear upload text
            gtk_entry_set_text(GTK_ENTRY(widgets.chat_tab.entry_message_set), "");
            // VScroll to max
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
        tea_ui_chat_interactable(TRUE);
    }
    return worked == FALSE;
}

gboolean on_chat_message_handler_async(gpointer)
{
    char buffer[128];
    const char *error_string = NULL;

    if(thread_reply_msg == NULL)
    {
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
                guint last_chance_state = check_chance_logouting;
                // Reset logouting chances
                check_chance_logouting = CHANCE_TO_LOGOUT;

                if(widgets.chat_tab.chat_synched == TRUE)
                {

                    if(last_chance_state != CHANCE_TO_LOGOUT)
                        strcpy(buffer, "Ваша сеть восстановлена. ");
                    else
                        buffer[0] = 0;

                    strcat(buffer, "Вы в сети.");
                    tea_ui_chat_status_text(buffer);
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
                // Аккаунт удален на стороне сервера, выводим предупреждение и выходим с сервера
            case TEA_STATUS_ID_NO_EXIST:
                if(error_string == NULL)
                {
                    error_string = "Ваш аккаунт был удален.";
                    check_chance_logouting = 0; // logout
                }
            // Сетевая ошибка, отменяем вход и выходим от прослушивания
            case TEA_STATUS_NETWORK_ERROR:
                if(error_string == NULL)
                {
                    snprintf(buffer, sizeof(buffer), "Сетевая ошибка. Ваше соединение было сброшено (%d осталось)", check_chance_logouting);
                    error_string = buffer;
                    --check_chance_logouting;
                }
            default:

                if(error_string == NULL)
                    error_string = "Неизвестная ошибка";
                else
                    tea_ui_chat_status_text(error_string);

                // Account has logouting in chances = 0
                if(check_chance_logouting == 0)
                {
                    tea_logout();
                }

                // wait
                sleep(1);

                if(error_string)
                    error(error_string);

                break;
        }
        // Resend
        if(check_chance_logouting)
        {
            thread_reply_msg = g_thread_new(NULL, G_CALLBACK(async_reply), NULL);
        }
    }
    return TRUE; // EVERYTHING
}

void ui_on_close_window(GtkWidget *window, gpointer data)
{
    // save login info
    tea_save();

    gtk_main_quit();
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

void tea_on_authenticate(struct tea_id_info *user_info)
{
    char buffer[300];
    app_settings.connected = TRUE;

    tea_ui_focus_tab(UI_TAB_CHAT);

    tea_ui_chat_enable(TRUE);

    tea_ui_chat_status_text("Вход выполнен.");

    tea_ui_chat_set_text_top(user_info->user_nickname);

    GDateTime *gtime = g_date_time_new_from_unix_local(user_info->creation_date);
    gchar *tmdate_regged = g_date_time_format(gtime, "%d.%m.%y %H:%M");
    g_free(gtime);
    gtime = g_date_time_new_from_unix_local(user_info->last_login);
    gchar *tmdate_logged = g_date_time_format(gtime, "%d.%m.%y %H:%M");
    g_free(gtime);
    snprintf(
        buffer,
        sizeof(buffer),
        "------\n"
        "Добро пожаловать на сервер Драконего Чая!\n"
        "Ваш ID: %lld\n"
        "Ваш Ник: %s\n"
        "Время регистраций: %s\n"
        "Время последнего входа: %s\n"
        "------\n",
        user_info->user_id,
        user_info->user_nickname,
        tmdate_regged,
        tmdate_logged);

    g_free(tmdate_regged);
    g_free(tmdate_logged);
    tea_ui_chat_push_text_raw(buffer, -1);

    // reset chances
    check_chance_logouting = CHANCE_TO_LOGOUT;

    on_chat_message_handler_async(NULL);

    widgets.chat_tab.timeout_periodic_sync = g_timeout_add(INTERVAL_CHAT_SYNC, G_CALLBACK(on_chat_message_handler_async), NULL);
}

void tea_on_logouted()
{
    app_settings.connected = FALSE;

    // remove message handler
    if(widgets.chat_tab.timeout_periodic_sync)
    {
        g_source_remove(widgets.chat_tab.timeout_periodic_sync);
        widgets.chat_tab.timeout_periodic_sync = 0;
    }

    // disable chat
    tea_ui_chat_enable(FALSE);

    // focusing to tab
    tea_ui_focus_tab(UI_TAB_AUTH);

    // wait computing external thread
    GThread *sender = thread_reply_msg;
    if(sender)
        g_thread_join(sender);
    // saved old data, reset at
    if(last_read_result.messages)
    {
        // Clear local messages
        for(int x = 0; x < last_read_result.messages->len; ++x)
        {
            free(g_array_index(last_read_result.messages, struct tea_message_id, x).message_text);
        }
        memset(&last_read_result, 0, sizeof(last_read_result));
    }

    // Clear local messages
    for(int x = 0; x < app_settings.local_msg_db->len; ++x)
    {
        free(g_array_index(app_settings.local_msg_db, struct tea_message_id, x).message_text);
    }
    g_array_set_size(app_settings.local_msg_db, 0);


}
