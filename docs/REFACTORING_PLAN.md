# REFACTORING_PLAN.md

## 개요

현재 `src/` 디렉토리에 30개 파일이 평탄하게 나열되어 있어 계층 파악이 어렵고,  
날짜 헬퍼·구분선 상수 등이 여러 파일에 중복되어 있다.  
리팩토링은 **작은 변경(메서드 레벨) → 중간 변경(클래스 레벨) → 큰 변경(디렉토리 레벨)** 순으로 진행한다.  
각 단계 완료 후 테스트(42개)를 실행하여 회귀가 없음을 확인한다.

---

## 현재 구조

```
SampleOrderSystem/src/          ← 30개 파일 평탄 나열
├── main.cpp
├── Models.h
├── DataStore.h / DataStore.cpp
├── Color.h
├── Utils.h
├── Validator.h
├── ProductionCalc.h
├── MainController.h / .cpp
├── SampleController.h / .cpp
├── SampleView.h / .cpp
├── OrderController.h / .cpp
├── OrderView.h / .cpp
├── ProductionController.h / .cpp
├── ProductionView.h / .cpp
├── MonitorController.h / .cpp
├── MonitorView.h / .cpp
├── ReleaseController.h / .cpp
└── ReleaseView.h / .cpp
```

---

## 목표 구조

```
SampleOrderSystem/src/
├── main.cpp
├── core/                        # 도메인 모델 + 공통 유틸
│   ├── Models.h
│   ├── Color.h
│   ├── Utils.h
│   ├── Validator.h
│   ├── ProductionCalc.h
│   └── DateTimeUtils.h          # (신규) 날짜·시간 헬퍼
├── repository/                  # 데이터 접근 레이어
│   ├── SampleRepository.h / .cpp
│   ├── OrderRepository.h / .cpp
│   ├── ProductionRepository.h / .cpp
│   └── DataStore.h / .cpp       # 3개 Repository 조합 Facade
├── sample/                      # [1] 시료 관리
│   ├── SampleController.h / .cpp
│   └── SampleView.h / .cpp
├── order/                       # [2][3] 주문 접수·승인/거절
│   ├── OrderController.h / .cpp
│   └── OrderView.h / .cpp
├── production/                  # [4] 생산라인
│   ├── ProductionController.h / .cpp
│   └── ProductionView.h / .cpp
├── monitor/                     # [5] 모니터링
│   ├── MonitorController.h / .cpp
│   └── MonitorView.h / .cpp
└── release/                     # [6] 출고 처리
    ├── ReleaseController.h / .cpp
    └── ReleaseView.h / .cpp
```

> `MainController.h/.cpp`는 전체를 조율하는 진입점으로 `src/` 루트에 유지한다.

---

## 단계별 계획

---

### R1. 날짜·시간 헬퍼 통합 (메서드 레벨)

**문제**

`OrderController`와 `ProductionController`에 유사한 날짜·시간 함수가 중복 정의되어 있다.

| 파일 | 중복 함수 |
|------|---------|
| `OrderController.cpp` | `currentDateStr()`, `currentDateCompact()`, `currentDateTimeStr()` |
| `ProductionController.cpp` | `nowStr()`, `elapsedMinutes()`, `addMinutes()` |

**해결**

헤더 전용 `core/DateTimeUtils.h` 네임스페이스로 추출한다.

```cpp
namespace DateTimeUtils {
    std::string nowDate();          // "YYYY-MM-DD"
    std::string nowDateCompact();   // "YYYYMMDD"
    std::string nowDateTime();      // "YYYY-MM-DD HH:MM:SS"
    double      elapsedMinutes(const std::string& startedAt, const std::string& now);
    std::string addMinutes(const std::string& dt, double minutes);
}
```

**작업 순서**
1. `DateTimeUtils.h` 작성
2. `OrderController.cpp`의 3개 private static 함수를 `DateTimeUtils::` 호출로 교체, 헤더에서 선언 제거
3. `ProductionController.cpp`의 3개 private static 함수를 `DateTimeUtils::` 호출로 교체, 헤더에서 선언 제거
4. 빌드 + 테스트 통과 확인

---

### R2. 구분선 상수 중복 제거 (메서드 레벨)

**문제**

`static const std::string SEP = "----..."` 선언이 View 파일 4곳에 중복된다.

| 파일 | 중복 선언 |
|------|---------|
| `OrderView.cpp` | `static const std::string SEP = "----..."` |
| `ReleaseView.cpp` | 동일 |
| `MonitorView.cpp` | 동일 |
| `SampleView.cpp` | `SEP_THIN`, `SEP_THICK` 두 가지 변형 |

**해결**

`core/UI.h`를 신규 생성하고 공통 상수를 정의한다.

```cpp
namespace UI {
    inline const std::string SEP_THICK = "============================================================";
    inline const std::string SEP_THIN  = "------------------------------------------------------------";
}
```

**작업 순서**
1. `UI.h` 작성
2. 각 View `.cpp`에서 로컬 `SEP` 선언 제거 후 `UI::SEP_THIN` 참조로 교체
3. `SampleView.cpp`의 `SEP_THICK` / `SEP_THIN` 로컬 선언 제거
4. 빌드 + 테스트 통과 확인

---

### R3. MainController::showSummary() 개선 (메서드 레벨)

**문제**

`showSummary()`가 대기 주문 수를 구하기 위해 `GetReservedOrders()`를 호출한다.  
이 메서드는 전체 주문을 순회하여 RESERVED 목록을 벡터로 복사 후 반환하는데,  
`showSummary()`는 그 `.size()`만 필요로 한다.  
`GetOrderStatusSummary()`가 이미 존재하므로 재사용한다.

```cpp
// 현재
int reserved = (int)dataStore_.GetReservedOrders().size();

// 개선 후
auto summary = dataStore_.GetOrderStatusSummary();
int reserved = summary.reserved;
```

**작업 순서**
1. `MainController.cpp`의 `showSummary()` 수정
2. 빌드 + 테스트 통과 확인

---

### R4. DataStore 분리 — Repository 패턴 (클래스 레벨)

**문제**

`DataStore`가 시료·주문·생산 큐를 모두 관리하는 God Object다.  
단일 책임 원칙(SRP) 위반이며 테스트 격리도 어렵다.

**해결**

역할별 Repository로 분리한다. `DataStore`는 3개를 조합하는 Facade로 유지하여  
기존 Controller 코드의 호출부를 변경하지 않아도 되도록 한다.

```
SampleRepository   ← 시료 CRUD, nextId, GenerateSampleId
OrderRepository    ← 주문 CRUD, nextOrderId
ProductionRepository ← 생산 큐 CRUD, GetQueuedQuantityForSample
DataStore (Facade) ← 위 3개 인스턴스 소유, 기존 public API 위임
```

**각 Repository 책임**

| 클래스 | 멤버 | 파일 |
|--------|------|------|
| `SampleRepository` | `samples_`, `nextId_`, CRUD, `GetTotalStock`, `GetStockStatus` | `repository/SampleRepository.h/.cpp` |
| `OrderRepository` | `orders_`, `nextOrderId_`, CRUD, `GetOrderStatusSummary` | `repository/OrderRepository.h/.cpp` |
| `ProductionRepository` | `queue_`, CRUD, `GetQueuedQuantityForSample` | `repository/ProductionRepository.h/.cpp` |
| `DataStore` | 3개 repo 인스턴스, `Load()`, 기존 API 위임 | `repository/DataStore.h/.cpp` |

**작업 순서**
1. `SampleRepository` 작성 및 테스트 (DataStore의 샘플 관련 메서드 이동)
2. `OrderRepository` 작성 및 테스트
3. `ProductionRepository` 작성 및 테스트
4. `DataStore`를 Facade로 리팩토링 (위임 호출로 교체)
5. 기존 Controller는 `DataStore`를 그대로 사용 — 호출부 변경 없음
6. 빌드 + 테스트(42개) 통과 확인

> **주의**: 각 Repository를 독립적으로 단위 테스트할 수 있도록  
> 테스트 파일 `test_sample_repository.cpp`, `test_order_repository.cpp`,  
> `test_production_repository.cpp`를 추가한다.

---

### R5. 디렉토리 구조 재편 (디렉토리 레벨)

**문제**

30개 파일이 `src/` 하나에 평탄하게 나열되어 있어  
레이어 구조가 디렉토리에 반영되지 않는다.

**해결**

기능 도메인별 서브디렉토리로 분리한다.

**이동 목록**

| 파일 | 이동 대상 |
|------|---------|
| `Models.h`, `Color.h`, `Utils.h`, `Validator.h`, `ProductionCalc.h`, `DateTimeUtils.h` (신규), `UI.h` (신규) | `src/core/` |
| `DataStore.h/.cpp`, `SampleRepository.h/.cpp`, `OrderRepository.h/.cpp`, `ProductionRepository.h/.cpp` | `src/repository/` |
| `SampleController.h/.cpp`, `SampleView.h/.cpp` | `src/sample/` |
| `OrderController.h/.cpp`, `OrderView.h/.cpp` | `src/order/` |
| `ProductionController.h/.cpp`, `ProductionView.h/.cpp` | `src/production/` |
| `MonitorController.h/.cpp`, `MonitorView.h/.cpp` | `src/monitor/` |
| `ReleaseController.h/.cpp`, `ReleaseView.h/.cpp` | `src/release/` |
| `MainController.h/.cpp`, `main.cpp` | `src/` (루트 유지) |

**필요 후속 작업**
1. 모든 `#include "..."` 경로를 새 위치에 맞게 수정
   - 예: `#include "Models.h"` → `#include "core/Models.h"`
   - 또는 `.vcxproj`의 `AdditionalIncludeDirectories`에 각 서브디렉토리 추가
2. `SampleOrderSystem.vcxproj`의 `<ClCompile>`, `<ClInclude>` 항목 경로 전량 수정
3. `SampleOrderSystem.Tests.vcxproj`의 소스 경로 수정
4. 빌드 + 테스트(42개) 통과 확인

**include 경로 전략 (두 가지 중 선택)**

| 전략 | 방법 | 장점 | 단점 |
|------|------|------|------|
| A. 상대 경로 수정 | `#include "core/Models.h"` | 경로가 명시적 | 모든 include 문 수정 필요 |
| B. vcxproj 경로 추가 | `AdditionalIncludeDirectories`에 서브디렉토리 열거 | include 문 수정 없음 | 디렉토리 구조가 include에서 보이지 않음 |

→ **A 전략 권장**: 파일 위치가 include 경로에 드러나 가독성이 높아진다.

---

## 실행 순서 요약

| 단계 | 변경 범위 | 예상 작업량 | 테스트 확인 |
|------|---------|-----------|-----------|
| R1. 날짜·시간 헬퍼 통합 | 메서드 | S | ✅ |
| R2. SEP 상수 중복 제거 | 메서드 | S | ✅ |
| R3. showSummary() 개선 | 메서드 | XS | ✅ |
| R4. DataStore → Repository 분리 | 클래스 | L | ✅ |
| R5. 디렉토리 구조 재편 | 프로젝트 전체 | XL | ✅ |

> 각 단계는 독립적인 커밋으로 관리하여 문제 발생 시 단계별 롤백이 가능하도록 한다.

---

## 불변 조건

리팩토링 전 기간 동안 아래 조건을 항상 만족해야 한다.

- 테스트 42개 전부 GREEN 유지
- 기존 public API(`DataStore` 메서드 시그니처) 변경 없음 (R4까지)
- 빌드 오류·경고 없음
- 기능 동작 변경 없음
