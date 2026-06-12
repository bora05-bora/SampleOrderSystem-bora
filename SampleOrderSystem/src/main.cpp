#include <windows.h>
#include <iostream>
#include "DataStore.h"
#include "MainController.h"

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    DataStore store("data");
    store.Load();

    MainController mainCtrl(store);
    mainCtrl.Run();

    return 0;
}
