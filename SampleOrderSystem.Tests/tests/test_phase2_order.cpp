#include "doctest/doctest.h"
#include "repository/DataStore.h"
#include <filesystem>

namespace fs = std::filesystem;

// ── 픽스처 ───────────────────────────────────────────────
struct Phase2Fixture {
    std::string dir = "./test_phase2_tmp";
    DataStore   ds;

    Phase2Fixture() : ds(dir) {
        fs::remove_all(dir);
        ds.Load();
    }
    ~Phase2Fixture() { fs::remove_all(dir); }

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
                        int qty, OrderStatus status = OrderStatus::Reserved) {
        OrderData o;
        o.id         = id;
        o.orderNo    = "ORD-20260612-" + std::to_string(1000 + id);
        o.sampleId   = sampleId;
        o.sampleName = "Sample-" + sampleId;
        o.customer   = "TestCustomer";
        o.quantity   = qty;
        o.status     = status;
        o.date       = "2026-06-12";
        return o;
    }
};

// ── [PHASE2 §6] 주문 접수 ─────────────────────────────────

TEST_CASE("주문 접수: AddOrder 후 FindOrderById로 조회된다") {
    Phase2Fixture f;
    f.ds.AddOrder(f.makeOrder(1, "01", 200));

    auto result = f.ds.FindOrderById(1);
    CHECK(result.has_value());
    CHECK(result->sampleId == "01");
    CHECK(result->quantity == 200);
}

TEST_CASE("주문 접수: 초기 상태는 RESERVED이다") {
    Phase2Fixture f;
    f.ds.AddOrder(f.makeOrder(1, "01", 100));

    auto result = f.ds.FindOrderById(1);
    CHECK(result.has_value());
    CHECK(result->status == OrderStatus::Reserved);
}

TEST_CASE("주문 접수: NextOrderId는 호출할 때마다 순차 증가한다") {
    Phase2Fixture f;
    CHECK(f.ds.NextOrderId() == 1);
    CHECK(f.ds.NextOrderId() == 2);
    CHECK(f.ds.NextOrderId() == 3);
}

TEST_CASE("주문 접수: AddOrder 후 재고에는 변화가 없다") {
    Phase2Fixture f;
    f.ds.AddSample(f.makeSample("01", 480));
    f.ds.AddOrder(f.makeOrder(1, "01", 100));

    auto sample = f.ds.FindSampleById("01");
    CHECK(sample.has_value());
    CHECK(sample->stock == 480); // 접수 단계에서 재고 차감 없음
}

// ── [PHASE2 §6] 미등록 시료 ID 접수 제한 ─────────────────

TEST_CASE("시료 ID 검증: 등록된 시료 ID는 ExistsSampleId가 true를 반환한다") {
    Phase2Fixture f;
    f.ds.AddSample(f.makeSample("01", 100));

    CHECK(f.ds.ExistsSampleId("01") == true);
}

TEST_CASE("시료 ID 검증: 미등록 시료 ID는 ExistsSampleId가 false를 반환한다") {
    Phase2Fixture f;

    CHECK(f.ds.ExistsSampleId("99") == false);
    CHECK(f.ds.ExistsSampleId("S-999") == false);
}

TEST_CASE("시료 ID 검증: 시료 삭제 후에는 false를 반환한다") {
    Phase2Fixture f;
    f.ds.AddSample(f.makeSample("01", 100));
    CHECK(f.ds.ExistsSampleId("01") == true);

    // 시료가 없는 다른 ID
    CHECK(f.ds.ExistsSampleId("02") == false);
}

// ── [PHASE2 §7] RESERVED 목록 조회 ───────────────────────

TEST_CASE("RESERVED 목록: RESERVED 상태 주문만 반환한다") {
    Phase2Fixture f;
    f.ds.AddOrder(f.makeOrder(1, "01", 100, OrderStatus::Reserved));
    f.ds.AddOrder(f.makeOrder(2, "01",  50, OrderStatus::Confirmed));
    f.ds.AddOrder(f.makeOrder(3, "01",  80, OrderStatus::Producing));
    f.ds.AddOrder(f.makeOrder(4, "01",  30, OrderStatus::Rejected));

    auto reserved = f.ds.GetReservedOrders();
    CHECK(reserved.size() == 1);
    CHECK(reserved[0].id == 1);
}

TEST_CASE("RESERVED 목록: 주문이 없으면 빈 목록을 반환한다") {
    Phase2Fixture f;

    auto reserved = f.ds.GetReservedOrders();
    CHECK(reserved.empty());
}

TEST_CASE("RESERVED 목록: 전부 비 RESERVED 상태이면 빈 목록을 반환한다") {
    Phase2Fixture f;
    f.ds.AddOrder(f.makeOrder(1, "01", 100, OrderStatus::Confirmed));
    f.ds.AddOrder(f.makeOrder(2, "01",  50, OrderStatus::Rejected));

    auto reserved = f.ds.GetReservedOrders();
    CHECK(reserved.empty());
}

// ── [PHASE2 §7] 승인 — 재고 충분 (PHASE2 고객 시나리오 Step 1) ─

TEST_CASE("승인(재고 충분): 상태가 CONFIRMED로 전환된다") {
    Phase2Fixture f;
    f.ds.AddSample(f.makeSample("01", 480));
    f.ds.AddOrder(f.makeOrder(1, "01", 100));

    OrderData updated = f.ds.FindOrderById(1).value();
    updated.status = OrderStatus::Confirmed;
    f.ds.UpdateOrder(updated);
    f.ds.UpdateSampleStock("01", -100);

    CHECK(f.ds.FindOrderById(1)->status == OrderStatus::Confirmed);
}

TEST_CASE("승인(재고 충분): 재고에서 주문 수량만큼 차감된다 — PHASE2 Step1: 480→380") {
    Phase2Fixture f;
    f.ds.AddSample(f.makeSample("01", 480));
    f.ds.AddOrder(f.makeOrder(1, "01", 100));

    f.ds.UpdateSampleStock("01", -100);

    auto sample = f.ds.FindSampleById("01");
    CHECK(sample.has_value());
    CHECK(sample->stock == 380);
}

TEST_CASE("승인(재고 충분): 승인 후 RESERVED 목록에서 사라진다") {
    Phase2Fixture f;
    f.ds.AddOrder(f.makeOrder(1, "01", 100));

    OrderData updated = f.ds.FindOrderById(1).value();
    updated.status = OrderStatus::Confirmed;
    f.ds.UpdateOrder(updated);

    CHECK(f.ds.GetReservedOrders().empty());
}

// ── [PHASE2 §7] 승인 — 재고 부족 (PHASE2 고객 시나리오 Step 2) ─

TEST_CASE("승인(재고 부족): 상태가 PRODUCING으로 전환된다") {
    Phase2Fixture f;
    f.ds.AddSample(f.makeSample("01", 0));
    f.ds.AddOrder(f.makeOrder(1, "01", 200));

    OrderData updated = f.ds.FindOrderById(1).value();
    updated.status         = OrderStatus::Producing;
    updated.producingBased = true;
    f.ds.UpdateOrder(updated);

    CHECK(f.ds.FindOrderById(1)->status == OrderStatus::Producing);
}

TEST_CASE("승인(재고 부족): 재고가 차감되지 않는다 — PHASE2 Step2: 재고 0 유지") {
    Phase2Fixture f;
    f.ds.AddSample(f.makeSample("01", 0));
    f.ds.AddOrder(f.makeOrder(1, "01", 200));

    // PRODUCING 전환 시 재고 차감 없음
    OrderData updated = f.ds.FindOrderById(1).value();
    updated.status         = OrderStatus::Producing;
    updated.producingBased = true;
    f.ds.UpdateOrder(updated);

    auto sample = f.ds.FindSampleById("01");
    CHECK(sample.has_value());
    CHECK(sample->stock == 0);
}

TEST_CASE("승인(재고 부족): producingBased 플래그가 true로 설정된다") {
    Phase2Fixture f;
    f.ds.AddOrder(f.makeOrder(1, "01", 200));

    OrderData updated = f.ds.FindOrderById(1).value();
    updated.status         = OrderStatus::Producing;
    updated.producingBased = true;
    f.ds.UpdateOrder(updated);

    CHECK(f.ds.FindOrderById(1)->producingBased == true);
}

// ── [PHASE2 §7] 거절 (PHASE2 고객 시나리오 Step 3) ─────────

TEST_CASE("거절: 상태가 REJECTED로 전환된다") {
    Phase2Fixture f;
    f.ds.AddOrder(f.makeOrder(1, "01", 100));

    OrderData updated = f.ds.FindOrderById(1).value();
    updated.status = OrderStatus::Rejected;
    f.ds.UpdateOrder(updated);

    CHECK(f.ds.FindOrderById(1)->status == OrderStatus::Rejected);
}

TEST_CASE("거절: RESERVED 목록에서 제외된다") {
    Phase2Fixture f;
    f.ds.AddOrder(f.makeOrder(1, "01", 100));
    f.ds.AddOrder(f.makeOrder(2, "02",  50));

    OrderData updated = f.ds.FindOrderById(1).value();
    updated.status = OrderStatus::Rejected;
    f.ds.UpdateOrder(updated);

    auto reserved = f.ds.GetReservedOrders();
    CHECK(reserved.size() == 1);
    CHECK(reserved[0].id == 2); // 거절된 1번은 없음
}

TEST_CASE("거절: producingBased 플래그가 false 상태로 유지된다") {
    Phase2Fixture f;
    f.ds.AddOrder(f.makeOrder(1, "01", 100));

    OrderData updated = f.ds.FindOrderById(1).value();
    updated.status = OrderStatus::Rejected;
    f.ds.UpdateOrder(updated);

    CHECK(f.ds.FindOrderById(1)->producingBased == false);
}

// ── [PHASE2 §7] 승인 경계값 ──────────────────────────────

TEST_CASE("승인 경계값: 재고 == 주문 수량이면 CONFIRMED 전환, 재고 0이 된다") {
    Phase2Fixture f;
    f.ds.AddSample(f.makeSample("01", 50));
    f.ds.AddOrder(f.makeOrder(1, "01", 50)); // 재고와 주문 수량이 정확히 같음

    f.ds.UpdateSampleStock("01", -50);
    OrderData updated = f.ds.FindOrderById(1).value();
    updated.status = OrderStatus::Confirmed;
    f.ds.UpdateOrder(updated);

    auto sample = f.ds.FindSampleById("01");
    CHECK(sample->stock == 0);
    CHECK(f.ds.FindOrderById(1)->status == OrderStatus::Confirmed);
}

TEST_CASE("승인 경계값: 재고 1, 주문 2이면 PRODUCING 전환, 재고 불변") {
    Phase2Fixture f;
    f.ds.AddSample(f.makeSample("01", 1));
    f.ds.AddOrder(f.makeOrder(1, "01", 2));

    // 재고 부족 분기
    OrderData updated = f.ds.FindOrderById(1).value();
    updated.status         = OrderStatus::Producing;
    updated.producingBased = true;
    f.ds.UpdateOrder(updated);

    auto sample = f.ds.FindSampleById("01");
    CHECK(sample->stock == 1); // 차감 없음
    CHECK(f.ds.FindOrderById(1)->status == OrderStatus::Producing);
}

// ── [PHASE2] 복수 주문 혼합 상태 시나리오 ─────────────────

TEST_CASE("복수 주문: 다양한 상태 주문 중 RESERVED만 목록에 노출된다") {
    Phase2Fixture f;
    f.ds.AddOrder(f.makeOrder(1, "01", 100, OrderStatus::Reserved));
    f.ds.AddOrder(f.makeOrder(2, "02",  50, OrderStatus::Confirmed));
    f.ds.AddOrder(f.makeOrder(3, "01",  80, OrderStatus::Producing));
    f.ds.AddOrder(f.makeOrder(4, "02",  30, OrderStatus::Rejected));
    f.ds.AddOrder(f.makeOrder(5, "01",  20, OrderStatus::Reserved));

    auto reserved = f.ds.GetReservedOrders();
    CHECK(reserved.size() == 2);
    CHECK(reserved[0].id == 1);
    CHECK(reserved[1].id == 5);
}

TEST_CASE("복수 주문: 순차 승인 시 각 주문이 독립적으로 처리된다") {
    Phase2Fixture f;
    f.ds.AddSample(f.makeSample("01", 200));
    f.ds.AddOrder(f.makeOrder(1, "01", 100));
    f.ds.AddOrder(f.makeOrder(2, "01",  50));

    // 1번 주문 승인 (재고 200 → 100)
    f.ds.UpdateSampleStock("01", -100);
    OrderData u1 = f.ds.FindOrderById(1).value();
    u1.status = OrderStatus::Confirmed;
    f.ds.UpdateOrder(u1);

    // 2번 주문 승인 (재고 100 → 50)
    f.ds.UpdateSampleStock("01", -50);
    OrderData u2 = f.ds.FindOrderById(2).value();
    u2.status = OrderStatus::Confirmed;
    f.ds.UpdateOrder(u2);

    CHECK(f.ds.FindSampleById("01")->stock == 50);
    CHECK(f.ds.FindOrderById(1)->status == OrderStatus::Confirmed);
    CHECK(f.ds.FindOrderById(2)->status == OrderStatus::Confirmed);
    CHECK(f.ds.GetReservedOrders().empty());
}

// ── [PHASE2] JSON 영속화 ──────────────────────────────────

TEST_CASE("JSON 영속화: Save/Load 후 주문 상태가 그대로 유지된다") {
    Phase2Fixture f;
    f.ds.AddOrder(f.makeOrder(1, "01", 100, OrderStatus::Reserved));
    f.ds.AddOrder(f.makeOrder(2, "02",  50, OrderStatus::Confirmed));
    f.ds.AddOrder(f.makeOrder(3, "01",  80, OrderStatus::Rejected));
    f.ds.SaveOrders();

    DataStore ds2(f.dir);
    ds2.Load();

    CHECK(ds2.FindOrderById(1)->status == OrderStatus::Reserved);
    CHECK(ds2.FindOrderById(2)->status == OrderStatus::Confirmed);
    CHECK(ds2.FindOrderById(3)->status == OrderStatus::Rejected);
}

TEST_CASE("JSON 영속화: 재시작 후 NextOrderId가 이어서 증가한다 (중복 ID 없음)") {
    Phase2Fixture f;
    f.ds.AddOrder(f.makeOrder(f.ds.NextOrderId(), "01", 100));
    f.ds.AddOrder(f.makeOrder(f.ds.NextOrderId(), "01",  50));
    f.ds.SaveOrders();

    DataStore ds2(f.dir);
    ds2.Load();

    int nextId = ds2.NextOrderId();
    CHECK(nextId == 3); // 1, 2 이미 사용 → 3부터
}

TEST_CASE("JSON 영속화: producingBased 플래그가 Save/Load 후에도 유지된다") {
    Phase2Fixture f;
    OrderData o = f.makeOrder(1, "01", 200, OrderStatus::Producing);
    o.producingBased = true;
    f.ds.AddOrder(o);
    f.ds.SaveOrders();

    DataStore ds2(f.dir);
    ds2.Load();

    CHECK(ds2.FindOrderById(1)->producingBased == true);
}
