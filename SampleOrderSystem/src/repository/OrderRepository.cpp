#include "repository/OrderRepository.h"
#include <fstream>
#include <filesystem>
#include "nlohmann/json.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

OrderRepository::OrderRepository(const std::string& dataDir) : dataDir_(dataDir) {}

void OrderRepository::ensureDataDir() const {
    fs::create_directories(dataDir_);
}

void OrderRepository::Load() {
    ensureDataDir();
    std::string path = dataDir_ + "/orders.json";
    if (!fs::exists(path)) return;
    std::ifstream f(path);
    if (!f.is_open()) return;
    json j;
    try {
        f >> j;
        nextId_ = j.value("nextId", 1);
        orders_.clear();
        for (const auto& item : j.value("orders", json::array()))
            orders_.push_back(OrderData::fromJson(item));
    } catch (...) {}
}

void OrderRepository::Save() {
    ensureDataDir();
    json arr = json::array();
    for (const auto& o : orders_)
        arr.push_back(o.toJson());
    std::ofstream f(dataDir_ + "/orders.json");
    f << json{{"nextId", nextId_}, {"orders", arr}}.dump(2);
}

const std::vector<OrderData>& OrderRepository::GetAll() const { return orders_; }

std::vector<OrderData> OrderRepository::GetByStatus(OrderStatus status) const {
    std::vector<OrderData> result;
    for (const auto& o : orders_)
        if (o.status == status) result.push_back(o);
    return result;
}

std::optional<OrderData> OrderRepository::FindById(int id) const {
    for (const auto& o : orders_)
        if (o.id == id) return o;
    return std::nullopt;
}

void OrderRepository::Add(const OrderData& order) {
    orders_.push_back(order);
    Save();
}

void OrderRepository::Update(const OrderData& order) {
    for (auto& o : orders_) {
        if (o.id == order.id) {
            o = order;
            Save();
            return;
        }
    }
}

bool OrderRepository::Release(int orderId) {
    for (auto& o : orders_) {
        if (o.id == orderId) {
            if (o.status != OrderStatus::Confirmed) return false;
            o.status = OrderStatus::Release;
            Save();
            return true;
        }
    }
    return false;
}

int OrderRepository::NextId() {
    return nextId_++;
}

OrderStatusSummary OrderRepository::GetStatusSummary() const {
    OrderStatusSummary summary;
    for (const auto& o : orders_) {
        switch (o.status) {
        case OrderStatus::Reserved:  summary.reserved++;  break;
        case OrderStatus::Producing: summary.producing++; break;
        case OrderStatus::Confirmed: summary.confirmed++; break;
        case OrderStatus::Release:   summary.release++;   break;
        default: break; // Rejected 제외
        }
    }
    return summary;
}
