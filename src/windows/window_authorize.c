#include "tea.h"

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
static int select_nick = sizeof(nicknames) / 17;

static struct
{
    GtkWidget *button_send;
    GtkWidget *button_generate;
    GtkWidget *image_loading;
    GtkWidget *entry_username;
    GThread *gtask;
    tea_register_result tea_reg_result;
} signup_widgets;

static struct
{
    GtkWidget *button_send;
    GtkWidget *image_loading;
    GtkWidget *entry_userid;
    GThread *gtask;
    struct tea_login_result tea_signed_result;
} signin_widgets;

const char *get_random_nickname()
{
    if(select_nick == sizeof(nicknames) / 17)
        select_nick = 0;
    else
        select_nick++;
    return nicknames[select_nick];
}

void on_click_generate_nick(GtkWidget *widget, GtkWidget *entry)
{
    gtk_entry_set_text(GTK_ENTRY(entry), get_random_nickname());
}

gpointer on_signup_async(const char *nickname)
{
    tea_create_account_and_signin(nickname, &signup_widgets.tea_reg_result);
    signup_widgets.gtask = NULL;
    return NULL;
}

gpointer on_signin_async(const char *user_id_s)
{
    long user_id = atol(user_id_s);
    tea_login(user_id, &signin_widgets.tea_signed_result);
    signin_widgets.gtask = NULL;
    return NULL;
}

void signup_ui_lock(int state)
{
    gtk_widget_set_visible(signup_widgets.image_loading, state);
    gtk_widget_set_sensitive(signup_widgets.button_generate, !state);
    gtk_widget_set_sensitive(signup_widgets.button_send, !state);
    gtk_widget_set_sensitive(signup_widgets.entry_username, !state);
}

void signin_ui_lock(int state)
{
    gtk_widget_set_visible(signin_widgets.image_loading, state);
    gtk_widget_set_sensitive(signin_widgets.button_send, !state);
    gtk_widget_set_sensitive(signin_widgets.entry_userid, !state);
}

gboolean on_sigin_user(gointer)
{
    // Thread end state
    if(signin_widgets.gtask == NULL)
    {
        char buf[256];
        if(signin_widgets.tea_signed_result.authorized == FALSE)
        {
            const char *error_msg;
            switch(signin_widgets.tea_signed_result.error)
            {
                case TEA_STATUS_ID_NO_EXIST:
                    error_msg = "Данный пользователь не существует.";
                    break;
                default:
                    error_msg = tea_get_error_string(signin_widgets.tea_signed_result.error);
                    break;
            }
            snprintf(
                buf,
                sizeof(buf),
                "Аутентификация не прошла:\nКод ошибки: %d\nСообщение ошибки: %s",
                signin_widgets.tea_signed_result.error,
                error_msg);
            error(buf);
        }
        signin_ui_lock(FALSE);
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

gboolean on_signup_user(gpointer)
{
    // Thread end state
    if(signup_widgets.gtask == NULL)
    {
        char buf[256];
        if(signup_widgets.tea_reg_result.authorized == FALSE)
        {
            snprintf(
                buf,
                sizeof(buf),
                "Ошибка регистраций:\nКод ошибки: %d\nСообщение ошибки: %s",
                signup_widgets.tea_reg_result.error,
                tea_get_error_string(signup_widgets.tea_reg_result.error));
            error(buf);
        }
        signup_ui_lock(FALSE);
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

void on_signup_click(GtkWidget *widget, gpointer)
{
    const char *user_nickname = gtk_entry_get_text(GTK_ENTRY(signup_widgets.entry_username));

    if(user_nickname == NULL || strlen(user_nickname) == 0)
    {
        error("Ник не может быть пустым.");
        return;
    }

    memset(&signup_widgets.tea_reg_result, 0, sizeof(signup_widgets.tea_reg_result));
    signup_widgets.gtask = g_thread_new(NULL, on_signup_async, user_nickname);
    signup_ui_lock(signup_widgets.gtask != NULL);
    g_timeout_add(1000, G_CALLBACK(on_signup_user), NULL);
}

void on_signin_click(GtkWidget *widget, gpointer)
{
    long user_id = atol(gtk_entry_get_text(GTK_ENTRY(signin_widgets.entry_userid)));

    if(user_id < 128)
    {
        error("Вы указали не валидный ID");
        return;
    }

    memset(&signin_widgets.tea_signed_result, 0, sizeof(signin_widgets.tea_signed_result));
    signin_widgets.gtask = g_thread_new(NULL, on_signin_async, gtk_entry_get_text(GTK_ENTRY(signin_widgets.entry_userid)));
    signin_ui_lock(signin_widgets.gtask != NULL);
    g_timeout_add(1000, G_CALLBACK(on_sigin_user), NULL);
}

void on_insert_text(GtkEntry *entry, const gchar *text, gint length, gint *position, gpointer user_data)
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
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), signin_box, gtk_label_new("Вход"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), signup_box, gtk_label_new("Зарегистрироваться"));

    // Поля Входа ----------------------------------------------------------------------------------------
    GtkWidget *vertical = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(signin_box), vertical, TRUE, TRUE, 0);

    GtkWidget *horizontal0 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(vertical), horizontal0, FALSE, TRUE, 0);
    GtkWidget *icon = gtk_image_new_from_icon_name("dialog-warning-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
    GtkWidget *label_info = gtk_label_new("Выполните вход через свой UserID\nПримечание: Только числа, и он должен быть уникальным.");

    gtk_box_pack_start(GTK_BOX(horizontal0), icon, TRUE, TRUE, 50);
    gtk_box_pack_end(GTK_BOX(horizontal0), label_info, TRUE, TRUE, 0);

    GtkWidget *image_loading_state = signin_widgets.image_loading =
        gtk_image_new_from_icon_name("content-loading-symbolic", GTK_ICON_SIZE_DIALOG);
    gtk_box_pack_start(GTK_BOX(vertical), image_loading_state, TRUE, TRUE, 0);

    GtkWidget *horizontal1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_end(GTK_BOX(vertical), horizontal1, FALSE, TRUE, 0);

    GtkWidget *entry_user_id = signin_widgets.entry_userid = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_user_id), "Введите UserID...");
    gtk_widget_set_hexpand(entry_user_id, TRUE);
    gtk_box_pack_start(GTK_BOX(horizontal1), entry_user_id, TRUE, TRUE, 0);
    g_signal_connect(entry_user_id, "insert-text", G_CALLBACK(on_insert_text), NULL);
    g_signal_connect(entry_user_id, "activate", G_CALLBACK(on_signin_click), entry_user_id);

    if(app_settings.user_info.user_id != -1)
    {
        int s = snprintf(NULL, 0, "%ld", app_settings.user_info.user_id) + 1;
        char *buf = (char *) malloc(s);
        snprintf(buf, s, "%ld", app_settings.user_info.user_id);
        gtk_entry_set_text(GTK_ENTRY(entry_user_id), buf);
        free(buf);
    }

    GtkWidget *send_button = signin_widgets.button_send = gtk_button_new();
    icon = gtk_image_new_from_icon_name("contact-new", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_label(GTK_BUTTON(send_button), "Войти");
    gtk_button_set_image(GTK_BUTTON(send_button), icon);
    g_signal_connect(send_button, "clicked", G_CALLBACK(on_signin_click), NULL);
    gtk_box_pack_start(GTK_BOX(horizontal1), send_button, FALSE, FALSE, 0);

    // Поля Регистраций-----------------------------------------------------------------------------------
    vertical = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(signup_box), vertical, TRUE, TRUE, 0);

    horizontal0 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(vertical), horizontal0, FALSE, TRUE, 0);
    icon = gtk_image_new_from_icon_name("dialog-warning-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
    label_info = gtk_label_new("Введите свой никнейм.\n"
                               "Принудительно используйте латинский алфавит,\n"
                               "длина ника не более 16 и не менее 3.");
    gtk_box_pack_start(GTK_BOX(horizontal0), icon, TRUE, TRUE, 50);
    gtk_box_pack_end(GTK_BOX(horizontal0), label_info, TRUE, TRUE, 0);

    image_loading_state = signup_widgets.image_loading = gtk_image_new_from_icon_name("content-loading-symbolic", GTK_ICON_SIZE_DIALOG);
    gtk_box_pack_start(GTK_BOX(vertical), image_loading_state, TRUE, TRUE, 0);

    horizontal1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_end(GTK_BOX(vertical), horizontal1, FALSE, TRUE, 0);

    GtkWidget *entry_username = signup_widgets.entry_username = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry_username), TEA_MAXLEN_USERNAME);
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_username), "Введите никнейм...");
    gtk_widget_set_hexpand(entry_username, TRUE);
    gtk_box_pack_start(GTK_BOX(horizontal1), entry_username, TRUE, TRUE, 0);
    g_signal_connect(entry_username, "activate", G_CALLBACK(on_signup_click), entry_username);

    // set icon for button
    GtkWidget *button_genick = signup_widgets.button_generate = gtk_button_new();
    icon = gtk_image_new_from_icon_name("system-reboot-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(button_genick), icon);
    g_signal_connect(button_genick, "clicked", G_CALLBACK(on_click_generate_nick), entry_username);
    gtk_box_pack_start(GTK_BOX(horizontal1), button_genick, FALSE, FALSE, 0);

    send_button = signup_widgets.button_send = gtk_button_new();
    gtk_button_set_label(GTK_BUTTON(send_button), "Зарегистрировать");
    icon = gtk_image_new_from_icon_name("contact-new", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(send_button), icon);
    g_signal_connect(send_button, "clicked", G_CALLBACK(on_signup_click), NULL);
    gtk_box_pack_start(GTK_BOX(horizontal1), send_button, FALSE, FALSE, 0);

    signin_ui_lock(FALSE);
    signup_ui_lock(FALSE);
    return notebook;
}
