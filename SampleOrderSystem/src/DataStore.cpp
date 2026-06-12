#include "DataStore.h"
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include "nlohmann/json.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

DataStore::DataStore(const std::string& dataDir) : dataDir_(dataDir) {}

void DataStore::Load() {
    ensureDataDir();
    loadSamples();
    loadOrders();
}

void DataStore::ensureDataDir() const {
    fs::create_directories(dataDir_);
}

void DataStore::loadSamples() {
    std::string path = dataDir_ + "/samples.json";
    if (!fs::exists(path)) return;

    std::ifstream f(path);
    if (!f.is_open()) return;

    json j;
    try {
        f >> j;
        nextId_ = j.value("nextId", 1);
        samples_.clear();
        for (const auto& item : j.value("samples", json::array()))
            samples_.push_back(SampleData::fromJson(item));
    } catch (...) {}
}

void DataStore::SaveSamples() {
    ensureDataDir();
    json arr = json::array();
    for (const auto& s : samples_)
        arr.push_back(s.toJson());

    std::ofstream f(dataDir_ + "/samples.json");
    f << json{{"nextId", nextId_}, {"samples", arr}}.dump(2);
}

std::string DataStore::GenerateSampleId() {
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << nextId_++;
    return oss.str();
}

const std::vector<SampleData>& DataStore::GetSamples() const {
    return samples_;
}

std::optional<SampleData> DataStore::FindSampleById(const std::string& id) const {
    for (const auto& s : samples_)
        if (s.id == id) return s;
    return std::nullopt;
}

std::vector<SampleData> DataStore::FindSamplesByName(const std::string& keyword) const {
    std::vector<SampleData> result;
    std::string lowerKw = keyword;
    std::transform(lowerKw.begin(), lowerKw.end(), lowerKw.begin(), ::tolower);

    for (const auto& s : samples_) {
        std::string lowerName = s.name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        if (lowerName.find(lowerKw) != std::string::npos)
            result.push_back(s);
    }
    return result;
}

bool DataStore::ExistsSampleId(const std::string& id) const {
    return FindSampleById(id).has_value();
}

void DataStore::AddSample(const SampleData& sample) {
    samples_.push_back(sample);
    SaveSamples();
}

bool DataStore::UpdateSampleStock(const std::string& sampleId, int delta) {
    for (auto& s : samples_) {
        if (s.id == sampleId) {
            s.stock += delta;
            SaveSamples();
            return true;
        }
    }
    return false;
}

int DataStore::GetTotalStock() const {
    int total = 0;
    for (const auto& s : samples_) total += s.stock;
    return total;
}

// ── Orders ───────────────────────────────────────────────

void DataStore::loadOrders() {
    std::string path = dataDir_ + "/orders.json";
    if (!fs::exists(path)) return;
    std::ifstream f(path);
    if (!f.is_open()) return;
    json j;
    try {
        f >> j;
        nextOrderId_ = j.value("nextId", 1);
        orders_.clear();
        for (const auto& item : j.value("orders", json::array()))
            orders_.push_back(OrderData::fromJson(item));
    } catch (...) {}
}

void DataStore::SaveOrders() {
    ensureDataDir();
    json arr = json::array();
    for (const auto& o : orders_)
        arr.push_back(o.toJson());
    std::ofstream f(dataDir_ + "/orders.json");
    f << json{{"nextId", nextOrderId_}, {"orders", arr}}.dump(2);
}

const std::vector<OrderData>& DataStore::GetOrders() const { return orders_; }

std::vector<OrderData> DataStore::GetReservedOrders() const {
    std::vector<OrderData> result;
    for (const auto& o : orders_)
        if (o.status == OrderStatus::Reserved)
            result.push_back(o);
    return result;
}

void DataStore::AddOrder(const OrderData& order) {
    orders_.push_back(order);
    SaveOrders();
}

void DataStore::UpdateOrder(const OrderData& order) {
    for (auto& o : orders_) {
        if (o.id == order.id) {
            o = order;
            SaveOrders();
            return;
        }
    }
}

int DataStore::NextOrderId() {
    return nextOrderId_++;
}
