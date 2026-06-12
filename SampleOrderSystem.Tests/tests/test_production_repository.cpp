#include "doctest/doctest.h"
#include "repository/ProductionRepository.h"
#include <filesystem>

namespace fs = std::filesystem;

struct ProductionRepoFixture {
    std::string           dir = "./test_production_repo_tmp";
    ProductionRepository  repo;

    ProductionRepoFixture() : repo(dir) {
        fs::remove_all(dir);
        repo.Load();
    }
    ~ProductionRepoFixture() { fs::remove_all(dir); }

    ProductionJob makeJob(int orderId, const std::string& sampleId, int qty) {
        ProductionJob j;
        j.orderId    = orderId;
        j.orderNo    = "ORD-TEST-" + std::to_string(orderId);
        j.sampleId   = sampleId;
        j.sampleName = "Sample-" + sampleId;
        j.quantity   = qty;
        j.stock      = 0;
        j.shortage   = qty;
        j.actualQty  = qty;
        j.totalTime  = 10.0;
        j.enqueuedAt = "2026-06-12 00:00:00";
        j.startedAt  = "2026-06-12 00:00:00";
        return j;
    }
};

TEST_CASE("ProductionRepository: Load on empty dir returns empty queue") {
    ProductionRepoFixture f;
    CHECK(f.repo.GetQueue().empty());
}

TEST_CASE("ProductionRepository: Enqueue adds job to queue") {
    ProductionRepoFixture f;
    f.repo.Enqueue(f.makeJob(1, "01", 100));
    CHECK(f.repo.GetQueue().size() == 1);
    CHECK(f.repo.GetQueue()[0].orderId == 1);
}

TEST_CASE("ProductionRepository: Dequeue pops front item (FIFO)") {
    ProductionRepoFixture f;
    f.repo.Enqueue(f.makeJob(1, "01", 100));
    f.repo.Enqueue(f.makeJob(2, "01",  50));

    ProductionJob out;
    bool ok = f.repo.Dequeue(out);
    CHECK(ok == true);
    CHECK(out.orderId == 1);
    CHECK(f.repo.GetQueue().size() == 1);
    CHECK(f.repo.GetQueue()[0].orderId == 2);
}

TEST_CASE("ProductionRepository: Dequeue returns false on empty queue") {
    ProductionRepoFixture f;
    ProductionJob out;
    CHECK(f.repo.Dequeue(out) == false);
}

TEST_CASE("ProductionRepository: SetFrontStartedAt updates front job") {
    ProductionRepoFixture f;
    f.repo.Enqueue(f.makeJob(1, "01", 100));

    f.repo.SetFrontStartedAt("2026-06-12 12:00:00");
    CHECK(f.repo.GetQueue().front().startedAt == "2026-06-12 12:00:00");
}

TEST_CASE("ProductionRepository: GetQueuedQuantityForSample sums by sampleId") {
    ProductionRepoFixture f;
    f.repo.Enqueue(f.makeJob(1, "01", 100));
    f.repo.Enqueue(f.makeJob(2, "01",  50));
    f.repo.Enqueue(f.makeJob(3, "02",  30));

    CHECK(f.repo.GetQueuedQuantityForSample("01") == 150);
    CHECK(f.repo.GetQueuedQuantityForSample("02") ==  30);
    CHECK(f.repo.GetQueuedQuantityForSample("99") ==   0);
}
