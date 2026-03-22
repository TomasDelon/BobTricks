#include "app/Application.h"

int main()
{
    Application app;
    if (!app.init())
        return 1;
    return app.run();
}
