#pragma once
#include <vector>
#include <string>
#include <optional>
#include "Models.h"

class OrderRepository {
public:
    explicit OrderRepository(const std::string& dataDir);
    void Load();
    void Save();

    const std::vector<OrderData>& GetAll() const;
    std::vector<OrderData>        GetByStatus(OrderStatus status) const;
    std::optional<OrderData>      FindById(int id) const;
    void                          Add(const OrderData& order);
    void                          Update(const OrderData& order);
    bool                          Release(int orderId);
    int                           NextId();
    OrderStatusSummary            GetStatusSummary() const;

private:
    std::string            dataDir_;
    std::vector<OrderData> orders_;
    int                    nextId_ = 1;
    void ensureDataDir() const;
};
