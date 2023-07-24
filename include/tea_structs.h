/*
 * author: badcast <lmecomposer@gmail.com>
 * year 2023
 * status: already coding
 */

#ifndef TEA_STRUCTS_H
#define TEA_STRUCTS_H

#define TEA_MAXLEN_MESSAGE 128
#define TEA_MAXLEN_USERNAME 16

// Set Max Filesystem Path
#ifdef __linux__
#define TEA_MAXLEN_PATH PATH_MAX
#elif WIN32
#define TEA_MAXLEN_PATH MAX_PATH
#endif

struct tea_message
{
    long message_id;
    const char message_text[TEA_MAXLEN_MESSAGE];
};

struct tea_id
{
    long user_id;
    const char user_name[TEA_MAXLEN_USERNAME];
};

struct tea_user_info
{
    long user_id;
    char user_nickname[TEA_MAXLEN_USERNAME];
    time_t creation_date;
    time_t last_login;
};

struct tea_settings
{
    // user information
    struct tea_user_info user_info;
    // status of connected to server (remote)
    int connected;
    // configuration directory
    char config_dir[TEA_MAXLEN_PATH];
    // setting file
    char setting_filename[TEA_MAXLEN_PATH];
    // message handler for update timer
    int timerid_message_watcher;
};

struct tea_login_result
{
    json_bool authorized;
    int error;
    struct tea_user_info result;
};

typedef struct tea_login_result tea_register_result;

#endif
