#include "tea.h"

#if __linux__
#include <sys/types.h>
#include <sys/stat.h>
#elif WIN32
#include <direct.h>
#endif

#include "ui_callbacks.h"

struct tea_settings app_settings;

const char *get_conf_dir();

void tea_init()
{
    // load settings
    tea_load();

    // init user interface
    tea_ui_init();
}

void tea_load_conf(struct tea_settings *tea, const char *filename);

void tea_save_conf(const struct tea_settings *save_tea, const char *filename);

void tea_load();

void tea_save();

int tea_version()
{
    return TEA_APP_VERSION;
}

int tea_is_connected()
{
    return app_settings.connected;
}

void tea_login(const struct tea_id_info *login_info)
{
    if(tea_is_connected())
        tea_logout();

    memcpy(&(app_settings.id_info), login_info, sizeof(app_settings.id_info));

    // event on authenticate
    tea_on_authenticate(login_info);
}

int tea_try_login()
{
    int result;
    if(result = app_settings.id_info.user_id != -1 && strcmp("", app_settings.id_info.user_nickname) != 0)
    {
        tea_ui_auth_sigin();
    }
    return result;
}

void tea_logout()
{
    tea_on_logouted();
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

    // Clear local messages
    g_array_set_size(app_settings.local_msg_db, 0);
}

const char *get_conf_dir()
{
    const char *home_dir;
    if(strlen(app_settings.config_dir) == 0)
    {
#ifdef __linux__
        home_dir = getenv("HOME");
        if(home_dir == NULL)
        {
            home_dir = "/tmp/temp-user-tea"; // set as default to tmp
            mkdir(home_dir, 0700);
        }
        strncpy(app_settings.config_dir, home_dir, sizeof(app_settings.config_dir));
        strncat(app_settings.config_dir, "/.config/DragonTea", sizeof(app_settings.config_dir));
        if(mkdir(app_settings.config_dir, 0700) != 0 && errno != EEXIST)
        {
            g_print(strerror(errno));
        }
#elif WIN32
        home_dir = getenv("APPDATA");
        strcat(app_settings.config_dir, home_dir);
        strcat(app_settings.config_dir, "\\DragonTea");

        _mkdir(app_settings.config_dir);
#endif
    }

    return app_settings.config_dir;
}

void tea_load()
{
    const char *conf_dir = get_conf_dir();
    strncat(app_settings.setting_filename, conf_dir, sizeof(app_settings.setting_filename) - 1);
    strncat(app_settings.setting_filename, "/tea.json", sizeof(app_settings.setting_filename) - 1);
    app_settings.local_msg_db = g_array_new(FALSE, FALSE, sizeof(struct tea_message_id));
    tea_load_conf(&app_settings, app_settings.setting_filename);
}

void tea_save()
{
    tea_save_conf(&app_settings, app_settings.setting_filename);
}

void tea_load_conf(struct tea_settings *tea, const char *filename)
{
    FILE *file;
    int _test = access(filename, F_OK) | access(filename, R_OK);
    file = fopen(filename, "r");
    // File exist and accessed,
    // then read application settings
    if(_test == 0 && file != NULL)
    {
        int len;
        char *buff;

        // get length of file
        fseek(file, 0, SEEK_END);
        len = ftell(file);
        fseek(file, 0, SEEK_SET);

        // read
        buff = (char *) malloc(len);
        fread(buff, 1, len, file);

        // close file
        fclose(file);

        struct json_object *userId, *user_nickname;
        struct json_object *parser = json_tokener_parse(buff);
        len = 0;
        free(buff);
        if(!json_object_object_get_ex(parser, "user_id", &userId))
        {
            len |= 1;
        }
        if(!json_object_object_get_ex(parser, "user_nickname", &user_nickname))
        {
            len |= 2;
        }

        if(len == 0)
        {
            tea->id_info.user_id = json_object_get_int64(userId);
            len = json_object_get_string_len(user_nickname);
            if(len < sizeof(tea->id_info.user_nickname))
                strncpy(tea->id_info.user_nickname, json_object_get_string(user_nickname), len);
        }

        json_object_put(parser);
    }
    else
    {
        // File not permit
        // then set application settings to default

        // set user id to default
        memset(&tea->id_info, 0, sizeof(tea->id_info));
        tea->id_info.user_id = -1;
    }
}

void tea_save_conf(const struct tea_settings *save_tea, const char *filename)
{
    json_object *jdata;

    // save only unchanged data
    struct tea_settings _from;
    memset(&_from, 0, sizeof(_from));
    tea_load_conf(&_from, filename);
    if(_from.id_info.user_id == save_tea->id_info.user_id && strcmp(save_tea->id_info.user_nickname, _from.id_info.user_nickname) == 0)
    {
        return;
    }

    jdata = json_object_new_object();
    json_object_object_add(jdata, "user_id", json_object_new_int64(save_tea->id_info.user_id));
    json_object_object_add(jdata, "user_nickname", json_object_new_string(save_tea->id_info.user_nickname));

    // save as
    if(json_object_to_file_ext(filename, jdata, JSON_C_TO_STRING_PRETTY) != 0)
    {
        g_print("error saving settings to \"%s\". %s", filename, json_util_get_last_err());
    }
}

void error(const char *str)
{
    GtkWidget *dialog = gtk_message_dialog_new(widgets.main_window, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Ошибка");
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),"%s", str);

    gtk_dialog_run(GTK_DIALOG(dialog));

    gtk_widget_destroy(dialog);
}

void error_fail(const char *str)
{
    error(str);
    exit(EXIT_FAILURE);
}

const char *tea_get_error_string(int error_code)
{
#define MACRO_PUT_CASE(X)  \
    case X:                \
        result_value = #X; \
        break;
    const char *result_value = NULL;
    switch(error_code)
    {
        MACRO_PUT_CASE(TEA_STATUS_OK);
        MACRO_PUT_CASE(TEA_STATUS_ID_NO_EXIST);
        MACRO_PUT_CASE(TEA_STATUS_INVALID_REQUEST_DATA);
        MACRO_PUT_CASE(TEA_STATUS_INVALID_AUTH_METHOD);
        MACRO_PUT_CASE(TEA_STATUS_INVALID_NICKNAME);
        MACRO_PUT_CASE(TEA_STATUS_INVALID_REGISTER);
        MACRO_PUT_CASE(TEA_STATUS_ADMIN_ACCOUNT_REACHABLE);
        MACRO_PUT_CASE(TEA_STATUS_PRIVATE_MESSAGE_NOT_SUPPORTED);

        MACRO_PUT_CASE(TEA_STATUS_INTERNAL_SERVER_ERROR);

        MACRO_PUT_CASE(TEA_STATUS_NETWORK_ERROR);
        default:
            result_value = "Unknown";
            break;
    }
    return result_value;
#undef MACRO_PUT_CASE
}
