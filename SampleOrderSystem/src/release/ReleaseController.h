#pragma once
#include "repository/DataStore.h"
#include "release/ReleaseView.h"

class ReleaseController {
public:
    explicit ReleaseController(DataStore& dataStore);
    void Run();

private:
    DataStore&  dataStore_;
    ReleaseView view_;
};
