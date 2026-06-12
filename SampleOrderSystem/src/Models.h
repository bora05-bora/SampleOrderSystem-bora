#pragma once
#include <string>
#include "nlohmann/json.hpp"

// ── 주문 상태 ────────────────────────────────────────────
enum class OrderStatus { Reserved, Rejected, Producing, Confirmed, Release };

inline std::string orderStatusToString(OrderStatus s) {
    switch (s) {
    case OrderStatus::Reserved:  return "RESERVED";
    case OrderStatus::Rejected:  return "REJECTED";
    case OrderStatus::Producing: return "PRODUCING";
    case OrderStatus::Confirmed: return "CONFIRMED";
    case OrderStatus::Release:   return "RELEASE";
    default:                     return "UNKNOWN";
    }
}

inline OrderStatus orderStatusFromString(const std::string& s) {
    if (s == "REJECTED"  || s == "Rejected")  return OrderStatus::Rejected;
    if (s == "PRODUCING" || s == "Producing") return OrderStatus::Producing;
    if (s == "CONFIRMED" || s == "Confirmed") return OrderStatus::Confirmed;
    if (s == "RELEASE"   || s == "Release")   return OrderStatus::Release;
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

// ── 생산 작업 ────────────────────────────────────────────
struct ProductionJob {
    int         orderId;
    std::string orderNo;
    std::string sampleId;
    std::string sampleName;
    int         quantity;    // 주문 수량
    int         stock;       // 승인 시점 재고
    int         shortage;    // 부족분 = quantity - stock
    int         actualQty;   // 실 생산량 = ceil(shortage / (yield * 0.9))
    double      totalTime;   // 총 생산시간(분) = productionTime * actualQty
    std::string enqueuedAt;  // "YYYY-MM-DD HH:MM:SS"
    std::string startedAt;   // 생산 시작 시각 (front 항목에만 유효, 대기는 "")

    nlohmann::json toJson() const {
        return {
            {"orderId",    orderId},
            {"orderNo",    orderNo},
            {"sampleId",   sampleId},
            {"sampleName", sampleName},
            {"quantity",   quantity},
            {"stock",      stock},
            {"shortage",   shortage},
            {"actualQty",  actualQty},
            {"totalTime",  totalTime},
            {"enqueuedAt", enqueuedAt},
            {"startedAt",  startedAt}
        };
    }

    static ProductionJob fromJson(const nlohmann::json& j) {
        ProductionJob p;
        p.orderId    = j.at("orderId").get<int>();
        p.orderNo    = j.at("orderNo").get<std::string>();
        p.sampleId   = j.at("sampleId").get<std::string>();
        p.sampleName = j.at("sampleName").get<std::string>();
        p.quantity   = j.value("quantity",  0);
        p.stock      = j.value("stock",     0);
        p.shortage   = j.at("shortage").get<int>();
        p.actualQty  = j.at("actualQty").get<int>();
        p.totalTime  = j.at("totalTime").get<double>();
        p.enqueuedAt = j.at("enqueuedAt").get<std::string>();
        p.startedAt  = j.value("startedAt", "");
        return p;
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
