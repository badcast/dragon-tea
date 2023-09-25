#include "tea.h"

#ifndef BUILTIN_SERVERS
#define BUILTIN_SERVERS_N 1
static const char builtin_servers[BUILTIN_SERVERS_N][64] = {"https://dragontea.lightmister.repl.co/"};
#else
extern const char builtin_servers[BUILTIN_SERVERS_N][64];
#endif

static const char nicknames[102][17] = {
    "LuckyUnicorn",    "SadEagle",      "LuckyTiger",      "ShinyRabbit",   "BraveDog",       "CoolDolphin",   "JollyEagle",
    "SadCat",          "ShinyRabbit",   "JollyEagle",      "ShinyEagle",    "LuckyCat",       "FearlessPanda", "SadDragon",
    "HappyDolphin",    "LuckyUnicorn",  "CoolDragon",      "FunnyCat",      "CleverDog",      "ShinyPanda",    "CleverTiger",
    "ShinyDolphin",    "LuckyEagle",    "ShinyDragon",     "LuckyLion",     "LuckyPanda",     "JollyPanda",    "ShinyLion",
    "LuckyPanda",      "FearlessLion",  "CleverPanda",     "SadDolphin",    "FearlessDragon", "LuckyDragon",   "ShinyDragon",
    "FearlessPanda",   "FunnyCat",      "ShinyEagle",      "LuckyDolphin",  "FunnyEagle",     "LuckyPanda",    "SadUnicorn",
    "JollyUnicorn",    "LuckyDolphin",  "FearlessDolphin", "JollyLion",     "HappyUnicorn",   "HappyDog",      "BravePanda",
    "BraveEagle",      "HappyRabbit",   "LuckyLion",       "ShinyPanda",    "CleverDolphin",  "HappyCat",      "BraveCat",
    "FearlessTiger",   "FearlessEagle", "CleverCat",       "CleverDolphin", "BraveCat",       "ShinyCat",      "LuckyTiger",
    "FunnyDragon",     "CoolRabbit",    "FunnyTiger",      "LuckyTiger",    "LuckyUnicorn",   "ShinyPanda",    "LuckyEagle",
    "FearlessUnicorn", "ShinyLion",     "SadCat",          "BraveDragon",   "ShinyTiger",     "HappyEagle",    "FearlessDog",
    "LuckyDolphin",    "JollyTiger",    "JollyTiger",      "JollyCat",      "CoolPanda",      "LuckyCat",      "ShinyLion",
    "JollyDolphin",    "ShinyDragon",   "BraveEagle",      "FunnyCat",      "ShinyDog",       "FearlessPanda", "ShinyDragon",
    "BraveCat",        "LuckyCat",      "JollyCat",        "CleverDog",     "FunnyTiger",     "FearlessPanda", "FearlessTiger",
    "FunnyDolphin",    "HappyRabbit",   "Drakozyabra",     "Marina"};
static int select_nick = sizeof(nicknames) / sizeof(nicknames[0]);

const char *get_random_nickname()
{
    if(select_nick == sizeof(nicknames) / sizeof(nicknames[0]))
        select_nick = 0;
    else
        select_nick++;
    return nicknames[select_nick];
}

void on_signup_click(GtkWidget *, gpointer);

void on_signin_click(GtkWidget *, gpointer);

void on_click_generate_nick(GtkWidget *widget, GtkWidget *entry)
{
    gtk_entry_set_text(GTK_ENTRY(entry), get_random_nickname());
}

gpointer on_signup_async(const char *nickname)
{
    net_api_signup(nickname, &widgets.signup_tab.tea_reg_result);
    widgets.signup_tab.gtask = NULL;
    return NULL;
}

gpointer on_signin_async(const char *user_id_s)
{
    tea_id_t user_id = atoll(user_id_s);
    net_api_signin(user_id, &widgets.signin_tab.tea_signed_result);
    widgets.signin_tab.gtask = NULL;
    return NULL;
}

void tea_ui_reg_lock(gboolean state)
{
    gtk_widget_set_visible(widgets.signup_tab.image_loading, state);
    gtk_widget_set_sensitive(widgets.signup_tab.button_generate, !state);
    gtk_widget_set_sensitive(widgets.signup_tab.button_send, !state);
    gtk_widget_set_sensitive(widgets.signup_tab.entry_username, !state);
}

void tea_ui_auth_lock(gboolean state)
{
    gtk_widget_set_visible(widgets.signin_tab.image_loading, state);
    gtk_widget_set_sensitive(widgets.signin_tab.button_send, !state);
    gtk_widget_set_sensitive(widgets.signin_tab.entry_userid, !state);
}

gboolean on_sigin_user(gpointer userData)
{
    // Thread end state
    if(widgets.signin_tab.gtask == NULL)
    {
        char buf[256];
        if(widgets.signin_tab.tea_signed_result.authorized == FALSE)
        {
            const char *error_msg;
            switch(widgets.signin_tab.tea_signed_result.status)
            {
                case TEA_STATUS_ID_NO_EXIST:
                    error_msg = _("User is not exists.");
                    break;
                default:
                    error_msg = error_string(widgets.signin_tab.tea_signed_result.status);
                    break;
            }
            snprintf(
                buf,
                sizeof(buf),
                _("Authentication is not confirmed:\nError code: %d\nError message: %s"),
                widgets.signin_tab.tea_signed_result.status,
                error_msg);
            error(buf);
        }
        else
        {
            tea_login(&widgets.signin_tab.tea_signed_result.result);
        }
        tea_ui_auth_lock(FALSE);
        tea_ui_reg_lock(FALSE);
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

gboolean on_signup_user(gpointer userData)
{
    // Thread end state
    if(widgets.signup_tab.gtask == NULL)
    {
        char buf[256];
        if(widgets.signup_tab.tea_reg_result.authorized == FALSE)
        {
            snprintf(
                buf,
                sizeof(buf),
                _("Error registration:\nError codew: %d\nMessage error: %s"),
                widgets.signup_tab.tea_reg_result.status,
                error_string(widgets.signup_tab.tea_reg_result.status));
            error(buf);
        }
        else
        {
            // set new user id to auth
            tea_ui_auth_set_id(widgets.signup_tab.tea_reg_result.result.user_id);
            gtk_entry_set_text(GTK_ENTRY(widgets.signup_tab.entry_username), "");
            tea_login(&widgets.signup_tab.tea_reg_result.result);
        }
        tea_ui_auth_lock(FALSE);
        tea_ui_reg_lock(FALSE);
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

void tea_ui_auth_set_id(tea_id_t user_id)
{
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%lld", user_id);
    gtk_entry_set_text(GTK_ENTRY(widgets.signin_tab.entry_userid), buffer);
}

void tea_ui_auth_sigin()
{
    on_signin_click(NULL, NULL);
}

void on_signup_click(GtkWidget *widget, gpointer userData)
{
    const char *user_nickname;
    if(widgets.signup_tab.gtask != NULL)
        return;

    user_nickname = gtk_entry_get_text(GTK_ENTRY(widgets.signup_tab.entry_username));

    if(user_nickname == NULL || strlen(user_nickname) == 0)
    {
        error(_("The nickname should not be empty."));
        return;
    }

    // Logout if is logged
    tea_logout();

    memset(&widgets.signup_tab.tea_reg_result, 0, sizeof(widgets.signup_tab.tea_reg_result));
    widgets.signup_tab.gtask = g_thread_new(NULL, (GThreadFunc) on_signup_async, (gpointer) user_nickname);
    tea_ui_reg_lock(widgets.signup_tab.gtask != NULL);
    tea_ui_auth_lock(TRUE);
    g_timeout_add(1000, on_signup_user, NULL);
}

void on_signin_click(GtkWidget *widget, gpointer userData)
{
    tea_id_t user_id;

    if(widgets.signin_tab.gtask != NULL)
        return;

    user_id = atoll(gtk_entry_get_text(GTK_ENTRY(widgets.signin_tab.entry_userid)));
    if(user_id < 128)
    {
        error(_("You set is invalid ID"));
        return;
    }

    tea_logout();

    memset(&widgets.signin_tab.tea_signed_result, 0, sizeof(widgets.signin_tab.tea_signed_result));
    widgets.signin_tab.gtask =
        g_thread_new(NULL, (GThreadFunc) on_signin_async, (gpointer) gtk_entry_get_text(GTK_ENTRY(widgets.signin_tab.entry_userid)));
    tea_ui_auth_lock(widgets.signin_tab.gtask != NULL);
    tea_ui_reg_lock(TRUE);
    g_timeout_add(1000, on_sigin_user, NULL);
}

// entry only number
void on_insert_text(GtkEntry *entry, const gchar *text, gint length, gint *position, gpointer userData)
{
    for(int i = 0; i < length; i++)
    {
        if(!isdigit(text[i]))
        {
            g_signal_stop_emission_by_name(G_OBJECT(entry), "insert-text");
            return;
        }
    }
}

GtkWidget *create_auth_widget()
{
    GtkWidget *notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_LEFT);

    GtkWidget *signin_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *signup_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), signin_box, gtk_label_new(_("Sign in")));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), signup_box, gtk_label_new(_("Sign up")));

    // Поля Входа ----------------------------------------------------------------------------------------
    GtkWidget *vertical = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(signin_box), vertical, TRUE, TRUE, 0);

    GtkWidget *horizontal0 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(vertical), horizontal0, FALSE, TRUE, 0);
    GtkWidget *icon = gtk_image_new_from_icon_name("dialog-information-symbolic", GTK_ICON_SIZE_DIALOG);
    GtkWidget *label_info = gtk_label_new(_("Log in using your UserID\nnote: Numbers only, and it must be unique."));

    gtk_box_pack_start(GTK_BOX(horizontal0), icon, TRUE, TRUE, 50);
    gtk_box_pack_end(GTK_BOX(horizontal0), label_info, TRUE, TRUE, 0);

    gtk_widget_set_vexpand(horizontal0, TRUE);

    // Loading wideget
    GtkWidget *image_loading_state = widgets.signin_tab.image_loading = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *widgetImage = gtk_image_new_from_icon_name("content-loading-symbolic", GTK_ICON_SIZE_DIALOG);
    gtk_box_pack_start(GTK_BOX(image_loading_state), widgetImage, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(image_loading_state), gtk_label_new(_("Signing...")), FALSE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(vertical), image_loading_state, FALSE, TRUE, 0);

    GtkWidget *horizontal1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_end(GTK_BOX(vertical), horizontal1, FALSE, TRUE, 0);

    GtkWidget *entry_user_id = widgets.signin_tab.entry_userid = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_user_id), _("Set UserID..."));
    gtk_widget_set_hexpand(entry_user_id, TRUE);
    gtk_box_pack_start(GTK_BOX(horizontal1), entry_user_id, TRUE, TRUE, 0);
    g_signal_connect(entry_user_id, "insert-text", G_CALLBACK(on_insert_text), NULL);
    g_signal_connect(entry_user_id, "activate", G_CALLBACK(on_signin_click), entry_user_id);

    GtkWidget *send_button = widgets.signin_tab.button_send = gtk_button_new();
    icon = gtk_image_new_from_icon_name("contact-new", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_label(GTK_BUTTON(send_button), _("Sign in"));
    gtk_button_set_image(GTK_BUTTON(send_button), icon);
    g_signal_connect(send_button, "clicked", G_CALLBACK(on_signin_click), NULL);
    gtk_box_pack_start(GTK_BOX(horizontal1), send_button, FALSE, FALSE, 0);

    // Поля Регистраций-----------------------------------------------------------------------------------
    vertical = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(signup_box), vertical, TRUE, TRUE, 0);

    horizontal0 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(vertical), horizontal0, FALSE, TRUE, 0);
    icon = gtk_image_new_from_icon_name("dialog-information-symbolic", GTK_ICON_SIZE_DIALOG);
    label_info = gtk_label_new(_("Enter your nickname.\n"
                                 "Force the use of the Latin alphabet,\n"
                                 "the length of the nickname is not more than 16 and not less than 3."));
    gtk_box_pack_start(GTK_BOX(horizontal0), icon, TRUE, TRUE, 50);
    gtk_box_pack_end(GTK_BOX(horizontal0), label_info, TRUE, TRUE, 0);

    gtk_widget_set_vexpand(horizontal0, TRUE);

    image_loading_state = widgets.signup_tab.image_loading = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    widgetImage = gtk_image_new_from_icon_name("content-loading-symbolic", GTK_ICON_SIZE_DIALOG);
    gtk_box_pack_start(GTK_BOX(image_loading_state), widgetImage, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(image_loading_state), gtk_label_new(_("Sign up...")), FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vertical), image_loading_state, FALSE, TRUE, 0);

    horizontal1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_end(GTK_BOX(vertical), horizontal1, FALSE, TRUE, 0);

    GtkWidget *entry_username = widgets.signup_tab.entry_username = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry_username), TEA_MAXLEN_USERNAME);
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_username), _("Enter nickname..."));
    gtk_widget_set_hexpand(entry_username, TRUE);
    gtk_box_pack_start(GTK_BOX(horizontal1), entry_username, TRUE, TRUE, 0);
    g_signal_connect(entry_username, "activate", G_CALLBACK(on_signup_click), entry_username);

    // set icon for button
    GtkWidget *button_genick = widgets.signup_tab.button_generate = gtk_button_new();
    icon = gtk_image_new_from_icon_name("system-reboot-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(button_genick), icon);
    g_signal_connect(button_genick, "clicked", G_CALLBACK(on_click_generate_nick), entry_username);
    gtk_box_pack_start(GTK_BOX(horizontal1), button_genick, FALSE, FALSE, 0);

    send_button = widgets.signup_tab.button_send = gtk_button_new();
    gtk_button_set_label(GTK_BUTTON(send_button), _("Sign up"));
    icon = gtk_image_new_from_icon_name("contact-new", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(send_button), icon);
    g_signal_connect(send_button, "clicked", G_CALLBACK(on_signup_click), NULL);
    gtk_box_pack_start(GTK_BOX(horizontal1), send_button, FALSE, FALSE, 0);

    return notebook;
}

int tea_get_server_id(const char *serverURI)
{
    int id = 1;

    while(*serverURI && (id *= 1 | (int) *(serverURI++)))
        ;

    return id;
}

int tea_get_builtin_server_list(char list[3][64])
{
    memcpy(list, builtin_servers, sizeof(builtin_servers));
    return sizeof(builtin_servers) / sizeof(builtin_servers[0]);
}

const char *tea_get_server_uri()
{
    int s = -1;
    for(int x = 0; x < sizeof(app_settings.servers) / sizeof(app_settings.servers[0]); ++x)
    {
        if(strlen(app_settings.servers[x]) == 0)
            break;
        if(app_settings.active_server == tea_get_server_id(app_settings.servers[x]))
        {
            s = x;
            break;
        }
    }

    if(s == -1)
    {
        return NULL;
    }

    return app_settings.servers[s];
}

const char *tea_get_server_auth()
{
    static char SAUTH[255];
    const char *server = tea_get_server_uri();
    if(!server)
        return server;
    snprintf(SAUTH, sizeof(SAUTH), "%sapi/auth.php", server);
    return SAUTH;
}

const char *tea_get_server_register()
{
    static char SREGISTER[255];
    const char *server = tea_get_server_uri();
    if(!server)
        return server;
    snprintf(SREGISTER, sizeof(SREGISTER), "%sapi/register.php", server);
    return SREGISTER;
}

const char *tea_get_server_message_handler()
{
    static char SMHANDLER[255];
    const char *server = tea_get_server_uri();
    if(!server)
        return server;
    snprintf(SMHANDLER, sizeof(SMHANDLER), "%sapi/messageHandler.php", server);
    return SMHANDLER;
}
