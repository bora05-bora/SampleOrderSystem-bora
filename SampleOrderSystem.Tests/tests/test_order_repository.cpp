#include "doctest/doctest.h"
#include "repository/OrderRepository.h"
#include <filesystem>

namespace fs = std::filesystem;

struct OrderRepoFixture {
    std::string     dir = "./test_order_repo_tmp";
    OrderRepository repo;

    OrderRepoFixture() : repo(dir) {
        fs::remove_all(dir);
        repo.Load();
    }
    ~OrderRepoFixture() { fs::remove_all(dir); }

    OrderData make(int id, const std::string& sampleId, int qty, OrderStatus status) {
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

TEST_CASE("OrderRepository: Load on empty dir returns empty") {
    OrderRepoFixture f;
    CHECK(f.repo.GetAll().empty());
}

TEST_CASE("OrderRepository: Add and FindById") {
    OrderRepoFixture f;
    f.repo.Add(f.make(1, "01", 10, OrderStatus::Reserved));

    auto result = f.repo.FindById(1);
    CHECK(result.has_value());
    CHECK(result->sampleId == "01");
    CHECK(result->quantity == 10);
}

TEST_CASE("OrderRepository: GetByStatus filters correctly") {
    OrderRepoFixture f;
    f.repo.Add(f.make(1, "01", 10, OrderStatus::Reserved));
    f.repo.Add(f.make(2, "01",  5, OrderStatus::Confirmed));
    f.repo.Add(f.make(3, "01",  8, OrderStatus::Rejected));

    auto reserved = f.repo.GetByStatus(OrderStatus::Reserved);
    CHECK(reserved.size() == 1);
    CHECK(reserved[0].id == 1);
}

TEST_CASE("OrderRepository: Update modifies existing order") {
    OrderRepoFixture f;
    f.repo.Add(f.make(1, "01", 10, OrderStatus::Reserved));

    OrderData updated = f.repo.FindById(1).value();
    updated.status = OrderStatus::Confirmed;
    f.repo.Update(updated);

    CHECK(f.repo.FindById(1)->status == OrderStatus::Confirmed);
}

TEST_CASE("OrderRepository: Release changes CONFIRMED to RELEASE") {
    OrderRepoFixture f;
    f.repo.Add(f.make(1, "01", 10, OrderStatus::Confirmed));

    bool ok = f.repo.Release(1);
    CHECK(ok == true);
    CHECK(f.repo.FindById(1)->status == OrderStatus::Release);
}

TEST_CASE("OrderRepository: Release returns false for non-CONFIRMED") {
    OrderRepoFixture f;
    f.repo.Add(f.make(1, "01", 10, OrderStatus::Reserved));

    bool ok = f.repo.Release(1);
    CHECK(ok == false);
    CHECK(f.repo.FindById(1)->status == OrderStatus::Reserved);
}

TEST_CASE("OrderRepository: NextId increments") {
    OrderRepoFixture f;
    CHECK(f.repo.NextId() == 1);
    CHECK(f.repo.NextId() == 2);
}

TEST_CASE("OrderRepository: GetStatusSummary counts correctly excluding Rejected") {
    OrderRepoFixture f;
    f.repo.Add(f.make(1, "01", 10, OrderStatus::Reserved));
    f.repo.Add(f.make(2, "01",  5, OrderStatus::Producing));
    f.repo.Add(f.make(3, "01",  8, OrderStatus::Confirmed));
    f.repo.Add(f.make(4, "01",  3, OrderStatus::Release));
    f.repo.Add(f.make(5, "01",  7, OrderStatus::Rejected));

    auto s = f.repo.GetStatusSummary();
    CHECK(s.reserved  == 1);
    CHECK(s.producing == 1);
    CHECK(s.confirmed == 1);
    CHECK(s.release   == 1);
}
