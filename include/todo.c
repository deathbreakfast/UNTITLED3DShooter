
const int SCREEN_BOARDER = 16;
const int SCREEN_PADDING = 8;
const int SCREENS_VERTICAL = 2;
const int SCREENS_HORIZONTAL = 2;

enum view_perspective {Top, Front, Side, FirstPerson};


// TODO convert this to static
struct viewport
{
    enum view_perspective per;
    int height, width, screen_x, screen_y;
};


/** TODO: Once I finish the rendering engine to the screen, migrate to a virtual screen.
 *   virtual screens can be rendered from multiple view points.
 */
struct viewport * create_views(enum view_perspective* perspectives, int num_perspectives)
{
    struct viewport * views;
    views = malloc(sizeof(struct viewport) * num_perspectives);
    int wide_width = SCREEN_HEIGHT - (SCREEN_BOARDER * 2);
    int slim_width = SCREEN_HEIGHT / 2 - (SCREEN_BOARDER * 2) - (SCREEN_PADDING);
    int short_height = SCREEN_WIDTH / 2 - (SCREEN_BOARDER * 2) - (SCREEN_PADDING);
    for (int i = 0; i < num_perspectives; i++)
    {
        int height = 0;
        int width = 0;
        int screen_x = 0;
        int screen_y = 0;
        if (num_perspectives == 1)
        {
            width = SCREEN_WIDTH;
            height = SCREEN_HEIGHT;
            screen_x = 0;
            screen_y = 0;
        }
        else if (num_perspectives == 2)
        {
            width = wide_width;
            height = short_height;
            screen_x = i + 1 % 2 == 0 ? width + SCREEN_BOARDER + SCREEN_PADDING : SCREEN_BOARDER;
            screen_y = i + 1 > 2 ? height + SCREEN_BOARDER + SCREEN_PADDING : SCREEN_BOARDER;
        }
        else if (num_perspectives == 3)
        {
            height = short_height;
            if (i == 0)
            {
                width = wide_width;
                screen_x = SCREEN_BOARDER;
                screen_y = SCREEN_BOARDER;
            }
            else
            {
                width = slim_width;
                screen_x = i + 1 % 2 == 0 ? width + SCREEN_BOARDER + SCREEN_PADDING : SCREEN_BOARDER;
                screen_y = i + 1 > 2 ? height + SCREEN_BOARDER + SCREEN_PADDING : SCREEN_BOARDER;
            }
        }
        else if (num_perspectives == 4)
        {
            width = slim_width;
            height = short_height;
            screen_x = i + 1 % 2 == 0 ? width + SCREEN_BOARDER + SCREEN_PADDING : SCREEN_BOARDER;
            screen_y = i + 1 > 2 ? height + SCREEN_BOARDER + SCREEN_PADDING : SCREEN_BOARDER;
        }
        
        struct viewport current_view = {perspectives[i], height, width, screen_x, screen_y};
        views[i] = current_view;
    }
    return views;
}
