#include "SampleRepository.h"
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include "nlohmann/json.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

SampleRepository::SampleRepository(const std::string& dataDir) : dataDir_(dataDir) {}

void SampleRepository::ensureDataDir() const {
    fs::create_directories(dataDir_);
}

void SampleRepository::Load() {
    ensureDataDir();
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

void SampleRepository::Save() {
    ensureDataDir();
    json arr = json::array();
    for (const auto& s : samples_)
        arr.push_back(s.toJson());
    std::ofstream f(dataDir_ + "/samples.json");
    f << json{{"nextId", nextId_}, {"samples", arr}}.dump(2);
}

std::string SampleRepository::GenerateId() {
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << nextId_++;
    return oss.str();
}

const std::vector<SampleData>& SampleRepository::GetAll() const { return samples_; }

std::optional<SampleData> SampleRepository::FindById(const std::string& id) const {
    for (const auto& s : samples_)
        if (s.id == id) return s;
    return std::nullopt;
}

std::vector<SampleData> SampleRepository::FindByName(const std::string& keyword) const {
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

bool SampleRepository::Exists(const std::string& id) const {
    return FindById(id).has_value();
}

void SampleRepository::Add(const SampleData& sample) {
    samples_.push_back(sample);
    Save();
}

bool SampleRepository::UpdateStock(const std::string& id, int delta) {
    for (auto& s : samples_) {
        if (s.id == id) {
            s.stock += delta;
            Save();
            return true;
        }
    }
    return false;
}

int SampleRepository::GetTotalStock() const {
    int total = 0;
    for (const auto& s : samples_) total += s.stock;
    return total;
}
