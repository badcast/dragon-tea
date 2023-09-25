#include "tea.h"

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");

// Check runned on Root
#if __unix__ || __linux__
    if(getuid() == 0)
    {
        printf("Program runned as root. It's maybe damage your system. Closing.");
        return EXIT_FAILURE;
    }
#endif

    bindtextdomain(DRAGON_TEA_TEXTDOMAIN, DRAGON_TEA_LOCALE_DIR);
    textdomain(DRAGON_TEA_TEXTDOMAIN);

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
