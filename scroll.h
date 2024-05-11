/*
 *
 *  https://ramiroblan.co
 *  mail: r@miroblan.co
 *
 */

void scrollbar_update();

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

typedef struct
{
        GLfloat top, bottom;
}
SCROLL_BAR;

extern SCROLL_BAR scroll_bar;
