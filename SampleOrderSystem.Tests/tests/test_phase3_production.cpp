#include "doctest/doctest.h"
#include "repository/DataStore.h"
#include "core/ProductionCalc.h"
#include <filesystem>

namespace fs = std::filesystem;

// ── 픽스처 ───────────────────────────────────────────────
struct Phase3Fixture {
    std::string dir = "./test_phase3_tmp";
    DataStore   ds;

    Phase3Fixture() : ds(dir) {
        fs::remove_all(dir);
        ds.Load();
    }
    ~Phase3Fixture() { fs::remove_all(dir); }

    SampleData makeSample(const std::string& id, int stock,
                          double yield = 0.92, double prodTime = 0.8) {
        SampleData s;
        s.id             = id;
        s.name           = "Sample-" + id;
        s.productionTime = prodTime;
        s.yield          = yield;
        s.stock          = stock;
        return s;
    }

    OrderData makeOrder(int id, const std::string& sampleId,
                        int qty, OrderStatus status = OrderStatus::Producing) {
        OrderData o;
        o.id             = id;
        o.orderNo        = "ORD-TEST-" + std::to_string(id);
        o.sampleId       = sampleId;
        o.sampleName     = "Sample-" + sampleId;
        o.customer       = "TestCustomer";
        o.quantity       = qty;
        o.status         = status;
        o.date           = "2026-06-12";
        o.producingBased = true;
        return o;
    }

    ProductionJob makeJob(int orderId, const std::string& sampleId,
                          int qty, int stock, int shortage, int actualQty,
                          double totalTime, const std::string& startedAt = "") {
        ProductionJob j;
        j.orderId    = orderId;
        j.orderNo    = "ORD-TEST-" + std::to_string(orderId);
        j.sampleId   = sampleId;
        j.sampleName = "Sample-" + sampleId;
        j.quantity   = qty;
        j.stock      = stock;
        j.shortage   = shortage;
        j.actualQty  = actualQty;
        j.totalTime  = totalTime;
        j.enqueuedAt = "2026-06-12 10:00:00";
        j.startedAt  = startedAt;
        return j;
    }
};

// ── [PHASE3 §5] 실 생산량 계산 — 문서 예시 A ───────────────
// 주문 200ea, 재고 30ea, 큐 대기 없음, 수율 0.92, 생산시간 0.8 min/ea

TEST_CASE("Phase3 예시A: 유효 가용 재고 = 재고 - 대기 없음") {
    int effectiveStock = ProductionCalc::calcEffectiveStock(30, 0);
    CHECK(effectiveStock == 30);
}

TEST_CASE("Phase3 예시A: 부족분 = 주문 수량 - 유효 가용 재고") {
    int shortage = ProductionCalc::calcShortage(200, 30);
    CHECK(shortage == 170);
}

TEST_CASE("Phase3 예시A: 실 생산량 = ceil(170 / (0.92 * 0.9)) = 206") {
    int actualQty = ProductionCalc::calcActualQty(170, 0.92);
    CHECK(actualQty == 206);
}

TEST_CASE("Phase3 예시A: 총 생산시간 = 0.8 * 206 = 164.8 min") {
    double totalTime = ProductionCalc::calcTotalTime(0.8, 206);
    CHECK(totalTime == doctest::Approx(164.8));
}

// ── [PHASE3 §5] 실 생산량 계산 — 문서 예시 B ───────────────
// 신규 주문 20ea, 재고 10ea, 큐 동일 시료 대기 20ea, 수율 0.80, 생산시간 5.0 min/ea

TEST_CASE("Phase3 예시B: 유효 가용 재고 = max(0, 10 - 20) = 0 (음수 → 0)") {
    int effectiveStock = ProductionCalc::calcEffectiveStock(10, 20);
    CHECK(effectiveStock == 0);
}

TEST_CASE("Phase3 예시B: 부족분 = 20 - 0 = 20 (전량 생산)") {
    int shortage = ProductionCalc::calcShortage(20, 0);
    CHECK(shortage == 20);
}

TEST_CASE("Phase3 예시B: 실 생산량 = ceil(20 / (0.80 * 0.9)) = 28") {
    int actualQty = ProductionCalc::calcActualQty(20, 0.80);
    CHECK(actualQty == 28);
}

TEST_CASE("Phase3 예시B: 총 생산시간 = 5.0 * 28 = 140.0 min") {
    double totalTime = ProductionCalc::calcTotalTime(5.0, 28);
    CHECK(totalTime == doctest::Approx(140.0));
}

// ── [PHASE3 §2] 생산라인 EMPTY → RUNNING 전환 ─────────────

TEST_CASE("생산라인 EMPTY: 큐가 비어 있으면 EMPTY 상태") {
    Phase3Fixture f;
    CHECK(f.ds.GetProductionQueue().empty());
}

TEST_CASE("생산라인 RUNNING: 첫 번째 job 등록 시 큐에 1건 존재") {
    Phase3Fixture f;
    f.ds.AddProductionJob(f.makeJob(1, "01", 200, 30, 170, 206, 164.8, "2026-06-12 10:00:00"));
    CHECK(f.ds.GetProductionQueue().size() == 1);
}

TEST_CASE("생산라인 EMPTY일 때 첫 번째 job의 startedAt이 설정되어 있어야 한다") {
    Phase3Fixture f;
    // OrderController가 EMPTY 상태에서 즉시 startedAt 기록 — 여기서는 직접 세팅하여 검증
    f.ds.AddProductionJob(f.makeJob(1, "01", 200, 30, 170, 206, 164.8, "2026-06-12 10:00:00"));
    const auto& front = f.ds.GetProductionQueue().front();
    CHECK(!front.startedAt.empty());
}

// ── [PHASE3 §2] FIFO 순서 보장 ────────────────────────────

TEST_CASE("FIFO: 3건 등록 시 등록 순서대로 큐에 쌓인다") {
    Phase3Fixture f;
    f.ds.AddProductionJob(f.makeJob(1, "01", 200, 30, 170, 206, 164.8, "2026-06-12 10:00:00"));
    f.ds.AddProductionJob(f.makeJob(2, "02", 150,  0, 150, 185, 120.5, ""));
    f.ds.AddProductionJob(f.makeJob(3, "01", 300, 10, 290, 357, 285.6, ""));

    const auto& q = f.ds.GetProductionQueue();
    CHECK(q.size() == 3);
    CHECK(q[0].orderId == 1);
    CHECK(q[1].orderId == 2);
    CHECK(q[2].orderId == 3);
}

TEST_CASE("FIFO: 대기 항목(인덱스 1 이상)의 startedAt은 빈 문자열이어야 한다") {
    Phase3Fixture f;
    f.ds.AddProductionJob(f.makeJob(1, "01", 200, 30, 170, 206, 164.8, "2026-06-12 10:00:00"));
    f.ds.AddProductionJob(f.makeJob(2, "02", 150,  0, 150, 185, 120.5, ""));

    const auto& q = f.ds.GetProductionQueue();
    CHECK(q[0].startedAt == "2026-06-12 10:00:00"); // front: 생산 중
    CHECK(q[1].startedAt == "");                     // 대기: 시작 전
}

// ── [PHASE3 §6] 유효 가용 재고 계산 — GetQueuedQuantityForSample ─

TEST_CASE("GetQueuedQuantityForSample: 동일 시료 대기 주문량 합산") {
    Phase3Fixture f;
    f.ds.AddProductionJob(f.makeJob(1, "01", 200, 30, 170, 206, 164.8, "2026-06-12 10:00:00"));
    f.ds.AddProductionJob(f.makeJob(2, "01", 100, 0,  100, 123,  98.4, ""));
    f.ds.AddProductionJob(f.makeJob(3, "02",  50, 0,   50,  62,  49.6, ""));

    CHECK(f.ds.GetQueuedQuantityForSample("01") == 300); // 200 + 100
    CHECK(f.ds.GetQueuedQuantityForSample("02") ==  50);
    CHECK(f.ds.GetQueuedQuantityForSample("99") ==   0); // 없는 시료
}

TEST_CASE("GetQueuedQuantityForSample: 유효 가용 재고 계산에 올바르게 사용된다") {
    Phase3Fixture f;
    // 재고 10, 동일 시료 대기 20 → 유효 가용 재고 0
    f.ds.AddProductionJob(f.makeJob(1, "01", 150, 10, 140, 173, 138.4, "2026-06-12 10:00:00"));
    f.ds.AddProductionJob(f.makeJob(2, "01",  50,  0,  50,  62,  49.6, ""));

    int queuedQty      = f.ds.GetQueuedQuantityForSample("01");
    int effectiveStock = ProductionCalc::calcEffectiveStock(10, queuedQty);

    CHECK(queuedQty      == 200);
    CHECK(effectiveStock ==   0); // max(0, 10 - 200) = 0
}

// ── [PHASE3 §7] 생산 완료 처리 통합 ────────────────────────

TEST_CASE("생산 완료: PopProductionJob 후 큐에서 제거된다") {
    Phase3Fixture f;
    f.ds.AddProductionJob(f.makeJob(1, "01", 200, 30, 170, 206, 164.8, "2026-06-12 10:00:00"));
    f.ds.AddProductionJob(f.makeJob(2, "02", 150,  0, 150, 185, 120.5, ""));

    ProductionJob out;
    bool ok = f.ds.PopProductionJob(out);

    CHECK(ok == true);
    CHECK(out.orderId == 1);
    CHECK(f.ds.GetProductionQueue().size() == 1);
    CHECK(f.ds.GetProductionQueue().front().orderId == 2);
}

TEST_CASE("생산 완료: actualQty만큼 시료 재고가 증가한다") {
    Phase3Fixture f;
    f.ds.AddSample(f.makeSample("01", 30));
    f.ds.AddProductionJob(f.makeJob(1, "01", 200, 30, 170, 206, 164.8, "2026-06-12 10:00:00"));

    ProductionJob job;
    f.ds.PopProductionJob(job);
    f.ds.UpdateSampleStock("01", job.actualQty); // 재고 += 206

    auto sample = f.ds.FindSampleById("01");
    CHECK(sample.has_value());
    CHECK(sample->stock == 236); // 30 + 206
}

TEST_CASE("생산 완료: 주문 상태가 PRODUCING → CONFIRMED로 전환된다") {
    Phase3Fixture f;
    f.ds.AddSample(f.makeSample("01", 30));
    f.ds.AddOrder(f.makeOrder(1, "01", 200, OrderStatus::Producing));
    f.ds.AddProductionJob(f.makeJob(1, "01", 200, 30, 170, 206, 164.8, "2026-06-12 10:00:00"));

    ProductionJob job;
    f.ds.PopProductionJob(job);
    f.ds.UpdateSampleStock("01", job.actualQty);

    auto order = f.ds.FindOrderById(1);
    OrderData updated = *order;
    updated.status = OrderStatus::Confirmed;
    f.ds.UpdateOrder(updated);

    auto result = f.ds.FindOrderById(1);
    CHECK(result.has_value());
    CHECK(result->status == OrderStatus::Confirmed);
}

TEST_CASE("생산 완료 후 다음 대기 작업이 SetFrontStartedAt으로 시작된다") {
    Phase3Fixture f;
    f.ds.AddProductionJob(f.makeJob(1, "01", 200, 30, 170, 206, 164.8, "2026-06-12 10:00:00"));
    f.ds.AddProductionJob(f.makeJob(2, "02", 150,  0, 150, 185, 120.5, ""));

    ProductionJob dummy;
    f.ds.PopProductionJob(dummy);

    // 다음 작업 생산 시작 기록
    f.ds.SetFrontStartedAt("2026-06-12 12:45:00");

    const auto& q = f.ds.GetProductionQueue();
    CHECK(!q.empty());
    CHECK(q.front().orderId == 2);
    CHECK(q.front().startedAt == "2026-06-12 12:45:00");
}

// ── [PHASE3 §7] JSON 영속화 ───────────────────────────────

TEST_CASE("JSON 영속화: Save 후 Load 시 큐 내용과 startedAt이 복원된다") {
    Phase3Fixture f;
    f.ds.AddProductionJob(f.makeJob(1, "01", 200, 30, 170, 206, 164.8, "2026-06-12 10:00:00"));
    f.ds.AddProductionJob(f.makeJob(2, "02", 150,  0, 150, 185, 120.5, ""));
    f.ds.SaveProduction();

    DataStore ds2(f.dir);
    ds2.Load();

    const auto& q = ds2.GetProductionQueue();
    CHECK(q.size() == 2);
    CHECK(q[0].orderId    == 1);
    CHECK(q[0].actualQty  == 206);
    CHECK(q[0].totalTime  == doctest::Approx(164.8));
    CHECK(q[0].startedAt  == "2026-06-12 10:00:00");
    CHECK(q[1].orderId    == 2);
    CHECK(q[1].startedAt  == "");                    // 대기 항목은 빈 문자열 유지
}

TEST_CASE("JSON 영속화: 큐가 비어 있을 때 Save/Load해도 빈 상태가 유지된다") {
    Phase3Fixture f;
    f.ds.SaveProduction();

    DataStore ds2(f.dir);
    ds2.Load();
    CHECK(ds2.GetProductionQueue().empty());
}
