# SampleOrderSystem

S-Semi 반도체 회사의 **반도체 시료 생산주문관리 시스템**입니다.

시료 등록부터 주문 접수, 승인/거절, 생산라인 관리, 출고 처리, 재고 모니터링까지 주문 전 흐름을 콘솔에서 통합 관리합니다.

---

## 주요 기능

| 메뉴 | 기능 |
|------|------|
| [1] 시료 관리 | 시료 등록, 목록 조회, 이름 검색 |
| [2] 시료 주문 | 고객 주문 접수 (RESERVED 상태 생성) |
| [3] 주문 승인/거절 | RESERVED 주문 승인(재고 자동 분기) 또는 거절 |
| [4] 생산라인 조회 | 생산 중 시료 및 대기 큐 확인, 생산 완료 처리 |
| [5] 모니터링 | 상태별 주문 건수, 시료별 재고 현황 |
| [6] 출고 처리 | CONFIRMED 주문을 RELEASE로 전환 |

### 주문 상태 흐름

```
RESERVED ─(승인)─▶ 재고 충분? ─ YES ─▶ CONFIRMED ─▶ RELEASE
                          │
                         NO
                          └─▶ PRODUCING ─(생산 완료)─▶ CONFIRMED ─▶ RELEASE
         ─(거절)─▶ REJECTED
```

### 생산량 계산 공식

```
유효 가용 재고 = max(0, 현재 재고 - 생산 대기 중인 동일 시료 주문량)
부족분        = 주문 수량 - 유효 가용 재고
실 생산량     = ceil(부족분 / (수율 × 0.9))
총 생산시간   = 평균 생산시간(min/ea) × 실 생산량
```

---

## 빌드 환경

| 항목 | 값 |
|------|-----|
| IDE | Visual Studio 2022 |
| PlatformToolset | v145 |
| C++ 표준 | C++17 |
| 플랫폼 | Windows x64 |
| 외부 의존성 | nlohmann/json (단일 헤더) |

### 의존성 설치

```powershell
.\setup_deps.ps1
```

`include/nlohmann/json.hpp`를 자동으로 다운로드합니다.

### 빌드

```powershell
# Debug
msbuild SampleOrderSystem.sln /p:Configuration=Debug /p:Platform=x64

# Release
msbuild SampleOrderSystem.sln /p:Configuration=Release /p:Platform=x64
```

빌드 결과물: `x64/Debug/SampleOrderSystem.exe`

---

## 실행

```powershell
.\x64\Debug\SampleOrderSystem.exe
```

데이터 파일(`data/orders.json`, `data/samples.json`, `data/production.json`)은 실행 시 자동 생성됩니다.

---

## 테스트

```powershell
# 테스트 빌드 후 실행
msbuild SampleOrderSystem.sln /p:Configuration=Debug /p:Platform=x64
.\x64\Debug\SampleOrderSystem.Tests.exe
```

doctest v2.4.11 기반 단위 테스트 65개가 포함되어 있습니다.

| 테스트 파일 | 대상 |
|------------|------|
| `test_release.cpp` | 출고 처리, 재고 차감 규칙 |
| `test_monitoring.cpp` | 주문 상태 집계, 재고 상태 판정 |
| `test_sample_repository.cpp` | 시료 CRUD |
| `test_order_repository.cpp` | 주문 CRUD, 상태 필터 |
| `test_production_repository.cpp` | 생산 큐 FIFO 동작 |
| `test_validator.cpp` | 입력값 검증 규칙 |
| `test_production_calc.cpp` | 생산량·시간 계산 공식 |

---

## 아키텍처

```
main.cpp
  └── MainController
        ├── SampleController  ──  SampleView
        ├── OrderController   ──  OrderView
        ├── ProductionController ── ProductionView
        ├── MonitorController ──  MonitorView
        └── ReleaseController ──  ReleaseView
              └── DataStore (Facade)
                    ├── SampleRepository    → data/samples.json
                    ├── OrderRepository     → data/orders.json
                    └── ProductionRepository → data/production.json
```

### 디렉토리 구조

```
SampleOrderSystem/src/
├── main.cpp
├── MainController.h / .cpp
├── core/            # 도메인 모델 + 공통 유틸
│   ├── Models.h         (SampleData, OrderData, ProductionJob)
│   ├── Validator.h      (입력값 검증 순수 함수)
│   ├── ProductionCalc.h (생산량 계산 순수 함수)
│   ├── DateTimeUtils.h  (날짜·시간 헬퍼)
│   ├── Color.h          (Windows 콘솔 컬러)
│   ├── UI.h             (공통 구분선 상수)
│   └── Utils.h
├── repository/      # 데이터 접근 레이어
│   ├── DataStore.h / .cpp         (Facade)
│   ├── SampleRepository.h / .cpp
│   ├── OrderRepository.h / .cpp
│   └── ProductionRepository.h / .cpp
├── sample/          ─ SampleController, SampleView
├── order/           ─ OrderController, OrderView
├── production/      ─ ProductionController, ProductionView
├── monitor/         ─ MonitorController, MonitorView
└── release/         ─ ReleaseController, ReleaseView
```

---

## 데이터 영속화

CRUD 작업마다 즉시 JSON 파일에 저장합니다.

```
data/
├── orders.json      # 주문 목록
├── samples.json     # 시료 목록
└── production.json  # 생산 큐
```

---

## 개발 이력

| Phase | 내용 |
|-------|------|
| Phase 1 | 프로젝트 기반 구축, 시료 관리 |
| Phase 2 | 주문 접수, 승인/거절 |
| Phase 3 | 생산라인 (FIFO 큐, 생산량 계산) |
| Phase 4 | 출고 처리, 모니터링, TDD 도입 (doctest) |
| Phase 5 | 입력 검증(Validator), 계산 공식 분리(ProductionCalc), 리팩토링 완성 |
| R1–R5 | 날짜 헬퍼 통합 → 상수 중복 제거 → Repository 패턴 → 도메인 디렉토리 재편 |
