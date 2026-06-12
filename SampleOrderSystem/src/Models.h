#pragma once
#include <string>
#include "nlohmann/json.hpp"

struct SampleData {
    std::string id;            // "S-001"
    std::string name;          // "실리콘 웨이퍼-8인치"
    double      productionTime; // min/ea
    double      yield;          // 0.0 초과 ~ 1.0
    int         stock;          // ea

    nlohmann::json toJson() const {
        return {
            {"id",             id},
            {"name",           name},
            {"productionTime", productionTime},
            {"yield",          yield},
            {"stock",          stock}
        };
    }

    static SampleData fromJson(const nlohmann::json& j) {
        SampleData s;
        s.id             = j.at("id").get<std::string>();
        s.name           = j.at("name").get<std::string>();
        s.productionTime = j.at("productionTime").get<double>();
        s.yield          = j.at("yield").get<double>();
        s.stock          = j.at("stock").get<int>();
        return s;
    }
};
