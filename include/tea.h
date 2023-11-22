/*
 * author: badcast <lmecomposer@gmail.com>
 * year 2023
 * status: already coding
 */

#ifndef TEA_HEADER_H
#define TEA_HEADER_H

#ifdef __linux__
#define TEA_OS_LINUX
#include <unistd.h>
#elif WIN32
#define TEA_OS_WINDOWS
#include <windows.h>
typedef DWORD uid_t;
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <json-c/json.h>

#include "tea_structs.h"
#include "tea_api.h"
#include "tea_localization.h"

#ifndef DRAGON_TEA_VERSION
#define DRAGON_TEA_VERSION "Release"
#endif

#ifndef DRAGON_TEA_LOCALE_DIR
#define DRAGON_TEA_LOCALE_DIR "locale"
#endif

#ifndef MESSAGES_PER_REQUEST
#define MESSAGES_PER_REQUEST 32
#endif

#ifndef INTERVAL_SEND
#define INTERVAL_SEND 100
#endif

#ifndef INTERVAL_CHAT_SYNC
#define INTERVAL_CHAT_SYNC 1450
#endif

#ifndef CHANCE_TO_LOGOUT
#define CHANCE_TO_LOGOUT 4
#endif

extern struct tea_net_stats net_stats;
extern struct tea_settings app_settings;
extern struct tea_app_widgets widgets;

void tea_init();

const char *tea_version();

// User Interface Implementation
GtkWidget *create_auth_widget();
GtkWidget *create_chat_widget();
GtkWidget *create_settings_widget();
void show_about_dialog();
void show_log_dialog();

// TODO: Get server version
int tea_server_version();
// result state is connected (Logged in system)
int tea_is_connected();
// to login
void tea_login(const struct tea_id_info *login);
// try login from settings
int tea_try_login();
// to logout
void tea_logout();

int tea_get_server_id(const char *serverURI);

int tea_get_builtin_server_list(char list[3][64]);

void tea_load();

void tea_save();

void error(const char *str);

void error_fail(const char *str);

const char *error_string(int error_code);

void net_init();

void net_free();

int net_api_read_messages(
    const struct tea_id_info *user,
    tea_id_t target_user_id,
    tea_id_t msg_id_start,
    int max_messages,
    struct tea_message_read_result *output);

int net_api_write_message(
    const struct tea_id_info *user_sender, tea_id_t target_user_id, const char *message, int len, struct tea_message_send_result *output);

int net_api_signin(tea_id_t user_id, tea_login_result *output);

int net_api_signup(const char *nickname, tea_register_result *output);

int net_api_server_info(struct tea_server_info *serverInfo);

void tea_ui_init();

void tea_ui_focus_tab(enum UiTabs tabIndex);

void tea_ui_chat_enable(int value);

void tea_ui_chat_interactable(int value);

void tea_ui_chat_status_text(const char *status_text);

void tea_ui_chat_clear();

void tea_ui_chat_sync();

void tea_ui_chat_set_text_top(const char *text);

void tea_ui_chat_set_text(const char *text);

void tea_ui_chat_vscroll_max();

void tea_ui_chat_push_block(const struct tea_message_id *message);

void tea_ui_chat_push_text_raw(const char *text, int len);

void tea_ui_auth_set_id(tea_id_t user_id);

void tea_ui_auth_sigin();

void tea_ui_auth_lock(gboolean state);

void tea_ui_reg_lock(gboolean state);

void tea_ui_update_settings();

#endif
