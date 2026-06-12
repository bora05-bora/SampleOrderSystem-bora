#include "ProductionController.h"
#include "Color.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <algorithm>

// ── 시간 헬퍼 ────────────────────────────────────────────

std::string ProductionController::nowStr() {
    std::time_t t = std::time(nullptr);
    struct tm ts = {};
    localtime_s(&ts, &t);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &ts);
    return buf;
}

// "YYYY-MM-DD HH:MM:SS" 또는 "YYYY-MM-DD HH:MM" 형식을 파싱해 경과 분 반환
double ProductionController::elapsedMinutes(const std::string& startedAt,
                                             const std::string& now) {
    if (startedAt.empty()) return 0.0;

    auto parse = [](const std::string& s) -> std::time_t {
        int Y = 0, M = 0, D = 0, h = 0, m = 0, sec = 0;
        sscanf_s(s.c_str(), "%d-%d-%d %d:%d:%d", &Y, &M, &D, &h, &m, &sec);
        struct tm tm = {};
        tm.tm_year = Y - 1900; tm.tm_mon = M - 1; tm.tm_mday = D;
        tm.tm_hour = h;        tm.tm_min = m;      tm.tm_sec  = sec;
        tm.tm_isdst = -1;
        return std::mktime(&tm);
    };

    double diff = std::difftime(parse(now), parse(startedAt));
    return diff / 60.0;
}

// dt 에 minutes 분을 더한 "YYYY-MM-DD HH:MM" 문자열 반환
std::string ProductionController::addMinutes(const std::string& dt, double minutes) {
    int Y = 0, M = 0, D = 0, h = 0, m = 0, sec = 0;
    sscanf_s(dt.c_str(), "%d-%d-%d %d:%d:%d", &Y, &M, &D, &h, &m, &sec);
    struct tm tm = {};
    tm.tm_year = Y - 1900; tm.tm_mon = M - 1; tm.tm_mday = D;
    tm.tm_hour = h;        tm.tm_min = m;      tm.tm_sec  = sec;
    tm.tm_isdst = -1;
    std::time_t t = std::mktime(&tm);
    t += static_cast<std::time_t>(minutes * 60.0);
    struct tm result = {};
    localtime_s(&result, &t);
    char buf[20];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", &result);
    return buf;
}

// ── 생성자 ───────────────────────────────────────────────

ProductionController::ProductionController(DataStore& dataStore)
    : dataStore_(dataStore) {}

// ── 생산 완료 자동 처리 ───────────────────────────────────

bool ProductionController::tryAutoComplete() {
    const auto& queue = dataStore_.GetProductionQueue();
    if (queue.empty()) return false;

    const ProductionJob& cur = queue.front();
    if (cur.startedAt.empty()) return false;

    std::string now = nowStr();
    double elapsed = elapsedMinutes(cur.startedAt, now);
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
    std::cout << "============================================================\n";
    std::cout << " [4] 생산라인 조회   FIFO 방식\n";
    std::cout << "============================================================\n";
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
    std::string now = nowStr();
    double elapsed  = elapsedMinutes(cur.startedAt, now);
    double pct      = std::min(100.0, cur.totalTime > 0.0
                                      ? elapsed / cur.totalTime * 100.0
                                      : 0.0);
    std::string completionAt = addMinutes(cur.startedAt, cur.totalTime);

    // 대기 항목 예상 완료 시각 계산 (누적)
    std::vector<std::string> waitingTimes;
    std::string prev = completionAt;
    for (size_t i = 1; i < queue.size(); i++) {
        prev = addMinutes(prev, queue[i].totalTime);
        waitingTimes.push_back(prev);
    }

    view_.ShowRunning(queue, pct, completionAt, waitingTimes);
    view_.InputBack();
}
