#pragma once
#include "repository/DataStore.h"
#include "sample/SampleController.h"
#include "order/OrderController.h"
#include "production/ProductionController.h"
#include "monitor/MonitorController.h"
#include "release/ReleaseController.h"

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
