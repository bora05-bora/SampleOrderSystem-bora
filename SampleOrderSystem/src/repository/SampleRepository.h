#pragma once
#include <vector>
#include <string>
#include <optional>
#include "core/Models.h"

class SampleRepository {
public:
    explicit SampleRepository(const std::string& dataDir);
    void Load();
    void Save();

    const std::vector<SampleData>& GetAll() const;
    std::optional<SampleData>      FindById(const std::string& id) const;
    std::vector<SampleData>        FindByName(const std::string& keyword) const;
    bool                           Exists(const std::string& id) const;
    void                           Add(const SampleData& sample);
    bool                           UpdateStock(const std::string& id, int delta);
    std::string                    GenerateId();
    int                            GetTotalStock() const;

private:
    std::string             dataDir_;
    std::vector<SampleData> samples_;
    int                     nextId_ = 1;
    void ensureDataDir() const;
};
