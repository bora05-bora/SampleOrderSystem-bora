#include "doctest/doctest.h"
#include "Validator.h"

// ── 사이클 1: isPositiveInt ───────────────────────────────

TEST_CASE("isPositiveInt accepts values >= 1") {
    CHECK(Validator::isPositiveInt(1)   == true);
    CHECK(Validator::isPositiveInt(100) == true);
}

TEST_CASE("isPositiveInt rejects zero") {
    CHECK(Validator::isPositiveInt(0) == false);
}

TEST_CASE("isPositiveInt rejects negative values") {
    CHECK(Validator::isPositiveInt(-1)  == false);
    CHECK(Validator::isPositiveInt(-99) == false);
}

// ── 사이클 2: isPositiveDouble ───────────────────────────

TEST_CASE("isPositiveDouble accepts values > 0.0") {
    CHECK(Validator::isPositiveDouble(0.1)  == true);
    CHECK(Validator::isPositiveDouble(10.0) == true);
}

TEST_CASE("isPositiveDouble rejects zero") {
    CHECK(Validator::isPositiveDouble(0.0) == false);
}

TEST_CASE("isPositiveDouble rejects negative values") {
    CHECK(Validator::isPositiveDouble(-0.1) == false);
}

// ── 사이클 3: isValidYield ───────────────────────────────

TEST_CASE("isValidYield accepts values in range (0.0, 1.0]") {
    CHECK(Validator::isValidYield(0.01) == true);
    CHECK(Validator::isValidYield(0.5)  == true);
    CHECK(Validator::isValidYield(1.0)  == true);
}

TEST_CASE("isValidYield rejects zero") {
    CHECK(Validator::isValidYield(0.0) == false);
}

TEST_CASE("isValidYield rejects values above 1.0") {
    CHECK(Validator::isValidYield(1.01) == false);
    CHECK(Validator::isValidYield(2.0)  == false);
}

TEST_CASE("isValidYield rejects negative values") {
    CHECK(Validator::isValidYield(-0.5) == false);
}

// ── 사이클 4: isNonEmpty ─────────────────────────────────

TEST_CASE("isNonEmpty accepts non-blank strings") {
    CHECK(Validator::isNonEmpty("Alpha") == true);
    CHECK(Validator::isNonEmpty("A")     == true);
}

TEST_CASE("isNonEmpty rejects empty string") {
    CHECK(Validator::isNonEmpty("") == false);
}

TEST_CASE("isNonEmpty rejects whitespace-only strings") {
    CHECK(Validator::isNonEmpty(" ")   == false);
    CHECK(Validator::isNonEmpty("   ") == false);
    CHECK(Validator::isNonEmpty("\t")  == false);
}
