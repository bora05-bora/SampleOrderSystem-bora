# Phase 5 TDD 계획 — 품질 완성 (안정성 · 사용성)

## 개요

Phase 5는 예외 상황에서의 견고성과 경계값 정확성을 다루는 Phase다.  
새 기능이 아닌 **기존 로직의 신뢰성**을 높이는 작업이므로 TDD가 가장 효과적으로 적용되는 시점이다.

**TDD 적용 범위**

| 항목 | 대상 | TDD 여부 |
|------|------|:--------:|
| 입력 유효성 검사 규칙 | `Validator` 클래스 (신규) | ✅ |
| 생산량·시간 계산 경계값 | `ProductionCalc` 네임스페이스 (신규) | ✅ |
| 재고 == 주문 수량 경계 | DataStore + 승인 흐름 | ✅ |
| 빈 목록 상태 반환 | DataStore (기존 메서드) | ✅ |
| 화면 구성 통일 | View 레이어 (콘솔 I/O) | ❌ 수동 확인 |

---

## 핵심 설계 방향

### 문제: 로직이 View 내부에 인라인으로 있음

현재 입력 검증(`while (true)` 루프)과 생산량 계산(`ceil`, `max`)이  
`SampleView`, `OrderView`, `OrderController` 안에 흩어져 있어 단위 테스트가 불가능하다.

### 해결: 순수 함수 레이어 분리

테스트 가능한 코드를 만들기 위해 두 개의 헤더 전용 유틸리티를 추출한다.

```
src/
├── Validator.h        # 입력값 검증 규칙 (순수 함수)
└── ProductionCalc.h   # 생산량 계산 공식 (순수 함수)
```

View와 Controller는 이 유틸리티를 호출하는 방식으로 리팩토링한다.  
기존 동작은 그대로 유지하면서 로직만 분리한다.

---

## 신규 API 설계

### Validator.h

```cpp
namespace Validator {
    bool isPositiveInt(int val);         // val >= 1
    bool isPositiveDouble(double val);   // val > 0.0
    bool isValidYield(double val);       // 0.0 < val <= 1.0
    bool isNonEmpty(const std::string& s); // trim 후 비어있지 않음
}
```

### ProductionCalc.h

```cpp
namespace ProductionCalc {
    int    calcEffectiveStock(int stock, int queuedQty);
    int    calcShortage(int quantity, int effectiveStock);
    int    calcActualQty(int shortage, double yield);
    double calcTotalTime(double productionTime, int actualQty);
}
```

---

## TDD 사이클

### 사이클 1 — Validator: isPositiveInt

**대상 규칙**: 주문 수량, 메뉴 번호 등 양의 정수 입력 검증

#### RED

```cpp
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
```

**예상 오류**: `'Validator': 네임스페이스가 아닙니다`

#### GREEN

```cpp
namespace Validator {
    inline bool isPositiveInt(int val) { return val >= 1; }
}
```

---

### 사이클 2 — Validator: isPositiveDouble

**대상 규칙**: 평균 생산시간 입력 검증 (0 초과 실수)

#### RED

```cpp
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
```

#### GREEN

```cpp
inline bool isPositiveDouble(double val) { return val > 0.0; }
```

---

### 사이클 3 — Validator: isValidYield

**대상 규칙**: 수율 입력 검증 (0.0 초과 ~ 1.0 이하)

#### RED

```cpp
TEST_CASE("isValidYield accepts values in range (0.0, 1.0]") {
    CHECK(Validator::isValidYield(0.01) == true);
    CHECK(Validator::isValidYield(0.5)  == true);
    CHECK(Validator::isValidYield(1.0)  == true); // 상한 포함
}

TEST_CASE("isValidYield rejects zero (lower bound exclusive)") {
    CHECK(Validator::isValidYield(0.0) == false);
}

TEST_CASE("isValidYield rejects values above 1.0") {
    CHECK(Validator::isValidYield(1.01) == false);
    CHECK(Validator::isValidYield(2.0)  == false);
}

TEST_CASE("isValidYield rejects negative values") {
    CHECK(Validator::isValidYield(-0.5) == false);
}
```

#### GREEN

```cpp
inline bool isValidYield(double val) { return val > 0.0 && val <= 1.0; }
```

---

### 사이클 4 — Validator: isNonEmpty

**대상 규칙**: 이름, 고객명 등 문자열 입력 검증

#### RED

```cpp
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
```

#### GREEN

```cpp
inline bool isNonEmpty(const std::string& s) {
    return s.find_first_not_of(" \t\r\n") != std::string::npos;
}
```

---

### 사이클 5 — ProductionCalc: calcEffectiveStock

**대상 규칙**: 유효 가용 재고 = max(0, 재고 - 생산 대기 동일 시료 주문량)

#### RED

```cpp
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
```

#### GREEN

```cpp
inline int calcEffectiveStock(int stock, int queuedQty) {
    return std::max(0, stock - queuedQty);
}
```

---

### 사이클 6 — ProductionCalc: calcShortage

**대상 규칙**: 부족분 = 주문 수량 - 유효 가용 재고

#### RED

```cpp
TEST_CASE("calcShortage returns difference when quantity exceeds effectiveStock") {
    CHECK(ProductionCalc::calcShortage(30, 10) == 20);
}

TEST_CASE("calcShortage returns 0 when effectiveStock covers quantity") {
    CHECK(ProductionCalc::calcShortage(10, 10) == 0); // 경계: 딱 맞음
    CHECK(ProductionCalc::calcShortage(10, 20) == 0); // 재고 여유
}
```

> **경계값**: `quantity == effectiveStock`일 때 부족분이 0 → CONFIRMED 분기 진입

#### GREEN

```cpp
inline int calcShortage(int quantity, int effectiveStock) {
    return std::max(0, quantity - effectiveStock);
}
```

---

### 사이클 7 — ProductionCalc: calcActualQty

**대상 규칙**: 실 생산량 = ceil(부족분 / (수율 × 0.9))

#### RED

```cpp
TEST_CASE("calcActualQty returns 0 when shortage is 0") {
    CHECK(ProductionCalc::calcActualQty(0, 0.9) == 0);
}

TEST_CASE("calcActualQty rounds up fractional result") {
    // ceil(1 / (0.9 * 0.9)) = ceil(1.234) = 2
    CHECK(ProductionCalc::calcActualQty(1, 0.9) == 2);
}

TEST_CASE("calcActualQty with yield 1.0 (100%)") {
    // ceil(9 / (1.0 * 0.9)) = ceil(10.0) = 10
    CHECK(ProductionCalc::calcActualQty(9, 1.0) == 10);
}

TEST_CASE("calcActualQty with low yield increases production significantly") {
    // ceil(10 / (0.5 * 0.9)) = ceil(22.22) = 23
    CHECK(ProductionCalc::calcActualQty(10, 0.5) == 23);
}

TEST_CASE("calcActualQty with stock exactly 0 and qty 1") {
    // 재고 0, 주문 1 → 부족분 1, yield=0.9 → ceil(1/0.81) = 2
    CHECK(ProductionCalc::calcActualQty(1, 0.9) == 2);
}
```

#### GREEN

```cpp
inline int calcActualQty(int shortage, double yield) {
    if (shortage <= 0) return 0;
    return (int)std::ceil((double)shortage / (yield * 0.9));
}
```

---

### 사이클 8 — ProductionCalc: calcTotalTime

**대상 규칙**: 총 생산시간 = 평균 생산시간 × 실 생산량

#### RED

```cpp
TEST_CASE("calcTotalTime multiplies productionTime by actualQty") {
    CHECK(ProductionCalc::calcTotalTime(10.0, 5) == doctest::Approx(50.0));
}

TEST_CASE("calcTotalTime returns 0 when actualQty is 0") {
    CHECK(ProductionCalc::calcTotalTime(10.0, 0) == doctest::Approx(0.0));
}

TEST_CASE("calcTotalTime handles fractional productionTime") {
    CHECK(ProductionCalc::calcTotalTime(2.5, 4) == doctest::Approx(10.0));
}
```

#### GREEN

```cpp
inline double calcTotalTime(double productionTime, int actualQty) {
    return productionTime * actualQty;
}
```

---

### 사이클 9 — 경계값 통합: 재고 == 주문 수량 → CONFIRMED 전환

위 `calcShortage` + `calcEffectiveStock` 함수를 사용한 통합 시나리오 테스트

#### RED

```cpp
TEST_CASE("approval with stock exactly equal to quantity results in CONFIRMED") {
    // 재고 50, 대기 0 → effectiveStock 50, 수량 50 → shortage 0 → CONFIRMED
    int stock = 50, queuedQty = 0, quantity = 50;
    int effectiveStock = ProductionCalc::calcEffectiveStock(stock, queuedQty);
    int shortage = ProductionCalc::calcShortage(quantity, effectiveStock);

    CHECK(effectiveStock == 50);
    CHECK(shortage == 0);
    // shortage == 0 → CONFIRMED 분기, 재고 -= quantity → 결과 재고 0
}

TEST_CASE("approval with stock 0 and qty 1 results in PRODUCING") {
    int stock = 0, queuedQty = 0, quantity = 1;
    int effectiveStock = ProductionCalc::calcEffectiveStock(stock, queuedQty);
    int shortage = ProductionCalc::calcShortage(quantity, effectiveStock);
    int actualQty = ProductionCalc::calcActualQty(shortage, 0.9);

    CHECK(effectiveStock == 0);
    CHECK(shortage == 1);
    CHECK(actualQty == 2); // ceil(1/0.81)
}
```

---

## 테스트 파일 구성

```
SampleOrderSystem.Tests/tests/
├── test_main.cpp          # (기존)
├── test_release.cpp       # (기존)
├── test_monitoring.cpp    # (기존)
├── test_validator.cpp     # (신규) Validator 검증 규칙
└── test_production_calc.cpp  # (신규) ProductionCalc 계산 공식
```

테스트 프로젝트 `.vcxproj`에 두 파일 추가 (DataStore.cpp 외 소스 추가 없음).

---

## 리팩토링 계획 (GREEN 이후)

### Validator 적용 대상

| 파일 | 변경 내용 |
|------|---------|
| `SampleView.cpp` | 생산시간/수율 입력 루프에 `Validator::isPositiveDouble`, `isValidYield` 적용 |
| `SampleView.cpp` | 이름 입력에 `Validator::isNonEmpty` 적용 |
| `OrderView.cpp` | 수량 입력에 `Validator::isPositiveInt` 적용 |

### ProductionCalc 적용 대상

| 파일 | 변경 내용 |
|------|---------|
| `OrderController.cpp` | `effectiveStock`, `shortage`, `actualQty`, `totalTime` 계산을 `ProductionCalc::` 함수로 교체 |

> 기존 동작 변경 없이 함수 호출만 교체 — 테스트가 회귀를 방어

---

## 예상 최종 테스트 수

| 파일 | 케이스 수 |
|------|---------|
| `test_validator.cpp` | 약 14개 |
| `test_production_calc.cpp` | 약 11개 |
| 기존 13개 유지 | 13개 |
| **합계** | **약 38개** |

---

## 완료 조건

- [ ] `Validator` 모든 케이스 GREEN
- [ ] `ProductionCalc` 모든 케이스 GREEN
- [ ] 기존 13개 테스트 회귀 없이 GREEN 유지
- [ ] `OrderController`가 `ProductionCalc` 함수를 사용하도록 리팩토링
- [ ] View 입력 루프에 `Validator` 적용
- [ ] 빌드 오류/경고 없음
