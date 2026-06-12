#include "OrderController.h"
#include "Color.h"
#include "ProductionCalc.h"
#include "DateTimeUtils.h"
#include <iostream>
#include <iomanip>
#include <sstream>

OrderController::OrderController(DataStore& dataStore)
    : dataStore_(dataStore) {}

std::string OrderController::formatOrderId(int id) {
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(4) << id;
    return oss.str();
}

// ── [2] 시료 주문 (접수) ─────────────────────────────────
void OrderController::PlaceOrder() {
    std::cout << "\n";
    Color::set(Color::CYAN);
    std::cout << "============================================================\n";
    std::cout << " [2] 시료 주문\n";
    std::cout << "============================================================\n";
    Color::reset();

    std::string sampleId = view_.InputSampleId();
    auto sample = dataStore_.FindSampleById(sampleId);
    if (!sample) {
        view_.ShowError("등록되지 않은 시료 ID입니다: " + sampleId);
        return;
    }

    std::string customer = view_.InputCustomer();
    int quantity = view_.InputQuantity();

    view_.ShowOrderConfirm(sampleId, sample->name, customer, quantity);
    char yn = view_.InputYN();
    if (yn == 'N') {
        std::cout << "\n 주문이 취소되었습니다.\n";
        return;
    }

    int id = dataStore_.NextOrderId();
    OrderData order;
    order.id         = id;
    order.orderNo    = "ORD-" + DateTimeUtils::nowDateCompact() + "-" + formatOrderId(id);
    order.sampleId   = sampleId;
    order.sampleName = sample->name;
    order.customer   = customer;
    order.quantity   = quantity;
    order.status     = OrderStatus::Reserved;
    order.date       = DateTimeUtils::nowDate();

    dataStore_.AddOrder(order);
    view_.ShowOrderPlaced(order);
}

// ── [3] 주문 승인/거절 ────────────────────────────────────
void OrderController::ProcessApprovals() {
    std::cout << "\n";
    Color::set(Color::CYAN);
    std::cout << "============================================================\n";
    std::cout << " [3] 주문 승인/거절\n";
    std::cout << "============================================================\n";
    Color::reset();

    auto reserved = dataStore_.GetReservedOrders();
    view_.ShowReservedList(reserved);

    if (reserved.empty()) return;

    int sel = view_.InputSelection((int)reserved.size());
    if (sel == 0) return;

    const OrderData& selected = reserved[sel - 1];
    auto sample = dataStore_.FindSampleById(selected.sampleId);
    if (!sample) {
        view_.ShowError("시료 정보를 찾을 수 없습니다.");
        return;
    }

    int queuedQty      = dataStore_.GetQueuedQuantityForSample(selected.sampleId);
    int effectiveStock = ProductionCalc::calcEffectiveStock(sample->stock, queuedQty);
    int shortage       = ProductionCalc::calcShortage(selected.quantity, effectiveStock);
    int actualQty      = ProductionCalc::calcActualQty(shortage, sample->yield);
    double totalTime   = ProductionCalc::calcTotalTime(sample->productionTime, actualQty);

    view_.ShowStockInfo(sample->name, sample->stock, queuedQty,
                        selected.quantity, actualQty, totalTime);
    char yn = view_.InputYN();

    OrderData updated = selected;
    OrderStatus before = selected.status;

    if (yn == 'N') {
        updated.status = OrderStatus::Rejected;
    } else {
        if (effectiveStock >= selected.quantity) {
            updated.status = OrderStatus::Confirmed;
            dataStore_.UpdateSampleStock(selected.sampleId, -selected.quantity);
        } else {
            bool wasEmpty = dataStore_.GetProductionQueue().empty();
            std::string now = DateTimeUtils::nowDateTime();

            ProductionJob job;
            job.orderId    = selected.id;
            job.orderNo    = selected.orderNo;
            job.sampleId   = selected.sampleId;
            job.sampleName = selected.sampleName;
            job.quantity   = selected.quantity;
            job.stock      = effectiveStock;   // 유효 가용 재고 기준
            job.shortage   = shortage;
            job.actualQty  = actualQty;
            job.totalTime  = totalTime;
            job.enqueuedAt = now;
            job.startedAt  = wasEmpty ? now : "";  // EMPTY 상태면 즉시 생산 시작
            dataStore_.AddProductionJob(job);
            updated.status = OrderStatus::Producing;
        }
    }

    dataStore_.UpdateOrder(updated);
    view_.ShowApprovalResult(updated, before);
}
