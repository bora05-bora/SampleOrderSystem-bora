#pragma once
#include <string>
#include "nlohmann/json.hpp"

// ── 주문 상태 ────────────────────────────────────────────
enum class OrderStatus { Reserved, Rejected, Producing, Confirmed, Release };

inline std::string orderStatusToString(OrderStatus s) {
    switch (s) {
    case OrderStatus::Reserved:  return "Reserved";
    case OrderStatus::Rejected:  return "Rejected";
    case OrderStatus::Producing: return "Producing";
    case OrderStatus::Confirmed: return "Confirmed";
    case OrderStatus::Release:   return "Release";
    default:                     return "Unknown";
    }
}

inline OrderStatus orderStatusFromString(const std::string& s) {
    if (s == "Rejected")  return OrderStatus::Rejected;
    if (s == "Producing") return OrderStatus::Producing;
    if (s == "Confirmed") return OrderStatus::Confirmed;
    if (s == "Release")   return OrderStatus::Release;
    return OrderStatus::Reserved;
}

// ── 주문 데이터 ──────────────────────────────────────────
struct OrderData {
    int         id;
    std::string orderNo;     // "ORD-20260612-0001"
    std::string sampleId;    // SampleData.id 참조
    std::string sampleName;  // 조회 편의를 위해 비정규화 저장
    std::string customer;
    int         quantity;
    OrderStatus status;
    std::string date;        // "YYYY-MM-DD"

    nlohmann::json toJson() const {
        return {
            {"id",         id},
            {"orderNo",    orderNo},
            {"sampleId",   sampleId},
            {"sampleName", sampleName},
            {"customer",   customer},
            {"quantity",   quantity},
            {"status",     orderStatusToString(status)},
            {"date",       date}
        };
    }

    static OrderData fromJson(const nlohmann::json& j) {
        OrderData o;
        o.id         = j.at("id").get<int>();
        o.orderNo    = j.at("orderNo").get<std::string>();
        o.sampleId   = j.at("sampleId").get<std::string>();
        o.sampleName = j.at("sampleName").get<std::string>();
        o.customer   = j.at("customer").get<std::string>();
        o.quantity   = j.at("quantity").get<int>();
        o.status     = orderStatusFromString(j.at("status").get<std::string>());
        o.date       = j.at("date").get<std::string>();
        return o;
    }
};

// ── 시료 데이터 ──────────────────────────────────────────
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
