#pragma once
#include <string>
#include <vector>
#include "Models.h"

class OrderView {
public:
    void ShowReservedList(const std::vector<OrderData>& orders) const;
    void ShowOrderConfirm(const std::string& sampleId,
                          const std::string& sampleName,
                          const std::string& customer,
                          int quantity) const;
    void ShowOrderPlaced(const OrderData& order) const;
    void ShowStockInfo(const std::string& sampleName,
                       int stock, int quantity) const;
    void ShowApprovalResult(const OrderData& order, OrderStatus before) const;
    void ShowError(const std::string& msg) const;

    std::string InputSampleId() const;
    std::string InputCustomer() const;
    int         InputQuantity() const;
    char        InputYN() const;
    int         InputSelection(int max) const;
};
