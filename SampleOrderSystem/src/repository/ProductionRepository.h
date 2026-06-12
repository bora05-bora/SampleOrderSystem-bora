#pragma once
#include <vector>
#include <string>
#include "core/Models.h"

class ProductionRepository {
public:
    explicit ProductionRepository(const std::string& dataDir);
    void Load();
    void Save();

    const std::vector<ProductionJob>& GetQueue() const;
    void                              Enqueue(const ProductionJob& job);
    bool                              Dequeue(ProductionJob& out);
    void                              SetFrontStartedAt(const std::string& dt);
    int                               GetQueuedQuantityForSample(const std::string& sampleId) const;

private:
    std::string                dataDir_;
    std::vector<ProductionJob> queue_;
    void ensureDataDir() const;
};
