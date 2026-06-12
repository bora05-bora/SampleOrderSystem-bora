# SampleOrderSystem — CLAUDE.md

## 프로젝트 개요

**S-Semi 반도체 회사**의 **반도체 시료 생산주문관리 시스템**입니다.

다양한 종류의 반도체 시료(Sample)를 생산하는 환경에서 다음 항목들을 통합 관리합니다.

- **주문 상태 관리**: 주문 상태 조회
- **재고 관리**: 시료별 재고 수량 조회·갱신
- **공정 현황 관리**: 생산 진행 상태(RESERVED/PRODUCING/CONFIRMED/RELEASE) 추적
- **시료 관리**: 시료 정보(수율, 생산시간 등) CRUD

주문 요청, 승인, 거절, 출고 기능도 제공합니다.

콘솔 기반 C++ 애플리케이션으로 구현하며, 데이터는 JSON 파일로 영속화합니다.

---

## 빌드 환경

| 항목 | 값 |
|------|-----|
| IDE | Visual Studio (솔루션 파일 기반) |
| 빌드 도구 | MSBuild |
| PlatformToolset | **v145** (Visual Studio 2022) |
| C++ 표준 | **C++17** |
| 플랫폼 | Windows x64 |
| SDK | Windows 10 SDK |

### 빌드 명령

```powershell
# Debug 빌드
msbuild SampleOrderSystem.sln /p:Configuration=Debug /p:Platform=x64

# Release 빌드
msbuild SampleOrderSystem.sln /p:Configuration=Release /p:Platform=x64
```

### 외부 의존성

- **nlohmann/json** (단일 헤더, `include/nlohmann/json.hpp`): JSON 직렬화/역직렬화
  - `setup_deps.ps1` PowerShell 스크립트로 다운로드
- `<windows.h>`: 콘솔 컬러 출력 (NOMINMAX 정의 필수)

---

## 아키텍처

### 레이어 구조 (MVC + Repository)

```
main.cpp
  └── Controller (비즈니스 로직 조정)
        ├── View (콘솔 입출력)
        └── Repository → DataStore (JSON 파일 I/O)
                              └── data/*.json
```

### 계층별 책임

| 계층 | 클래스 예시 | 책임 |
|------|------------|------|
| Model | `SampleData`, `OrderData` | 데이터 구조 + `toJson()`/`fromJson()` |
| Repository / DataStore | `SampleRepository`, `OrderRepository` | CRUD + JSON 영속화 |
| Controller | `SampleController`, `OrderController` | 메뉴 분기, 비즈니스 규칙 |
| View | `SampleView`, `OrderView` | 입력 수집, 화면 출력 |
| Monitor | `Monitor` | 임계값 기반 알림 생성 |

---

## 핵심 도메인 모델

### OrderStatus (주문 상태)

```cpp
enum class OrderStatus { Reserved, Rejected, Producing, Confirmed, Release };
```

| 값 | 의미 |
|----|------|
| Reserved | 주문 접수 (최초 등록) |
| Rejected | 주문 거절 (모니터링 제외) |
| Producing | 승인 완료 + 재고 부족으로 생산 중 |
| Confirmed | 승인 완료 + 출고 대기 중 |
| Release | 출고 완료 |

### SampleData (시료)

```cpp
struct SampleData {
    int id;
    std::string name;       // 시료명 (예: "Alpha-001")
    int stock;              // 재고 수량
    double productionTime;  // 생산 소요 시간 (시간 단위)
    double yield;           // 수율 (%)
    // toJson(), fromJson()
};
```

### OrderData (주문)

```cpp
struct OrderData {
    int id;
    std::string orderNo;    // 주문번호 (예: ORD-20260416-0043)
    std::string sample;     // 시료명 (SampleData.name 참조)
    std::string customer;   // 고객명
    int quantity;           // 주문 수량
    OrderStatus status;
    std::string date;       // "YYYY-MM-DD"
    // toJson(), fromJson()
};
```

---

## 데이터 영속화

JSON 파일 2개로 분리 저장합니다.

```
data/
├── orders.json     # 주문 목록
└── samples.json    # 시료 목록
```

### 파일 스키마

```json
// orders.json
{
  "nextId": 3,
  "orders": [
    {"id": 1, "sample": "Alpha-001", "quantity": 87, "status": "Pending", "date": "2026-03-15"}
  ]
}

// samples.json
{
  "nextId": 3,
  "samples": [
    {"id": 1, "name": "Alpha-001", "stock": 187, "productionTime": 12.5, "yield": 86.5}
  ]
}
```

- CRUD 작업마다 즉시 저장 (데이터 손실 방지)
- 프로그램 시작 시 `load()`로 복원
- `std::filesystem`으로 `data/` 폴더 자동 생성

---

## 모니터링 재고 상태 기준

모니터링 화면에서 시료별 재고 상태를 표기합니다. (PRD 기준)

| 상태 | 조건 |
|------|------|
| 여유 | 주문 대비 재고 충분 |
| 부족 | 주문 대비 재고 수량 부족 |
| 고갈 | 재고 수량 == 0 |

모니터링 주문량 집계는 RESERVED / PRODUCING / CONFIRMED / RELEASE 상태만 포함하며 **REJECTED는 제외**합니다.

---

## 코드 컨벤션

POC 저장소들의 컨벤션을 통합하여 아래 규칙을 따릅니다.

### 네이밍

| 대상 | 규칙 | 예시 |
|------|------|------|
| 클래스/구조체 | PascalCase | `OrderController`, `SampleData` |
| enum class | PascalCase 값 | `OrderStatus::Pending` |
| public 메서드 | PascalCase | `FindById`, `UpdateStock` |
| private 메서드 | camelCase | `loadOrders`, `nextOrderId` |
| 멤버 변수 | `_` 접미사 | `orders_`, `dataDir_` |
| 상수 | UPPER_SNAKE_CASE | `LOW_STOCK_THRESHOLD` |

### 기타 규칙

- 헤더 가드: `#pragma once`
- 한글 주석 허용 (메뉴 텍스트, UI 레이블)
- `NOMINMAX` 매크로 정의 (`<windows.h>` 사용 시)
- `std::optional` 활용 (단건 조회 반환값)
- 콘솔 컬러: `Color::set()` / `Color::reset()` 헬퍼 네임스페이스

---

## 주요 문서

| 문서 | 경로 | 설명 |
|------|------|------|
| PRD | `docs/PRD.md` | 시스템 전체 요구사항 (배경, 사용자 역할, 주문 상태 흐름, 기능 목록) |
| 기능 명세 | `docs/features/` | 기능별 상세 요구사항 (시료 관리, 주문, 승인/거절, 모니터링, 생산라인, 출고) |
| 개발 계획 | `docs/PLAN.md` | Phase별 개발 목표 — 각 Phase 종료 시점에 동작하는 SW를 기준으로 목표를 정의하며, 고객 테스트 피드백 루프를 기반으로 진행 |

---

## 참조 POC 저장소

본 시스템은 아래 4개 POC를 참고하여 구현합니다.

| 저장소 | 목적 | 핵심 참조 내용 |
|--------|------|---------------|
| [ConsoleMVC-bora](https://github.com/bora05-bora/ConsoleMVC-bora) | MVC 아키텍처 기초 | Model/View/Controller/Repository 분리 구조, 메뉴 루프 패턴 |
| [DataPersistence-bora](https://github.com/bora05-bora/DataPersistence-bora) | JSON 영속화 | nlohmann/json CRUD, 즉시 저장 패턴, `std::filesystem` 폴더 생성 |
| [DummyDataGenerator-bora](https://github.com/bora05-bora/DummyDataGenerator-bora) | 테스트 데이터 생성 | `OrderData`/`SampleData` 스키마, `OrderStatus` enum, CLI 생성 파라미터 |
| [DataMonitor-bora](https://github.com/bora05-bora/DataMonitor-bora) | 모니터링 및 알림 | `DataStore` CRUD, `Monitor` 임계값 체크, `Alert` struct, Windows 콘솔 컬러 |

### 각 POC의 주요 클래스 참조

**ConsoleMVC-bora**
- `UserModel` → `SampleData`, `OrderData` 모델링 참조
- `UserRepository` → Repository CRUD 인터페이스 참조
- `UserController::Run()` → 메인 메뉴 루프 구조 참조

**DataPersistence-bora**
- `DataManager` → `Save()`/`Load()` 즉시 저장 패턴
- `data/records.json` → JSON 파일 스키마 설계 참조
- `std::filesystem` 경로 처리 참조

**DummyDataGenerator-bora**
- `DataModels.h` → `OrderData`/`SampleData` 구조체 및 `OrderStatus` enum 설계
- `setup_deps.ps1` → 의존성 설치 스크립트 패턴
- `OrderGenerator`/`SampleGenerator` → 테스트 데이터 자동 생성 (개발·테스트용)

**DataMonitor-bora**
- `DataStore` → `orders_`/`samples_` 분리 저장, `updateStock()` 참조
- `Monitor` → 임계값 상수, `runAll()` → `std::vector<Alert>` 반환 패턴
- `main.cpp` → Windows 콘솔 컬러 적용, 메뉴 레이아웃 참조

---

## 디렉토리 구조 (예정)

```
SampleOrderSystem/
├── SampleOrderSystem.sln
├── SampleOrderSystem/
│   ├── SampleOrderSystem.vcxproj
│   ├── include/
│   │   └── nlohmann/json.hpp
│   ├── src/
│   │   ├── main.cpp
│   │   ├── Models.h                  # SampleData, OrderData, OrderStatus
│   │   ├── DataStore.h / .cpp        # JSON 파일 I/O, CRUD
│   │   ├── Monitor.h / .cpp          # 임계값 알림
│   │   ├── SampleController.h / .cpp
│   │   ├── SampleView.h / .cpp
│   │   ├── OrderController.h / .cpp
│   │   └── OrderView.h / .cpp
│   └── data/
│       ├── orders.json
│       └── samples.json
├── setup_deps.ps1                    # nlohmann/json 다운로드
├── CLAUDE.md
├── .gitignore
└── .gitattributes
```

---

## 개발 시 주의사항

- `<windows.h>` 포함 시 반드시 `#define NOMINMAX` 선언 (std::min/max 충돌 방지)
- JSON 파일 경로는 실행 파일 기준 상대 경로 사용 (`data/`)
- `std::filesystem::create_directories()`로 `data/` 폴더 자동 생성
- 단건 조회는 `std::optional<T>` 반환
- CRUD 작업 직후 즉시 파일 저장 (`saveOrders()` / `saveSamples()` 호출)
- 주문의 `sample` 필드는 `SampleData::name`을 외래 키로 사용
