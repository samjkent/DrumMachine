#define MAX_TITLE 32
#define MAX_SUBMENUS 16
 
struct menu_s
{
    char title[MAX_TITLE];
    void (*command)();
    short num_submenus;
    struct menu_s *submenu[MAX_SUBMENUS];
};

struct menu_s menu = {"Menu", NULL, 2, {
                            {"Files", NULL, 0, NULL},
                            {"Volume", NULL, 0,
                                    {
                                        {"Attack", NULL, 0, NULL},
                                        {"Decay", NULL, 0, NULL},
                                        {"Sustain", NULL, 0, NULL},
                                        {"Release", NULL, 0, NULL},
                                    }
                            },
                        }
                    };

struct menu_s *_menu;
