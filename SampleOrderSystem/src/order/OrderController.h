#pragma once
#include "repository/DataStore.h"
#include "order/OrderView.h"

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
