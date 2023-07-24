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
#include <curl/curl.h>
#include <json-c/json.h>

#include "tea_structs.h"
#include "tea_api.h"

extern struct tea_settings app_settings;

void tea_run();

int tea_version();

// TODO: Get server version
int tea_server_version();

// result state is connected (Logged in system)
int tea_is_connected();
// to login
int tea_login(long user_id, struct tea_login_result *login_info);
// try login from settings
int tea_try_login();
// create a new account
int tea_create_account(const char *nickname, tea_register_result *registration_info);
// create account and signin
int tea_create_account_and_signin(const char *nickname, tea_register_result *registration_info);
// to logout
void tea_logout();

void error(const char *str);

void error_fail(const char *str);

const char *tea_get_error_string(int error_code);

void net_init();

void net_free();

int net_api_signin(long user_id, struct tea_login_result *output);

int net_api_signup(const char *nickname, tea_register_result *output);

GtkWidget *create_auth_widget();

GtkWidget *create_main_widget();

void show_about_dialog(gpointer, gpointer);

#endif
