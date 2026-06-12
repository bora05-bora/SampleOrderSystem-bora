#pragma once
#include <vector>
#include <string>
#include "Models.h"

class ReleaseView {
public:
    void ShowConfirmedList(const std::vector<OrderData>& orders) const;
    void ShowReleaseResult(const OrderData& order) const;
    void ShowError(const std::string& msg) const;
    int  InputSelection(int max) const;
};
