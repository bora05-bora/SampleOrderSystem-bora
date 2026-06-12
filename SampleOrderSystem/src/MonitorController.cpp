#include "MonitorController.h"
#include "Color.h"
#include <iostream>

MonitorController::MonitorController(DataStore& dataStore)
    : dataStore_(dataStore) {}

void MonitorController::Run() {
    std::cout << "\n";
    Color::set(Color::CYAN);
    std::cout << "============================================================\n";
    std::cout << " [5] 모니터링\n";
    std::cout << "============================================================\n";
    Color::reset();

    auto summary = dataStore_.GetOrderStatusSummary();
    const auto& samples = dataStore_.GetSamples();

    view_.ShowMonitor(summary, samples, dataStore_);
    view_.InputBack();
}
