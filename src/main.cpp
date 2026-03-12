#include "app.hpp"



int main() {
    if (!appInit()) {
        return 1;
    }

    while (appStep()) {
    }
    appShutdown();
    return 0;
}
