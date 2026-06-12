#pragma once
#include "DataStore.h"
#include "ReleaseView.h"

class ReleaseController {
public:
    explicit ReleaseController(DataStore& dataStore);
    void Run();

private:
    DataStore&  dataStore_;
    ReleaseView view_;
};
