#include "tea.h"

const gchar *const authors[2] = {"badcast", NULL};

void show_about_dialog()
{
    // Создание GtkAboutDialog
    GtkAboutDialog *about_dialog = GTK_ABOUT_DIALOG(gtk_about_dialog_new());
    gtk_about_dialog_set_program_name(about_dialog, _("Драконий Чай"));
    gtk_about_dialog_set_version(about_dialog, DRAGON_TEA_VERSION);
    gtk_about_dialog_set_comments(about_dialog, _("Свободный мессенджер для обмена информациями."));
    gtk_about_dialog_set_website(about_dialog, "https://github.com/badcast/dragon-tea");
    gtk_about_dialog_set_website_label(about_dialog, _("Веб-страница"));
    gtk_about_dialog_set_authors(about_dialog, (gchar**)authors);
    gtk_about_dialog_set_license_type(about_dialog, GTK_LICENSE_GPL_3_0);
    gtk_about_dialog_set_logo_icon_name(about_dialog, "mail-message-new-symbolic");

    // Отображение диалогового окна "О программе"
    gtk_widget_show_all(GTK_WIDGET(about_dialog));

    // Ожидание закрытия диалогового окна
    gtk_dialog_run(GTK_DIALOG(about_dialog));
    gtk_widget_destroy(GTK_WIDGET(about_dialog));
}
