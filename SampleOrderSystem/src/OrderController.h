#pragma once
#include "DataStore.h"
#include "OrderView.h"

class OrderController {
public:
    explicit OrderController(DataStore& dataStore);
    void PlaceOrder();
    void ProcessApprovals();

private:
    DataStore&  dataStore_;
    OrderView   view_;

    static std::string formatOrderId(int id);
};
