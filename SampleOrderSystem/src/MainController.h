#pragma once
#include "DataStore.h"
#include "SampleController.h"

class MainController {
public:
    explicit MainController(DataStore& dataStore);
    void Run();

private:
    DataStore&        dataStore_;
    SampleController  sampleCtrl_;

    void showHeader() const;
    void showSummary() const;
    void showMenu() const;
};
