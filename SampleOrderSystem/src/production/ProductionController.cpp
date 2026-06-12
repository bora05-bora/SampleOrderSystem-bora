#include "production/ProductionController.h"
#include "core/Color.h"
#include "core/UI.h"
#include "core/DateTimeUtils.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>

// ── 생성자 ───────────────────────────────────────────────

ProductionController::ProductionController(DataStore& dataStore)
    : dataStore_(dataStore) {}

// ── 생산 완료 자동 처리 ───────────────────────────────────

bool ProductionController::tryAutoComplete() {
    const auto& queue = dataStore_.GetProductionQueue();
    if (queue.empty()) return false;

    const ProductionJob& cur = queue.front();
    if (cur.startedAt.empty()) return false;

    std::string now = DateTimeUtils::nowDateTime();
    double elapsed = DateTimeUtils::elapsedMinutes(cur.startedAt, now);
    if (elapsed < cur.totalTime) return false;

    // 완료 처리
    ProductionJob job = cur;

    auto sample  = dataStore_.FindSampleById(job.sampleId);
    int oldStock = sample ? sample->stock : 0;
    dataStore_.UpdateSampleStock(job.sampleId, job.actualQty);
    int newStock = oldStock + job.actualQty;

    ProductionJob dummy;
    dataStore_.PopProductionJob(dummy);

    // 다음 작업이 있으면 즉시 생산 시작
    const auto& remaining = dataStore_.GetProductionQueue();
    bool hasNext  = !remaining.empty();
    std::string nextName = hasNext ? remaining.front().sampleName : "";
    if (hasNext) dataStore_.SetFrontStartedAt(now);

    // 주문 상태 갱신
    auto order = dataStore_.FindOrderById(job.orderId);
    if (order) {
        OrderData updated = *order;
        updated.status = OrderStatus::Confirmed;
        dataStore_.UpdateOrder(updated);
    }

    view_.ShowAutoComplete(job, oldStock, newStock, hasNext, nextName);
    return true;
}

// ── Run ─────────────────────────────────────────────────

void ProductionController::Run() {
    std::cout << "\n";
    Color::set(Color::CYAN);
    std::cout << UI::SEP_THICK << "\n";
    std::cout << " [4] 생산라인 조회   FIFO 방식\n";
    std::cout << UI::SEP_THICK << "\n";
    Color::reset();

    // 완료된 작업 자동 처리 (프로그램 재시작 후 경과 시간 반영)
    while (tryAutoComplete()) {}

    const auto& queue = dataStore_.GetProductionQueue();

    // 라인 상태 표시
    std::cout << " 생산라인 상태  ";
    if (queue.empty()) {
        Color::set(Color::GRAY);
        std::cout << "EMPTY\n";
    } else {
        Color::set(Color::GREEN);
        std::cout << "RUNNING\n";
    }
    Color::reset();

    if (queue.empty()) {
        view_.ShowEmpty();
        view_.InputBack();
        return;
    }

    // 현재 작업 진행도 계산
    const ProductionJob& cur = queue.front();
    std::string now = DateTimeUtils::nowDateTime();
    double elapsed  = DateTimeUtils::elapsedMinutes(cur.startedAt, now);
    double pct      = std::min(100.0, cur.totalTime > 0.0
                                      ? elapsed / cur.totalTime * 100.0
                                      : 0.0);
    std::string completionAt = DateTimeUtils::addMinutes(cur.startedAt, cur.totalTime);

    // 대기 항목 예상 완료 시각 계산 (누적)
    std::vector<std::string> waitingTimes;
    std::string prev = completionAt;
    for (size_t i = 1; i < queue.size(); i++) {
        prev = DateTimeUtils::addMinutes(prev, queue[i].totalTime);
        waitingTimes.push_back(prev);
    }

    view_.ShowRunning(queue, pct, completionAt, waitingTimes);
    view_.InputBack();
}
