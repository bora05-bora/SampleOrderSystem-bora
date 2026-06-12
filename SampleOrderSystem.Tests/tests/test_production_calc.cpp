#include "doctest/doctest.h"
#include "ProductionCalc.h"

// ── 사이클 5: calcEffectiveStock ─────────────────────────

TEST_CASE("calcEffectiveStock subtracts queued quantity from stock") {
    CHECK(ProductionCalc::calcEffectiveStock(100, 30) == 70);
}

TEST_CASE("calcEffectiveStock returns 0 when queued exceeds stock") {
    CHECK(ProductionCalc::calcEffectiveStock(10, 20) == 0);
}

TEST_CASE("calcEffectiveStock returns stock when no queued quantity") {
    CHECK(ProductionCalc::calcEffectiveStock(50, 0) == 50);
}

TEST_CASE("calcEffectiveStock returns 0 when stock is 0") {
    CHECK(ProductionCalc::calcEffectiveStock(0, 0) == 0);
}

// ── 사이클 6: calcShortage ───────────────────────────────

TEST_CASE("calcShortage returns difference when quantity exceeds effectiveStock") {
    CHECK(ProductionCalc::calcShortage(30, 10) == 20);
}

TEST_CASE("calcShortage returns 0 when effectiveStock exactly covers quantity") {
    CHECK(ProductionCalc::calcShortage(10, 10) == 0);
}

TEST_CASE("calcShortage returns 0 when effectiveStock exceeds quantity") {
    CHECK(ProductionCalc::calcShortage(10, 20) == 0);
}

// ── 사이클 7: calcActualQty ──────────────────────────────

TEST_CASE("calcActualQty returns 0 when shortage is 0") {
    CHECK(ProductionCalc::calcActualQty(0, 0.9) == 0);
}

TEST_CASE("calcActualQty rounds up fractional result") {
    // ceil(1 / (0.9 * 0.9)) = ceil(1.234) = 2
    CHECK(ProductionCalc::calcActualQty(1, 0.9) == 2);
}

TEST_CASE("calcActualQty with yield 1.0") {
    // ceil(9 / (1.0 * 0.9)) = ceil(10.0) = 10
    CHECK(ProductionCalc::calcActualQty(9, 1.0) == 10);
}

TEST_CASE("calcActualQty with low yield increases production significantly") {
    // ceil(10 / (0.5 * 0.9)) = ceil(22.22) = 23
    CHECK(ProductionCalc::calcActualQty(10, 0.5) == 23);
}

// ── 사이클 8: calcTotalTime ──────────────────────────────

TEST_CASE("calcTotalTime multiplies productionTime by actualQty") {
    CHECK(ProductionCalc::calcTotalTime(10.0, 5) == doctest::Approx(50.0));
}

TEST_CASE("calcTotalTime returns 0 when actualQty is 0") {
    CHECK(ProductionCalc::calcTotalTime(10.0, 0) == doctest::Approx(0.0));
}

TEST_CASE("calcTotalTime handles fractional productionTime") {
    CHECK(ProductionCalc::calcTotalTime(2.5, 4) == doctest::Approx(10.0));
}

// ── 사이클 9: 경계값 통합 시나리오 ──────────────────────

TEST_CASE("stock exactly equal to quantity results in zero shortage") {
    int effectiveStock = ProductionCalc::calcEffectiveStock(50, 0);
    int shortage       = ProductionCalc::calcShortage(50, effectiveStock);

    CHECK(effectiveStock == 50);
    CHECK(shortage == 0);
}

TEST_CASE("stock 0 and qty 1 produces correct actualQty") {
    int effectiveStock = ProductionCalc::calcEffectiveStock(0, 0);
    int shortage       = ProductionCalc::calcShortage(1, effectiveStock);
    int actualQty      = ProductionCalc::calcActualQty(shortage, 0.9);

    CHECK(effectiveStock == 0);
    CHECK(shortage == 1);
    CHECK(actualQty == 2); // ceil(1 / 0.81)
}
