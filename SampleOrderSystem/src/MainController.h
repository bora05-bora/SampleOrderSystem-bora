#pragma once
#include "DataStore.h"
#include "SampleController.h"
#include "OrderController.h"
#include "ProductionController.h"

class MainController {
public:
    explicit MainController(DataStore& dataStore);
    void Run();

private:
    DataStore&            dataStore_;
    SampleController      sampleCtrl_;
    OrderController       orderCtrl_;
    ProductionController  productionCtrl_;

    void showHeader() const;
    void showSummary() const;
    void showMenu() const;
};
