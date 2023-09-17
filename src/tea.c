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

const char* tea_version()
{
    return DRAGON_TEA_VERSION;
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
    int result = app_settings.id_info.user_id != -1 && strcmp("", app_settings.id_info.user_nickname) != 0;
    if(result)
    {
        tea_ui_auth_set_id(app_settings.id_info.user_id);
        tea_ui_auth_sigin();
    }
    return result;
}

void tea_logout()
{
    tea_on_logouted();
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
            g_print("%s", strerror(errno));
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
    char *buff;
    int deltaFlag, validate;
    size_t len;
    FILE *config_file;
    struct json_object *userId, *user_nickname, *active_server, *servers, *server, *parser;

    // File not permit
    // then set application settings to default

    // set user id to default
    memset(&tea->id_info, 0, sizeof(tea->id_info));
    tea->id_info.user_id = -1;
    validate = ~0;

    // File exist and accessed,
    // then read application settings
    deltaFlag = access(filename, F_OK) | access(filename, R_OK);
    config_file = fopen(filename, "r");

    while(deltaFlag == 0 && config_file != NULL)
    {

        // get length of file
        fseek(config_file, 0, SEEK_END);
        len = ftell(config_file);

        // size config file >= 5M
        if(len >= 500000)
        {
            // config file is big (maybe corrupted)
            // close file
            fclose(config_file);
            break;
        }

        rewind(config_file);
        // read
        buff = (char *) malloc(len);
        fread(buff, 1, len, config_file);

        // close file
        fclose(config_file);

        parser = json_tokener_parse(buff);
        free(buff);

        validate = 0;

        if(!json_object_object_get_ex(parser, "user_id", &userId) || !json_object_is_type(userId, json_type_int))
            validate |= 1;
        if(!json_object_object_get_ex(parser, "user_nickname", &user_nickname) || !json_object_is_type(user_nickname, json_type_string))
            validate |= 2;
        if(!json_object_object_get_ex(parser, "active_server", &active_server) || !json_object_is_type(active_server, json_type_int))
            validate |= 4;
        if(!json_object_object_get_ex(parser, "servers", &servers) || !json_object_is_type(servers, json_type_array))
            validate |= 8;

        // nickname and userId
        if((validate & 3) == 0)
        {
            tea->id_info.user_id = json_object_get_int64(userId);
            len = json_object_get_string_len(user_nickname);
            if(len < sizeof(tea->id_info.user_nickname))
                strncpy(tea->id_info.user_nickname, json_object_get_string(user_nickname), len);
        }

        // active server
        if((validate & 4) == 0)
        {
            tea->active_server = json_object_get_int(active_server);
        }

        // servers
        if((validate & 8) == 0)
        {
            deltaFlag = json_object_array_length(servers);
            deltaFlag = MIN(sizeof(tea->servers) / sizeof(tea->servers[0]), deltaFlag);
            for(; deltaFlag-- > 0;)
            {
                server = json_object_array_get_idx(servers, deltaFlag);
                len = json_object_get_string_len(server);

                strncpy(tea->servers[deltaFlag], json_object_get_string(server), MIN(len, sizeof(tea->servers[0])));
            }
        }

        json_object_put(parser);
        break;
    }

    //  reset servers to built in an error
    if((validate & 8))
    {
        char builtin_servers[3][64];
        int list_num = tea_get_builtin_server_list(builtin_servers);
        for(; list_num-- > 0;)
        {
            len = strlen(builtin_servers[0]);
            strncpy(tea->servers[list_num], builtin_servers[list_num], len);
        }

        tea->active_server = tea_get_server_id(builtin_servers[0]); // set as first
    }
}

void tea_save_conf(const struct tea_settings *save_tea, const char *filename)
{
    json_object *jdata, *jarr;
    int len;
    size_t cmp1, cmp2;

    // save only unchanged data
    struct tea_settings _from;
    memset(&_from, 0, sizeof(_from));

    tea_load_conf(&_from, filename);

    for(len = 0, cmp1 = 1; strlen(_from.servers[len]); ++len)
        cmp1 *= 1 | tea_get_server_id(_from.servers[len]);

    for(len = 0, cmp2 = 1; strlen(app_settings.servers[len]); ++len)
        cmp2 *= 1 | tea_get_server_id(app_settings.servers[len]);

    if(_from.id_info.user_id == save_tea->id_info.user_id && strcmp(save_tea->id_info.user_nickname, _from.id_info.user_nickname) == 0 &&
       _from.active_server == app_settings.active_server && cmp1 == cmp2)
    {
        return;
    }

    jdata = json_object_new_object();
    json_object_object_add(jdata, "user_id", json_object_new_int64(save_tea->id_info.user_id));
    json_object_object_add(jdata, "user_nickname", json_object_new_string(save_tea->id_info.user_nickname));
    json_object_object_add(jdata, "active_server", json_object_new_int(save_tea->active_server));

    // save server list

    jarr = json_object_new_array_ext(len);
    for(; len-- > 0;)
    {
        json_object_array_add(jarr, json_object_new_string(app_settings.servers[len]));
    }
    json_object_object_add(jdata, "servers", jarr);

    // save as
    if(json_object_to_file_ext(filename, jdata, JSON_C_TO_STRING_PRETTY) != 0)
    {
        g_print("error saving settings to \"%s\". %s", filename, json_util_get_last_err());
    }
}

void error(const char *str)
{
    GtkWidget *dialog = gtk_message_dialog_new(
        widgets.main_window, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Ошибка");
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", str);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void error_fail(const char *str)
{
    error(str);
    exit(EXIT_FAILURE);
}

const char *error_string(int error_code)
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
