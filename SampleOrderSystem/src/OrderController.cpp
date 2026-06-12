#include "OrderController.h"
#include "Color.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>

OrderController::OrderController(DataStore& dataStore)
    : dataStore_(dataStore) {}

// ── 날짜 헬퍼 ────────────────────────────────────────────
std::string OrderController::currentDateStr() {
    std::time_t now = std::time(nullptr);
    struct tm ts = {};
    localtime_s(&ts, &now);
    char buf[16];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", &ts);
    return buf;
}

std::string OrderController::currentDateCompact() {
    std::time_t now = std::time(nullptr);
    struct tm ts = {};
    localtime_s(&ts, &now);
    char buf[12];
    std::strftime(buf, sizeof(buf), "%Y%m%d", &ts);
    return buf;
}

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
    order.orderNo    = "ORD-" + currentDateCompact() + "-" + formatOrderId(id);
    order.sampleId   = sampleId;
    order.sampleName = sample->name;
    order.customer   = customer;
    order.quantity   = quantity;
    order.status     = OrderStatus::Reserved;
    order.date       = currentDateStr();

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

    view_.ShowStockInfo(sample->name, sample->stock, selected.quantity);
    char yn = view_.InputYN();

    OrderData updated = selected;
    OrderStatus before = selected.status;

    if (yn == 'N') {
        updated.status = OrderStatus::Rejected;
    } else {
        if (sample->stock >= selected.quantity) {
            updated.status = OrderStatus::Confirmed;
            dataStore_.UpdateSampleStock(selected.sampleId, -selected.quantity);
        } else {
            updated.status = OrderStatus::Producing;
        }
    }

    dataStore_.UpdateOrder(updated);
    view_.ShowApprovalResult(updated, before);
}
