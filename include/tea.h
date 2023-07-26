/*
 * author: badcast <lmecomposer@gmail.com>
 * year 2023
 * status: already coding
 */

#ifndef TEA_HEADER_H
#define TEA_HEADER_H

#if WIN32
#include <windows.h>
typedef DWORD uid_t;
#elif __linux__
#include <unistd.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <time.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <json-c/json.h>

#include "tea_structs.h"
#include "tea_api.h"

extern struct tea_settings app_settings;
extern struct tea_app_widgets widgets;

void tea_init();

int tea_version();

// User Interface Implementation
GtkWidget *create_auth_widget();
GtkWidget *create_chat_widget();
void show_about_dialog();

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

void tea_load();

void tea_save();

void error(const char *str);

void error_fail(const char *str);

const char *tea_get_error_string(int error_code);

void net_init();

void net_free();

int net_api_read_messages(const struct tea_id_info *user, tea_id_t target_user_id, tea_id_t msg_id_start, int max_messages, struct tea_message_read_result *output);

int net_api_write_message(
    const struct tea_id_info *user_sender, tea_id_t target_user_id, const char *message, int len, struct tea_message_send_result *output);

int net_api_signin(tea_id_t user_id, tea_login_result *output);

int net_api_signup(const char *nickname, tea_register_result *output);

void tea_ui_init();

void tea_ui_focus_tab(enum UiTabs tabIndex);

void tea_ui_chat_enable(int value);

void tea_ui_chat_interactable(int value);

void tea_ui_chat_status_text(const char *status_text);

void tea_ui_chat_clear();

void tea_ui_chat_set_text(const char *text);

void tea_ui_chat_vscroll_max();

void tea_ui_chat_push_block(struct tea_message_id *message);

void tea_ui_chat_push_text_raw(const char *text, int len);

void tea_ui_auth_sigin();

void tea_ui_auth_lock(gboolean state);

void tea_ui_reg_lock(gboolean state);

#endif
