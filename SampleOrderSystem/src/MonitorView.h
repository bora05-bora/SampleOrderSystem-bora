#pragma once
#include <vector>
#include "Models.h"
#include "DataStore.h"

class MonitorView {
public:
    void ShowMonitor(const OrderStatusSummary& summary,
                     const std::vector<SampleData>& samples,
                     const DataStore& ds) const;
    void InputBack() const;
};
