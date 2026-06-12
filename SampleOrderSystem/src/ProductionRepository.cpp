#include "ProductionRepository.h"
#include <fstream>
#include <filesystem>
#include "nlohmann/json.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

ProductionRepository::ProductionRepository(const std::string& dataDir) : dataDir_(dataDir) {}

void ProductionRepository::ensureDataDir() const {
    fs::create_directories(dataDir_);
}

void ProductionRepository::Load() {
    ensureDataDir();
    std::string path = dataDir_ + "/production.json";
    if (!fs::exists(path)) return;
    std::ifstream f(path);
    if (!f.is_open()) return;
    json j;
    try {
        f >> j;
        queue_.clear();
        for (const auto& item : j.value("queue", json::array()))
            queue_.push_back(ProductionJob::fromJson(item));
    } catch (...) {}
}

void ProductionRepository::Save() {
    ensureDataDir();
    json arr = json::array();
    for (const auto& p : queue_)
        arr.push_back(p.toJson());
    std::ofstream f(dataDir_ + "/production.json");
    f << json{{"queue", arr}}.dump(2);
}

const std::vector<ProductionJob>& ProductionRepository::GetQueue() const { return queue_; }

void ProductionRepository::Enqueue(const ProductionJob& job) {
    queue_.push_back(job);
    Save();
}

bool ProductionRepository::Dequeue(ProductionJob& out) {
    if (queue_.empty()) return false;
    out = queue_.front();
    queue_.erase(queue_.begin());
    Save();
    return true;
}

void ProductionRepository::SetFrontStartedAt(const std::string& dt) {
    if (queue_.empty()) return;
    queue_.front().startedAt = dt;
    Save();
}

int ProductionRepository::GetQueuedQuantityForSample(const std::string& sampleId) const {
    int total = 0;
    for (const auto& job : queue_)
        if (job.sampleId == sampleId)
            total += job.quantity;
    return total;
}
