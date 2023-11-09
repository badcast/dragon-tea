#include "tea.h"

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");

    bindtextdomain(DRAGON_TEA_TEXTDOMAIN, DRAGON_TEA_LOCALE_DIR);
    textdomain(DRAGON_TEA_TEXTDOMAIN);

// Check runned on Root
#if __unix__ || __linux__
    if(getuid() == 0)
    {
        printf(_("The program is running as root. These privileges can harm the system, please be aware of this. Closing."));
        return EXIT_FAILURE;
    }
#endif

    // GUI Init
    gtk_init(&argc, &argv);

    // Run Dragon Tea Messenger
    tea_init();

    // Window Looping
    gtk_main();

    // Free networks
    net_free();

    return 0;
}
