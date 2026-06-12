#pragma once
#include <vector>
#include <string>
#include <optional>
#include "Models.h"

class DataStore {
public:
    explicit DataStore(const std::string& dataDir);

    void Load();

    // Samples
    const std::vector<SampleData>& GetSamples() const;
    std::optional<SampleData>      FindSampleById(const std::string& id) const;
    std::vector<SampleData>        FindSamplesByName(const std::string& keyword) const;
    bool                           ExistsSampleId(const std::string& id) const;
    void                           AddSample(const SampleData& sample);
    void                           SaveSamples();
    std::string                    GenerateSampleId();

    int GetTotalStock() const;

private:
    std::string              dataDir_;
    std::vector<SampleData>  samples_;
    int                      nextId_ = 1;

    void loadSamples();
    void ensureDataDir() const;
};
