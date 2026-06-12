#pragma once
#include "DataStore.h"
#include "SampleView.h"

class SampleController {
public:
    explicit SampleController(DataStore& dataStore);
    void Run();

private:
    DataStore&  dataStore_;
    SampleView  view_;

    void registerSample();
    void listSamples();
    void searchSamples();
};
