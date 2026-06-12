#include "doctest/doctest.h"
#include "repository/DataStore.h"
#include <filesystem>

namespace fs = std::filesystem;

// ── 테스트 픽스처 ─────────────────────────────────────────
struct ReleaseFixture {
    std::string dir = "./test_release_tmp";
    DataStore   ds;

    ReleaseFixture() : ds(dir) {
        fs::remove_all(dir);
        ds.Load();
    }
    ~ReleaseFixture() {
        fs::remove_all(dir);
    }

    SampleData makeSample(const std::string& id, int stock) {
        SampleData s;
        s.id             = id;
        s.name           = "Sample-" + id;
        s.productionTime = 10.0;
        s.yield          = 0.9;
        s.stock          = stock;
        return s;
    }

    OrderData makeOrder(int id, const std::string& sampleId,
                        int qty, OrderStatus status) {
        OrderData o;
        o.id         = id;
        o.orderNo    = "ORD-TEST-" + std::to_string(id);
        o.sampleId   = sampleId;
        o.sampleName = "Sample-" + sampleId;
        o.customer   = "TestCustomer";
        o.quantity   = qty;
        o.status     = status;
        o.date       = "2026-06-12";
        return o;
    }
};

// ── [RED] GetConfirmedOrders ──────────────────────────────

TEST_CASE("GetConfirmedOrders returns only CONFIRMED orders") {
    ReleaseFixture f;
    f.ds.AddOrder(f.makeOrder(1, "01", 10, OrderStatus::Confirmed));
    f.ds.AddOrder(f.makeOrder(2, "01", 5,  OrderStatus::Reserved));
    f.ds.AddOrder(f.makeOrder(3, "01", 20, OrderStatus::Release));
    f.ds.AddOrder(f.makeOrder(4, "01", 8,  OrderStatus::Producing));

    auto confirmed = f.ds.GetConfirmedOrders();

    CHECK(confirmed.size() == 1);
    CHECK(confirmed[0].id == 1);
    CHECK(confirmed[0].status == OrderStatus::Confirmed);
}

TEST_CASE("GetConfirmedOrders returns empty when no CONFIRMED orders exist") {
    ReleaseFixture f;
    f.ds.AddOrder(f.makeOrder(1, "01", 10, OrderStatus::Reserved));
    f.ds.AddOrder(f.makeOrder(2, "01", 5,  OrderStatus::Rejected));

    auto confirmed = f.ds.GetConfirmedOrders();

    CHECK(confirmed.empty());
}

// ── [RED] ReleaseOrder ────────────────────────────────────

TEST_CASE("ReleaseOrder changes CONFIRMED order status to RELEASE") {
    ReleaseFixture f;
    f.ds.AddOrder(f.makeOrder(1, "01", 10, OrderStatus::Confirmed));

    bool result = f.ds.ReleaseOrder(1);

    CHECK(result == true);
    auto updated = f.ds.FindOrderById(1);
    CHECK(updated.has_value());
    CHECK(updated->status == OrderStatus::Release);
}

TEST_CASE("ReleaseOrder does not change stock") {
    ReleaseFixture f;
    f.ds.AddSample(f.makeSample("01", 50));
    f.ds.AddOrder(f.makeOrder(1, "01", 10, OrderStatus::Confirmed));

    f.ds.ReleaseOrder(1);

    auto sample = f.ds.FindSampleById("01");
    CHECK(sample.has_value());
    CHECK(sample->stock == 50);
}

TEST_CASE("ReleaseOrder returns false when order does not exist") {
    ReleaseFixture f;

    bool result = f.ds.ReleaseOrder(999);

    CHECK(result == false);
}

TEST_CASE("ReleaseOrder returns false when order is not CONFIRMED") {
    ReleaseFixture f;
    f.ds.AddOrder(f.makeOrder(1, "01", 10, OrderStatus::Reserved));

    bool result = f.ds.ReleaseOrder(1);

    CHECK(result == false);
    auto order = f.ds.FindOrderById(1);
    CHECK(order->status == OrderStatus::Reserved); // unchanged
}
