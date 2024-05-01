#include "tea.h"

#ifdef TEA_OS_LINUX
#include <sys/types.h>
#include <sys/stat.h>
#elif TEA_OS_WINDOWS
#include <direct.h>
#endif

#include "ui_callbacks.h"

struct tea_settings env;

const char *get_conf_dir();

void tea_init()
{
    // Init networking
    net_init();

    // load settings
    tea_load();

    // init user interface
    tea_ui_init();
}

void tea_free()
{
    net_free();
    notify_uninit();
}

void tea_load_conf(struct tea_settings *tea, const char *filename);

void tea_save_conf(const struct tea_settings *save_tea, const char *filename);

void tea_load();

void tea_save();

const char *tea_error_string(int error_code)
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

const char *tea_version()
{
    return DRAGON_TEA_VERSION;
}

int tea_is_connected()
{
    return env.connected;
}

void tea_login(const struct tea_id_info *login_info)
{
    tea_logout();

    memcpy(&(env.id_info), login_info, sizeof(env.id_info));

    // event on authenticate
    tea_on_authenticate(login_info);
}

int tea_try_login()
{
    int result = env.id_info.user_id != -1 && strlen(env.id_info.user_nickname) > 0;
    if(result)
    {
        tea_ui_auth_set_id(env.id_info.user_id);
        tea_ui_auth_sigin();
    }
    return result;
}

const char *get_conf_dir()
{
    const char *home_dir;
    if(strlen(env.config_dir) == 0)
    {

#ifdef TEA_OS_LINUX
        home_dir = getenv("HOME");
        if(home_dir == NULL)
        {
            home_dir = "/tmp/temp-user-tea"; // set as default to tmp
            mkdir(home_dir, 0700);
        }
        strncpy(env.config_dir, home_dir, sizeof(env.config_dir));
        strncat(env.config_dir, "/.config/DragonTea", sizeof(env.config_dir));
        if(mkdir(env.config_dir, 0700) != 0 && errno != EEXIST)
        {
            printf("%s", strerror(errno));
        }
#elif TEA_OS_WINDOWS
        home_dir = getenv("APPDATA");
        strcat(app_settings.config_dir, home_dir);
        strcat(app_settings.config_dir, "\\DragonTea");

        _mkdir(app_settings.config_dir);
#endif
    }

    return env.config_dir;
}

void tea_load()
{
    const char *conf_dir = get_conf_dir();
    strncat(env.setting_filename, conf_dir, sizeof(env.setting_filename) - 1);
    strncat(env.setting_filename, "/tea-config.json", sizeof(env.setting_filename) - 1);
    env.local_msg_db = g_array_new(FALSE, FALSE, sizeof(struct tea_message_id));
    tea_load_conf(&env, env.setting_filename);

    // Init logs
    env.log_buffer = gtk_text_buffer_new(NULL);

    // switching server
    tea_switch_server(env.active_server);
}

void tea_save()
{
    tea_save_conf(&env, env.setting_filename);
}

void tea_load_conf(struct tea_settings *tea, const char *filename)
{
    char *buff;
    int deltaFlag, validate;
    size_t len;
    FILE *config_file;
    struct json_object *userId, *user_nickname, *active_server, *servers, *show_logs, *old_notify, *autoLogin, *server, *parser;

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

        // size config file >= 5MB
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

        if(!json_object_object_get_ex(parser, "show_logs", &show_logs) || !json_object_is_type(show_logs, json_type_boolean))
            tea->show_logs = 1; // Show as default
        else
            tea->show_logs = (int) json_object_get_boolean(show_logs);

        if(!json_object_object_get_ex(parser, "autologin", &autoLogin) || !json_object_is_type(autoLogin, json_type_boolean))
            tea->autologin = 1;
        else
            tea->autologin = (int) json_object_get_boolean(autoLogin);

        if(!json_object_object_get_ex(parser, "remove_old_notify", &old_notify) || !json_object_is_type(old_notify, json_type_boolean))
            tea->old_notify_remove = 1; // Remove old notify = yes as default
        else
            tea->old_notify_remove = (int) json_object_get_boolean(old_notify);

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
        char builtin_servers[BUILTIN_SERVERS_N][64];
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
    json_object *confJson, *jarr;
    size_t cmp1, cmp2;
    int len;

    // save only unchanged data
    struct tea_settings prevset;
    memset(&prevset, 0, sizeof(prevset));

    tea_load_conf(&prevset, filename);

    for(len = 0, cmp1 = 1; strlen(prevset.servers[len]); ++len)
        cmp1 *= 1 | tea_get_server_id(prevset.servers[len]);

    for(len = 0, cmp2 = 1; strlen(env.servers[len]); ++len)
        cmp2 *= 1 | tea_get_server_id(env.servers[len]);

    if(prevset.id_info.user_id == save_tea->id_info.user_id &&
       strcmp(save_tea->id_info.user_nickname, prevset.id_info.user_nickname) == 0 && prevset.active_server == env.active_server &&
       cmp1 == cmp2 && save_tea->show_logs == prevset.show_logs && save_tea->old_notify_remove == prevset.old_notify_remove &&
       save_tea->autologin == prevset.autologin)
    {
        return;
    }

    confJson = json_object_new_object();
    json_object_object_add(confJson, "user_id", json_object_new_int64(save_tea->id_info.user_id));
    json_object_object_add(confJson, "user_nickname", json_object_new_string(save_tea->id_info.user_nickname));
    json_object_object_add(confJson, "active_server", json_object_new_int(save_tea->active_server));
    json_object_object_add(confJson, "show_logs", json_object_new_boolean(save_tea->show_logs));
    json_object_object_add(confJson, "remove_old_notify", json_object_new_boolean(save_tea->old_notify_remove));
    json_object_object_add(confJson, "autologin", json_object_new_boolean(save_tea->autologin));

    // save server list
    jarr = json_object_new_array_ext(len);
    for(; len-- > 0;)
    {
        json_object_array_add(jarr, json_object_new_string(env.servers[len]));
    }
    json_object_object_add(confJson, "servers", jarr);

    // save as
    if(json_object_to_file_ext(filename, confJson, JSON_C_TO_STRING_PRETTY) != 0)
    {
        printf("error saving settings to \"%s\". %s", filename, json_util_get_last_err());
    }

    json_object_put(confJson);

}

void ui_error(const char *str)
{
    tea_log(str);

    GtkWidget *dialog = gtk_message_dialog_new(
        widgets.main_window, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Error"));
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", str);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void ui_error_fail(const char *str)
{
    ui_error(str);
    exit(EXIT_FAILURE);
}

int tea_server_version()
{
    if(strlen(cur_server.urls.url_base) == 0)
        return -1;

    return (int) (cur_server.server_version.major | cur_server.server_version.minor << 8 | cur_server.server_version.patch << 16);
}
