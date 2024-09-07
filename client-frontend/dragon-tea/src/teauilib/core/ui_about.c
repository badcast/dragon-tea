#include "tea.h"

const gchar * authors[2] = {"badcast", NULL};

void show_about_dialog()
{
    // Create About dialog
    GtkAboutDialog *about_dialog = GTK_ABOUT_DIALOG(gtk_about_dialog_new());
    gtk_about_dialog_set_program_name(about_dialog, _("Dragon Tea"));
    gtk_about_dialog_set_version(about_dialog, DRAGON_TEA_VERSION);
    gtk_about_dialog_set_comments(about_dialog, _("Free messenger for transmission"));
    gtk_about_dialog_set_website(about_dialog, "https://github.com/badcast/dragon-tea");
    gtk_about_dialog_set_website_label(about_dialog, _("WEB-Page"));
    gtk_about_dialog_set_authors(about_dialog, authors);
    gtk_about_dialog_set_copyright(about_dialog, *authors);
    gtk_about_dialog_set_translator_credits(about_dialog, "badcast");
    gtk_about_dialog_set_license_type(about_dialog, GTK_LICENSE_GPL_3_0);
    gtk_about_dialog_set_logo_icon_name(about_dialog, "mail-message-new-symbolic");
    // Show
    gtk_widget_show_all(GTK_WIDGET(about_dialog));

    // Wait for close Dialog
    gtk_dialog_run(GTK_DIALOG(about_dialog));
    gtk_widget_destroy(GTK_WIDGET(about_dialog));
}
