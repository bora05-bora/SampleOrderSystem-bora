#pragma once
#include "DataStore.h"
#include "MonitorView.h"

class MonitorController {
public:
    explicit MonitorController(DataStore& dataStore);
    void Run();

private:
    DataStore&   dataStore_;
    MonitorView  view_;
};
