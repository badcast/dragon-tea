#include "tea.h"

#include <sys/types.h> // Для Linux
// #include <direct.h> // Для Windows

struct tea_settings app_settings;
struct widgets
{
    GtkWidget *widget_main;
    GtkWidget *widget_auth;
    GtkWindow *main_window;
} widgets;

const char *get_conf_dir();

void tea_load_conf(struct tea_settings *tea, const char *filename);

void tea_save_conf(const struct tea_settings *save_tea, const char *filename);

int tea_version()
{
    return TEA_APP_VERSION;
}

void on_close_window(GtkWidget *window, gpointer data)
{
    gtk_main_quit();
}

int tea_login(long user_id, struct tea_login_result *login_info)
{
    int result;
    if(result = net_api_signin(user_id, login_info))
    {
        memcpy(&app_settings.user_info, &login_info->result, sizeof(app_settings.user_info));
        on_tea_authenticated(&login_info->result);
    }
    return result;
}

int tea_try_login()
{
    struct tea_login_result slogin;
    int has_id = app_settings.user_info.user_id != -1 && strcmp("", app_settings.user_info.user_nickname) != 0;
    if(has_id == TRUE)
    {
        tea_login(app_settings.user_info.user_id, &slogin); // login to system
    }
    return has_id && tea_is_connected();
}

int tea_create_account(const char *nickname, tea_register_result *registration_info)
{
    return net_api_signup(nickname, registration_info);
}

int tea_create_account_and_signin(const char *nickname, tea_register_result *registration_info)
{
    int result = tea_create_account(nickname, registration_info);
    if(result)
    {
        memcpy(&app_settings.user_info, &registration_info->result, sizeof(app_settings.user_info));
        on_tea_authenticated(&registration_info->result);
    }
    return result;
}

void tea_logout()
{
}

gboolean on_message_watching(gpointer user_data)
{

    return TRUE; // EVERYTHING
}

void on_tea_authenticated(struct tea_user_info *user_info)
{
    app_settings.connected = TRUE;
    app_settings.timerid_message_watcher = g_timeout_add(400, G_CALLBACK(on_message_watching), NULL);
}

void on_tea_logouted()
{
}

void tea_run()
{
    const char *conf_dir;

    conf_dir = get_conf_dir();
    strncat(app_settings.setting_filename, conf_dir, TEA_MAXLEN_PATH);
    strncat(app_settings.setting_filename, "/tea.json", TEA_MAXLEN_PATH);

    // load settings
    tea_load_conf(&app_settings, app_settings.setting_filename);

    // Инициализация окна
    GtkWidget *main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "Драконий Чай");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 500, 400);
    gtk_window_set_position(main_window, GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(main_window), FALSE);
    gtk_window_set_icon_name(GTK_WINDOW(main_window), "mail-message-new-symbolic");
    g_signal_connect(main_window, "destroy", G_CALLBACK(on_close_window), NULL);

    GtkWidget *notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(main_window), notebook);

    // Инициализация виджетов
    widgets.main_window = GTK_WINDOW(main_window);

    widgets.widget_auth = create_auth_widget(on_tea_authenticated);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widgets.widget_auth, gtk_label_new("Аутентификация"));
    widgets.widget_main = create_main_widget();
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widgets.widget_main, gtk_label_new("Драконий чат"));

    GtkWidget *button_about = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_button_set_label(GTK_BUTTON(button_about), "О программе");
    g_signal_connect(button_about, "clicked", G_CALLBACK(show_about_dialog), NULL);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), NULL, button_about);

    // Show window
    gtk_widget_show_all(main_window);

    // Попытка автоматический войти, если настройки успешно загружены
    if(tea_try_login() == FALSE)
    {
        gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 0);
    }
    else
    {
        gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 1);
    }
}

const char *get_conf_dir()
{
    const char *home_dir;
#ifdef __linux__
    home_dir = getenv("HOME");
    if(home_dir == NULL)
    {
        home_dir = "/tmp/temp-user-tea"; // set as default to tmp
    }
    strncat(app_settings.config_dir, home_dir, TEA_MAXLEN_PATH);
    strncat(app_settings.config_dir, "/.config/DragonTea", TEA_MAXLEN_PATH);
    if(mkdir(app_settings.config_dir, 0700) != 0 && errno != EEXIST)
    {
        error_fail(strerror(errno));
    }
#elif WIN32
    home_dir = getenv("APPDATA");
    // strcat(tea_settings.config_dir, home_dir);
    // strcat(tea_settings.config_dir, "/DragonTea");
#endif

    return app_settings.config_dir;
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
        long len;
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

        struct json_object *parser = json_tokener_parse(buff);
        free(buff);
        len = 0;
        struct json_object *userId, *user_nickname;
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
            len = json_object_get_string_len(user_nickname);
            tea->user_info.user_id = json_object_get_int64(userId);
            strncpy(tea->user_info.user_id, json_object_get_string(user_nickname), len);
        }

        json_object_put(parser);
    }
    else
    {
        // File not permit
        // then set application settings to default

        // set user id to default
        memset(&tea->user_info, 0, sizeof(tea->user_info));
        tea->user_info.user_id = -1;
    }
}
void tea_save_conf(const struct tea_settings *save_tea, const char *filename)
{
}

void error(const char *str)
{
    GtkWidget *dialog = gtk_message_dialog_new(widgets.main_window, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Ошибка");
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), str);

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
#define MACRO_PUT(X)       \
    case X:                \
        result_value = #X; \
        break;
    const char *result_value = NULL;
    switch(error_code)
    {
        MACRO_PUT(TEA_STATUS_OK);
        MACRO_PUT(TEA_STATUS_ID_NO_EXIST);
        MACRO_PUT(TEA_STATUS_INVALID_REQUEST_DATA);
        MACRO_PUT(TEA_STATUS_INVALID_AUTH_METHOD);
        MACRO_PUT(TEA_STATUS_INVALID_NICKNAME);
        MACRO_PUT(TEA_STATUS_INVALID_REGISTER);
        MACRO_PUT(TEA_STATUS_INTERNAL_SERVER_ERROR);
        MACRO_PUT(TEA_STATUS_NETWORK_ERROR);
        default:
        result_value = "Unknown";
        break;
    }
    return result_value;
#undef MACRO_PUT
}

int tea_is_connected()
{
    return app_settings.connected;
}
