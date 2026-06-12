# Phase 4 TDD 계획 — 출고 처리 + 모니터링

## 개요

PRD 및 기능 명세를 기반으로 Phase 4 기능을 TDD(Red-Green-Refactor) 방식으로 구현한다.

**대상 기능**
- [6] 출고 처리: CONFIRMED 상태 주문을 RELEASE로 전환
- [5] 모니터링: 상태별 주문 건수 조회, 시료별 재고 상태 표시

---

## 테스트 프레임워크 선택

**doctest v2.4.11** (단일 헤더)

선택 이유:
- 단일 헤더 파일 하나만 추가하면 되는 구성 (nlohmann/json과 동일한 방식)
- `test_main.cpp`에 `DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN` 한 줄로 실행 진입점 설정 가능
- 별도 링크 설정 불필요, 기존 프로젝트 스타일과 일관됨

**설치**
```powershell
Invoke-WebRequest -Uri "https://raw.githubusercontent.com/doctest/doctest/v2.4.11/doctest/doctest.h" `
    -OutFile "SampleOrderSystem\include\doctest\doctest.h"
```

**테스트 프로젝트 구성**
```
SampleOrderSystem.Tests/
├── SampleOrderSystem.Tests.vcxproj   # 메인 프로젝트의 include/src 경로 참조
└── tests/
    ├── test_main.cpp                 # DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
    ├── test_release.cpp              # 출고 처리 테스트
    └── test_monitoring.cpp           # 모니터링 테스트
```

`.vcxproj` 핵심 설정:
- `AdditionalIncludeDirectories`: `..\SampleOrderSystem\include;..\SampleOrderSystem\src`
- 컴파일 대상: `DataStore.cpp` + 각 test 파일 (`main.cpp` 제외)
- 메인 프로젝트와 동일 toolset (v145), C++17, `/utf-8`

---

## 요구사항 분석

### 출고 처리 (feature-release.md)

| 항목 | 내용 |
|------|------|
| 입력 | CONFIRMED 상태 주문 목록 표시 → 사용자 선택 |
| 처리 | 선택한 주문의 상태를 CONFIRMED → RELEASE로 변경 |
| **재고 차감 규칙** | 승인 즉시 확정(직접 Confirmed) 경로: 승인 시점에 재고 차감 → 출고 시 차감 없음 |
| | 생산 경로(Producing → Confirmed): 승인 시 차감 없음, 생산 완료 시 actualQty 증가 → **출고 시 주문 수량만큼 차감** |
| 반환 | 출고 완료 메시지 표시 |

> **경로 구분:** `OrderData.producingBased` 필드(bool)로 구분. 생산 경로로 전환 시 `true`로 설정, 직접 Confirmed는 기본값 `false` 유지.

### 모니터링 (feature-monitoring.md + PRD)

| 항목 | 내용 |
|------|------|
| 주문 현황 | RESERVED / PRODUCING / CONFIRMED / RELEASE 건수 집계 |
| 제외 | REJECTED는 집계에서 제외 |
| 재고 현황 | 시료별 재고 수량 + 재고 상태 표시 |
| 재고 상태 기준 | 여유: RESERVED 주문 대비 재고 충분 / 부족: 재고 < RESERVED 주문 합계 / 고갈: 재고 == 0 |

---

## 새 API 설계 (DataStore)

테스트 작성 전에 원하는 API를 먼저 설계한다.

### Models.h에 추가

```cpp
enum class StockStatus { Plenty, Short, Depleted };

struct OrderStatusSummary {
    int reserved  = 0;
    int producing = 0;
    int confirmed = 0;
    int release   = 0;
};
```

### DataStore에 추가

```cpp
// 출고 처리
std::vector<OrderData> GetConfirmedOrders() const;
bool                   ReleaseOrder(int orderId);

// 모니터링
OrderStatusSummary     GetOrderStatusSummary() const;
StockStatus            GetStockStatus(const std::string& sampleId) const;
```

---

## TDD 사이클

### 사이클 1 — 출고 처리: GetConfirmedOrders

#### RED

```cpp
TEST_CASE("GetConfirmedOrders returns only CONFIRMED orders") {
    // CONFIRMED 1개, Reserved 1개, Release 1개, Producing 1개 추가
    auto confirmed = ds.GetConfirmedOrders();
    CHECK(confirmed.size() == 1);
    CHECK(confirmed[0].status == OrderStatus::Confirmed);
}

TEST_CASE("GetConfirmedOrders returns empty when no CONFIRMED orders exist") {
    // Reserved, Rejected만 존재
    CHECK(ds.GetConfirmedOrders().empty());
}
```

**예상 컴파일 오류**: `'GetConfirmedOrders': 'DataStore'의 멤버가 아닙니다`

#### GREEN

```cpp
std::vector<OrderData> DataStore::GetConfirmedOrders() const {
    std::vector<OrderData> result;
    for (const auto& o : orders_)
        if (o.status == OrderStatus::Confirmed)
            result.push_back(o);
    return result;
}
```

---

### 사이클 2 — 출고 처리: ReleaseOrder

#### RED

```cpp
TEST_CASE("ReleaseOrder changes CONFIRMED order status to RELEASE") { ... }

// 직접 Confirmed 경로: 재고 불변
TEST_CASE("ReleaseOrder does not change stock for direct-confirmed order") {
    // producingBased = false (기본값), 재고 50 → 출고 후 50 유지
}

// 생산 경로: 출고 시 quantity 차감
TEST_CASE("ReleaseOrder deducts order quantity from stock for production-based order") {
    // producingBased = true, 재고 11(생산 완료 후), 주문 10
    // 출고 후 재고 = 11 - 10 = 1
}

TEST_CASE("ReleaseOrder returns false when order does not exist") { ... }
TEST_CASE("ReleaseOrder returns false when order is not CONFIRMED") { ... }
```

#### GREEN

```cpp
bool DataStore::ReleaseOrder(int orderId) {
    auto order = FindOrderById(orderId);          // 주문 정보 선조회
    if (!order) return false;
    bool ok = orderRepo_.Release(orderId);        // 상태 RELEASE로 전환
    if (ok && order->producingBased)              // 생산 경로였다면
        sampleRepo_.UpdateStock(order->sampleId, -order->quantity); // 재고 차감
    return ok;
}
```

---

### 사이클 3 — 모니터링: GetOrderStatusSummary

#### RED

```cpp
TEST_CASE("GetOrderStatusSummary counts orders by status, excluding REJECTED") {
    // RESERVED 2, PRODUCING 1, CONFIRMED 1, RELEASE 1, REJECTED 2 추가
    auto summary = ds.GetOrderStatusSummary();
    CHECK(summary.reserved  == 2);
    CHECK(summary.producing == 1);
    CHECK(summary.confirmed == 1);
    CHECK(summary.release   == 1);
    // REJECTED는 집계되지 않음 (필드 없음)
}

TEST_CASE("GetOrderStatusSummary returns all zeros when only REJECTED orders exist") {
    auto summary = ds.GetOrderStatusSummary();
    CHECK(summary.reserved  == 0);
    CHECK(summary.producing == 0);
    CHECK(summary.confirmed == 0);
    CHECK(summary.release   == 0);
}
```

**예상 컴파일 오류**: `'OrderStatusSummary': 식별자를 찾을 수 없습니다`, `'GetOrderStatusSummary': 'DataStore'의 멤버가 아닙니다`

#### GREEN

```cpp
OrderStatusSummary DataStore::GetOrderStatusSummary() const {
    OrderStatusSummary summary;
    for (const auto& o : orders_) {
        switch (o.status) {
        case OrderStatus::Reserved:  summary.reserved++;  break;
        case OrderStatus::Producing: summary.producing++; break;
        case OrderStatus::Confirmed: summary.confirmed++; break;
        case OrderStatus::Release:   summary.release++;   break;
        default: break; // Rejected 제외
        }
    }
    return summary;
}
```

---

### 사이클 4 — 모니터링: GetStockStatus

#### RED

```cpp
TEST_CASE("GetStockStatus returns Depleted when stock is 0") {
    // 재고 0인 시료
    CHECK(ds.GetStockStatus("01") == StockStatus::Depleted);
}

TEST_CASE("GetStockStatus returns Short when RESERVED orders exceed stock") {
    // 재고 5, RESERVED 주문 수량 10
    CHECK(ds.GetStockStatus("01") == StockStatus::Short);
}

TEST_CASE("GetStockStatus returns Plenty when stock covers all RESERVED orders") {
    // 재고 20, RESERVED 주문 합계 15
    CHECK(ds.GetStockStatus("01") == StockStatus::Plenty);
}

TEST_CASE("GetStockStatus returns Plenty when no RESERVED orders exist for sample") {
    // CONFIRMED, PRODUCING 주문만 존재 → RESERVED가 없으면 여유
    CHECK(ds.GetStockStatus("01") == StockStatus::Plenty);
}

TEST_CASE("GetStockStatus ignores other samples RESERVED orders") {
    // "01" 시료 재고 5, "02" 시료에 RESERVED 주문 20 → "01"은 여유
    CHECK(ds.GetStockStatus("01") == StockStatus::Plenty);
}
```

**예상 컴파일 오류**: `'StockStatus': 클래스 또는 네임스페이스 이름이 아닙니다`, `'GetStockStatus': 'DataStore'의 멤버가 아닙니다`

#### GREEN

```cpp
StockStatus DataStore::GetStockStatus(const std::string& sampleId) const {
    auto sample = FindSampleById(sampleId);
    if (!sample) return StockStatus::Depleted;
    if (sample->stock == 0) return StockStatus::Depleted;

    int reservedTotal = 0;
    for (const auto& o : orders_)
        if (o.sampleId == sampleId && o.status == OrderStatus::Reserved)
            reservedTotal += o.quantity;

    return (sample->stock < reservedTotal) ? StockStatus::Short : StockStatus::Plenty;
}
```

---

## 최종 테스트 결과

```
[doctest] test cases: 13 | 13 passed | 0 failed | 0 skipped
[doctest] assertions: 25 | 25 passed | 0 failed |
[doctest] Status: SUCCESS!
```

| 파일 | 테스트 케이스 | 검증 내용 |
|------|--------------|---------|
| `test_release.cpp` | 5개 | `GetConfirmedOrders` 필터링, `ReleaseOrder` 상태 전환, 재고 불변, 실패 케이스 |
| `test_monitoring.cpp` | 8개 | `GetOrderStatusSummary` REJECTED 제외 집계, `GetStockStatus` 고갈/부족/여유 판정 |

---

## Controller / View 구현 (TDD 범위 외)

콘솔 I/O에 종속된 Controller/View 레이어는 단위 테스트 작성이 어려워 DataStore 비즈니스 로직 테스트로 대체한다.

| 클래스 | 역할 |
|--------|------|
| `ReleaseController` | `GetConfirmedOrders()` → 목록 표시 → `ReleaseOrder()` 호출 |
| `ReleaseView` | CONFIRMED 목록, 출고 완료 결과 출력 |
| `MonitorController` | `GetOrderStatusSummary()` + `GetStockStatus()` 결과 표시 |
| `MonitorView` | 주문 현황(건수) + 재고 현황(상태 색상) 출력 |

`MainController`에서 메뉴 5번(모니터링), 6번(출고 처리) 활성화.

---

## 설계 결정 사항

| 결정 | 이유 |
|------|------|
| 직접 Confirmed: 출고 시 재고 차감 없음 | 승인 시점에 이미 차감됨 (재고 충분 → 즉시 확정 경로) |
| 생산 Confirmed: 출고 시 `quantity` 차감 | 승인 시 재고 미차감, 생산 완료 후 actualQty 증가 → 출고 시 주문량 차감 필요 |
| `OrderData.producingBased` 필드로 경로 구분 | 출고 시점에 두 경로를 구분하는 최소 변경. 생산 큐 소비 후에도 이력 보존 |
| `GetStockStatus`는 RESERVED 주문만 비교 | CONFIRMED·PRODUCING은 이미 처리된 주문 — 현재 재고에 영향 없음 |
| `StockStatus::Depleted`를 최우선 판정 | 재고 0이면 RESERVED 주문 여부와 무관하게 고갈 |
| doctest 선택 | 단일 헤더, 프로젝트 기존 스타일(nlohmann/json)과 일치 |
