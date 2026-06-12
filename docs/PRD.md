# PRD — 반도체 시료 생산주문관리 시스템

## 배경 및 목적

S-Semi는 다양한 종류의 반도체 시료(Sample)를 생산하여 연구소, 팹리스(Fabless) 업체, 대학 연구실 등에 납품하는 회사입니다.

주문량이 급증하면서 엑셀·메모장 기반의 수작업 관리로는 아래 문제들이 발생했습니다.

- 주문 처리 여부를 확인하기 어려움
- 공정 완성 시점을 예측할 수 없음
- 재고가 충분한데도 불필요한 추가 공정이 가동됨

이를 해결하기 위해 **콘솔 기반 반도체 시료 생산주문관리 시스템**을 개발합니다.

---

## 사용자 역할

| 역할 | 설명 | 주요 행동 |
|------|------|---------|
| 고객 | 시료 요청자 | 필요한 시료를 이메일로 요청 |
| 주문 담당자 | 주문서 관리 | 고객 요청에 맞게 주문서 작성 및 시스템 입력 |
| 생산 담당자 | 시료 생산·승인 | 개발 시료 등록, 주문 수신 후 승인 또는 거절 |

---

## 주문 상태 흐름

모든 주문은 아래 5가지 상태 중 하나를 가집니다.

```
RESERVED → (승인) → 재고 확인
                      ├─ 재고 충분 → CONFIRMED → RELEASE
                      └─ 재고 부족 → PRODUCING → CONFIRMED → RELEASE
           → (거절) → REJECTED
```

| 상태 | 의미 |
|------|------|
| RESERVED | 주문 접수 (최초 등록 상태) |
| REJECTED | 주문 거절 (정상 흐름 외 상태, 모니터링 제외) |
| PRODUCING | 주문 승인 완료 + 재고 부족으로 생산 중 |
| CONFIRMED | 주문 승인 완료 + 출고 대기 중 |
| RELEASE | 출고 완료 |

---

## 메인 메뉴 구성

시스템 진입 시 전체 시료 요약 정보(등록 시료 수, 총 재고, 전체 주문 수, 생산라인 대기 수)를 함께 표시합니다.

| 메뉴 | 설명 | 상세 문서 |
|------|------|---------|
| 시료 관리 | 시료 등록, 목록 조회, 검색 | [feature-sample-management.md](features/feature-sample-management.md) |
| 시료 주문 | 고객 주문 접수 (RESERVED 생성) | [feature-order.md](features/feature-order.md) |
| 주문 승인/거절 | RESERVED 주문에 대한 승인 또는 거절 처리 | [feature-order-approval.md](features/feature-order-approval.md) |
| 모니터링 | 상태별 주문 수 및 시료별 재고 현황 확인 | [feature-monitoring.md](features/feature-monitoring.md) |
| 생산라인 조회 | 현재 생산 중 시료 및 대기 중인 생산 큐 확인 | [feature-production-line.md](features/feature-production-line.md) |
| 출고 처리 | CONFIRMED 주문에 대해 출고 실행 | [feature-release.md](features/feature-release.md) |

---

## 핵심 요구사항 요약

### 시료 (Sample)

- 시스템에 등록된 시료만 주문 가능
- 시료는 고유 ID, 이름, 평균 생산시간, 수율 속성을 가짐
- 수율 = 정상 시료 수 / 총 생산 시료 수 (예: 0.92)

### 주문

- 주문 접수 시 입력값: 시료 ID, 고객명, 주문 수량
- 주문번호는 시스템이 자동 부여 (예: ORD-YYYYMMDD-NNNN)

### 승인 처리 (자동 분기)

- 재고 충분 → 즉시 CONFIRMED 전환
- 재고 부족 → 생산라인 자동 등록 + PRODUCING 전환

### 생산라인

- 생산라인은 단일 라인 (시료를 하나씩 순차 생산)
- 스케줄링: FIFO 방식
- 실 생산량 = `ceil(부족분 / (수율 × 0.9))`
- 총 생산 시간 = 평균 생산시간 × 실 생산량
- 생산 완료 시 주문 상태 PRODUCING → CONFIRMED 자동 전환

### 모니터링

- REJECTED 주문은 모니터링에서 제외
- 재고 상태: 여유(주문대비 충분) / 부족(주문대비 부족) / 고갈(수량 0)
