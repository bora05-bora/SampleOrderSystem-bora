#pragma once
#include <string>
#include <vector>
#include "core/Models.h"

class SampleView {
public:
    void ShowMenu() const;
    void ShowSampleList(const std::vector<SampleData>& samples) const;
    void ShowSearchResult(const std::vector<SampleData>& results, const std::string& keyword) const;
    void ShowRegistered(const SampleData& sample) const;
    void ShowError(const std::string& msg) const;
    void ShowSuccess(const std::string& msg) const;

    std::string InputSampleId() const;
    std::string InputName() const;
    double      InputProductionTime() const;
    double      InputYield() const;
    std::string InputSearchKeyword() const;
};
