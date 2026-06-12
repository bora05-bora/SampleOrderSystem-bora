#include "MonitorController.h"
#include "Color.h"
#include "UI.h"
#include <iostream>

MonitorController::MonitorController(DataStore& dataStore)
    : dataStore_(dataStore) {}

void MonitorController::Run() {
    std::cout << "\n";
    Color::set(Color::CYAN);
    std::cout << UI::SEP_THICK << "\n";
    std::cout << " [5] 모니터링\n";
    std::cout << UI::SEP_THICK << "\n";
    Color::reset();

    auto summary = dataStore_.GetOrderStatusSummary();
    const auto& samples = dataStore_.GetSamples();

    view_.ShowMonitor(summary, samples, dataStore_);
    view_.InputBack();
}
