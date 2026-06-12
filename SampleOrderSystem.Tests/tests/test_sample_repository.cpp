#include "doctest/doctest.h"
#include "SampleRepository.h"
#include <filesystem>

namespace fs = std::filesystem;

struct SampleRepoFixture {
    std::string       dir = "./test_sample_repo_tmp";
    SampleRepository  repo;

    SampleRepoFixture() : repo(dir) {
        fs::remove_all(dir);
        repo.Load();
    }
    ~SampleRepoFixture() { fs::remove_all(dir); }

    SampleData make(const std::string& id, const std::string& name, int stock) {
        SampleData s;
        s.id             = id;
        s.name           = name;
        s.productionTime = 10.0;
        s.yield          = 0.9;
        s.stock          = stock;
        return s;
    }
};

TEST_CASE("SampleRepository: Load on empty dir returns empty") {
    SampleRepoFixture f;
    CHECK(f.repo.GetAll().empty());
}

TEST_CASE("SampleRepository: Add and FindById") {
    SampleRepoFixture f;
    f.repo.Add(f.make("01", "Alpha", 100));

    auto result = f.repo.FindById("01");
    CHECK(result.has_value());
    CHECK(result->name == "Alpha");
    CHECK(result->stock == 100);
}

TEST_CASE("SampleRepository: FindByName is case-insensitive") {
    SampleRepoFixture f;
    f.repo.Add(f.make("01", "Alpha-001", 100));
    f.repo.Add(f.make("02", "Beta-002",  50));

    auto results = f.repo.FindByName("alpha");
    CHECK(results.size() == 1);
    CHECK(results[0].id == "01");
}

TEST_CASE("SampleRepository: Exists returns correct bool") {
    SampleRepoFixture f;
    f.repo.Add(f.make("01", "Alpha", 100));
    CHECK(f.repo.Exists("01") == true);
    CHECK(f.repo.Exists("99") == false);
}

TEST_CASE("SampleRepository: UpdateStock adjusts stock by delta") {
    SampleRepoFixture f;
    f.repo.Add(f.make("01", "Alpha", 100));

    f.repo.UpdateStock("01", 50);
    CHECK(f.repo.FindById("01")->stock == 150);

    f.repo.UpdateStock("01", -30);
    CHECK(f.repo.FindById("01")->stock == 120);
}

TEST_CASE("SampleRepository: GenerateId increments sequentially") {
    SampleRepoFixture f;
    CHECK(f.repo.GenerateId() == "01");
    CHECK(f.repo.GenerateId() == "02");
}

TEST_CASE("SampleRepository: GetTotalStock sums all stocks") {
    SampleRepoFixture f;
    f.repo.Add(f.make("01", "Alpha", 100));
    f.repo.Add(f.make("02", "Beta",   50));
    CHECK(f.repo.GetTotalStock() == 150);
}

TEST_CASE("SampleRepository: data persists after reload") {
    std::string dir = "./test_sample_repo_persist_tmp";
    fs::remove_all(dir);
    {
        SampleRepository repo(dir);
        repo.Load();
        SampleData s;
        s.id = "01"; s.name = "Alpha"; s.productionTime = 10.0; s.yield = 0.9; s.stock = 100;
        repo.Add(s);
    }
    {
        SampleRepository repo(dir);
        repo.Load();
        CHECK(repo.GetAll().size() == 1);
        CHECK(repo.FindById("01")->name == "Alpha");
    }
    fs::remove_all(dir);
}
