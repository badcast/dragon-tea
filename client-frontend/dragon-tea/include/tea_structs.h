/*
 * author: badcast <lmecomposer@gmail.com>
 * year 2023
 * status: already coding
 */

#ifndef TEA_STRUCTS_H
#define TEA_STRUCTS_H

#include <glib.h>
#include <gtk/gtk.h>

#define TEA_MAXLEN_MESSAGE 255
#define TEA_MAXLEN_USERNAME 16

// Set Max Filesystem Path
#ifdef TEA_OS_LINUX
#define TEA_MAXLEN_PATH PATH_MAX
#elif TEA_OS_WINDOWS
#define TEA_MAXLEN_PATH MAX_PATH
#endif

typedef unsigned long long tea_id_t;

struct tea_id_info
{
    tea_id_t user_id;
    int64_t creation_date;
    int64_t last_login;
    char user_nickname[TEA_MAXLEN_USERNAME * 2];
};

struct tea_message_id
{
    tea_id_t msg_id;
    int64_t time_saved;
    int64_t time_received;
    tea_id_t sent_user_id;
    char sent_user_name[TEA_MAXLEN_USERNAME * 2];
    char *message_text;
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
    int64_t time_received;
    int64_t time_saved;
    tea_id_t msg_id;
};

struct tea_net_stats
{
    size_t transmitted_bytes;
    size_t received_bytes;
    size_t lost_bytes;

    int active_requests;
    int success_req;
    int error_req;
};

struct tea_server_urls
{
    char url_base[64];
    char url_auth[64];
    char url_reg[64];
    char url_msg_handler[64];
    char url_info[64];
};

// Server features
enum
{
    // Server can get information
    SV_VERFI = 1,
    // Server can get First or Last MsgID
    SV_MSGFL = 2,
};

struct tea_server_info
{
    struct
    {
        char major;
        char minor;
        char patch;
    } server_version;

    char maintainer[32];
    char license[16];

    int features;

    struct tea_server_urls urls;
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
    // active server
    int active_server;
    // loaded from configurations
    char servers[32][255];
    // User messages
    GArray *local_msg_db;
    // Log I/O
    GtkTextBuffer *log_buffer;
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
        GtkWidget *label_traffics;
        GtkWidget *entry_message_set;
        GtkWidget *top_box;
        GtkWidget *top_button_logout;
        GtkWidget *top_label_user_state;
        guint timeout_periodic_sync;
        gboolean chat_synched;
    } chat_tab;

    struct
    {
        GtkWidget *combx_server_list;
        GtkWidget *button_apply;
    } settings_tab;

    GtkWidget *widget_main;
    GtkWidget *widget_auth;
    GtkWidget *widget_settings;
    GtkWindow *main_window;

    GtkWidget *notebook;
};

enum UiTabs
{
    UI_TAB_AUTH = 0,
    UI_TAB_CHAT = 1
};

#endif
