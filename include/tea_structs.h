/*
 * author: badcast <lmecomposer@gmail.com>
 * year 2023
 * status: already coding
 */

#ifndef TEA_STRUCTS_H
#define TEA_STRUCTS_H

#include <glib/garray.h>

#define TEA_MAXLEN_MESSAGE 255
#define TEA_MAXLEN_USERNAME 16

// Set Max Filesystem Path
#ifdef __linux__
#define TEA_MAXLEN_PATH PATH_MAX
#elif WIN32
#define TEA_MAXLEN_PATH MAX_PATH
#endif

typedef long long tea_id_t;

struct tea_id_info
{
    tea_id_t user_id;
    long creation_date;
    long last_login;
    char user_nickname[TEA_MAXLEN_USERNAME];
};

struct tea_message_id
{
    tea_id_t msg_id;
    long time_saved;
    long time_received;
    tea_id_t sent_user_id;
    char sent_user_name[TEA_MAXLEN_USERNAME];
    char message_text[TEA_MAXLEN_MESSAGE];
};

struct tea_message_read_result
{
    int status;
    size_t message_length;
    GArray *messages;
};

struct tea_message_send_result
{
    int status;
    long time_received;
    long time_saved;
    tea_id_t msg_id;
};

typedef struct
{
    int authorized;
    int status;
    struct tea_id_info result;
} tea_login_result, tea_register_result;

struct tea_settings
{
    // user information
    struct tea_id_info id_info;
    // status of connected to server (remote)
    int connected;
    // configuration directory
    char config_dir[TEA_MAXLEN_PATH];
    // setting file
    char setting_filename[TEA_MAXLEN_PATH];
    // message handler
    int timer_message_handler;

    // User messages
    GArray *local_msg_db;
};

struct tea_app_widgets
{
    struct
    {
        GtkWidget *button_send;
        GtkWidget *button_generate;
        GtkWidget *image_loading;
        GtkWidget *entry_username;
        GThread *gtask;
        tea_register_result tea_reg_result;
    } signup_tab;

    struct
    {
        GtkWidget *button_send;
        GtkWidget *image_loading;
        GtkWidget *entry_userid;
        GThread *gtask;
        tea_login_result tea_signed_result;
    } signin_tab;

    struct
    {
        GtkWidget *textview_chat;
        GtkWidget *scrolled_window_chat;
        GtkWidget *button_send;
        GtkWidget *label_status;
        GtkWidget *entry_message_set;
    } chat_tab;

    GtkWidget *widget_main;
    GtkWidget *widget_auth;
    GtkWindow *main_window;

    GtkWidget *notebook;
};

enum UiTabs
{
    UI_TAB_AUTH = 0,
    UI_TAB_CHAT = 1
};

#endif
