#include "doctest/doctest.h"
#include "DataStore.h"
#include <filesystem>

namespace fs = std::filesystem;

// ── 테스트 픽스처 ─────────────────────────────────────────
struct MonitorFixture {
    std::string dir = "./test_monitor_tmp";
    DataStore   ds;

    MonitorFixture() : ds(dir) {
        fs::remove_all(dir);
        ds.Load();
    }
    ~MonitorFixture() {
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

// ── [RED] GetOrderStatusSummary ───────────────────────────

TEST_CASE("GetOrderStatusSummary counts orders by status, excluding REJECTED") {
    MonitorFixture f;
    f.ds.AddOrder(f.makeOrder(1, "01", 10, OrderStatus::Reserved));
    f.ds.AddOrder(f.makeOrder(2, "01", 5,  OrderStatus::Reserved));
    f.ds.AddOrder(f.makeOrder(3, "01", 8,  OrderStatus::Producing));
    f.ds.AddOrder(f.makeOrder(4, "01", 20, OrderStatus::Confirmed));
    f.ds.AddOrder(f.makeOrder(5, "01", 15, OrderStatus::Release));
    f.ds.AddOrder(f.makeOrder(6, "01", 3,  OrderStatus::Rejected)); // 제외
    f.ds.AddOrder(f.makeOrder(7, "01", 7,  OrderStatus::Rejected)); // 제외

    auto summary = f.ds.GetOrderStatusSummary();

    CHECK(summary.reserved  == 2);
    CHECK(summary.producing == 1);
    CHECK(summary.confirmed == 1);
    CHECK(summary.release   == 1);
}

TEST_CASE("GetOrderStatusSummary returns all zeros when only REJECTED orders exist") {
    MonitorFixture f;
    f.ds.AddOrder(f.makeOrder(1, "01", 10, OrderStatus::Rejected));
    f.ds.AddOrder(f.makeOrder(2, "01", 5,  OrderStatus::Rejected));

    auto summary = f.ds.GetOrderStatusSummary();

    CHECK(summary.reserved  == 0);
    CHECK(summary.producing == 0);
    CHECK(summary.confirmed == 0);
    CHECK(summary.release   == 0);
}

// ── [RED] GetStockStatus ──────────────────────────────────

TEST_CASE("GetStockStatus returns Depleted when stock is 0") {
    MonitorFixture f;
    f.ds.AddSample(f.makeSample("01", 0));

    CHECK(f.ds.GetStockStatus("01") == StockStatus::Depleted);
}

TEST_CASE("GetStockStatus returns Short when RESERVED orders exceed stock") {
    MonitorFixture f;
    f.ds.AddSample(f.makeSample("01", 5));
    f.ds.AddOrder(f.makeOrder(1, "01", 10, OrderStatus::Reserved));

    CHECK(f.ds.GetStockStatus("01") == StockStatus::Short);
}

TEST_CASE("GetStockStatus returns Plenty when stock covers all RESERVED orders") {
    MonitorFixture f;
    f.ds.AddSample(f.makeSample("01", 20));
    f.ds.AddOrder(f.makeOrder(1, "01", 10, OrderStatus::Reserved));
    f.ds.AddOrder(f.makeOrder(2, "01", 5,  OrderStatus::Reserved));

    CHECK(f.ds.GetStockStatus("01") == StockStatus::Plenty);
}

TEST_CASE("GetStockStatus returns Plenty when no RESERVED orders exist for sample") {
    MonitorFixture f;
    f.ds.AddSample(f.makeSample("01", 10));
    // CONFIRMED, PRODUCING 주문은 재고 상태 판단에서 제외
    f.ds.AddOrder(f.makeOrder(1, "01", 10, OrderStatus::Confirmed));
    f.ds.AddOrder(f.makeOrder(2, "01", 5,  OrderStatus::Producing));

    CHECK(f.ds.GetStockStatus("01") == StockStatus::Plenty);
}

TEST_CASE("GetStockStatus ignores other samples RESERVED orders") {
    MonitorFixture f;
    f.ds.AddSample(f.makeSample("01", 5));
    f.ds.AddSample(f.makeSample("02", 5));
    f.ds.AddOrder(f.makeOrder(1, "02", 20, OrderStatus::Reserved)); // 다른 시료

    CHECK(f.ds.GetStockStatus("01") == StockStatus::Plenty);
}
