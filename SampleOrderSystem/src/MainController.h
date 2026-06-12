#pragma once
#include "DataStore.h"
#include "SampleController.h"
#include "OrderController.h"
#include "ProductionController.h"
#include "MonitorController.h"
#include "ReleaseController.h"

class MainController {
public:
    explicit MainController(DataStore& dataStore);
    void Run();

private:
    DataStore&            dataStore_;
    SampleController      sampleCtrl_;
    OrderController       orderCtrl_;
    ProductionController  productionCtrl_;
    MonitorController     monitorCtrl_;
    ReleaseController     releaseCtrl_;

    void showHeader() const;
    void showSummary() const;
    void showMenu() const;
};
