#include "OrderView.h"
#include "Color.h"
#include "Utils.h"
#include <iostream>
#include <iomanip>

static const std::string SEP = "------------------------------------------------------------";

// ── 예약 목록 ─────────────────────────────────────────────
void OrderView::ShowReservedList(const std::vector<OrderData>& orders) const {
    std::cout << "\n";
    std::cout << " 승인 대기 중인 예약 목록  (RESERVED)\n";
    std::cout << SEP << "\n";

    if (orders.empty()) {
        Color::set(Color::YELLOW);
        std::cout << " 처리 대기 중인 주문이 없습니다.\n";
        Color::reset();
        std::cout << SEP << "\n";
        return;
    }

    Color::set(Color::CYAN);
    std::cout << " " << std::left
              << std::setw(5)  << "번호"
              << std::setw(20) << "주문번호"
              << std::setw(14) << "고객"
              << std::setw(14) << "시료"
              << "수량\n";
    Color::reset();
    std::cout << SEP << "\n";

    int idx = 1;
    for (const auto& o : orders) {
        std::string num = "[" + std::to_string(idx++) + "]";
        std::cout << " " << std::left
                  << std::setw(5)  << num
                  << std::setw(20) << o.orderNo
                  << std::setw(14) << o.customer
                  << std::setw(14) << o.sampleName;
        std::cout << std::right << std::setw(4) << o.quantity << " ea\n";
    }
    std::cout << SEP << "\n";
}

// ── 주문 접수 확인 화면 ───────────────────────────────────
void OrderView::ShowOrderConfirm(const std::string& sampleId,
                                  const std::string& sampleName,
                                  const std::string& customer,
                                  int quantity) const {
    std::cout << "\n";
    Color::set(Color::CYAN);
    std::cout << " ── 입력 내용 확인 ─────────────────────────────────────\n";
    Color::reset();
    std::cout << " 시료      " << sampleName << "  (" << sampleId << ")\n";
    std::cout << " 고객      " << customer << "\n";
    std::cout << " 수량      " << quantity << " ea\n";
    std::cout << SEP << "\n";
    std::cout << " [Y] 예약 접수   [N] 취소\n";
}

// ── 주문 접수 완료 ────────────────────────────────────────
void OrderView::ShowOrderPlaced(const OrderData& order) const {
    std::cout << "\n";
    Color::set(Color::GREEN);
    std::cout << " 예약 접수 완료.\n";
    Color::reset();
    std::cout << SEP << "\n";
    std::cout << " 주문번호   " << order.orderNo << "\n";
    std::cout << " 현재 상태  RESERVED\n";
    Color::set(Color::YELLOW);
    std::cout << " ※ 재고 확인은 [3] 승인 메뉴에서 진행하세요.\n";
    Color::reset();
    std::cout << SEP << "\n";
}

// ── 재고 확인 화면 ────────────────────────────────────────
void OrderView::ShowStockInfo(const std::string& sampleName,
                               int stock, int quantity) const {
    std::cout << "\n";
    Color::set(Color::CYAN);
    std::cout << " 재고 확인 중...\n";
    Color::reset();

    std::cout << " 시료       " << sampleName << "\n";
    std::cout << " 현재 재고  " << stock    << " ea\n";
    std::cout << " 주문 수량  " << quantity << " ea\n";

    if (stock >= quantity) {
        Color::set(Color::GREEN);
        std::cout << "\n 재고 충분. 승인하시겠습니까?  [Y] 승인   [N] 거절\n";
        Color::reset();
    } else {
        int shortage = quantity - stock;
        Color::set(Color::YELLOW);
        std::cout << " 부족분     " << shortage << " ea\n";
        Color::reset();
        Color::set(Color::RED);
        std::cout << "\n 재고 부족. 승인하시겠습니까?  [Y] 승인   [N] 거절\n";
        Color::reset();
    }
}

// ── 승인/거절 결과 ────────────────────────────────────────
void OrderView::ShowApprovalResult(const OrderData& order, OrderStatus before) const {
    std::cout << "\n";
    bool approved = (order.status != OrderStatus::Rejected);

    if (approved) {
        Color::set(Color::GREEN);
        std::cout << " 승인 완료.\n";
    } else {
        Color::set(Color::RED);
        std::cout << " 거절 완료.\n";
    }
    Color::reset();

    std::cout << SEP << "\n";
    std::cout << " 상태 변경  "
              << orderStatusToString(before)
              << " → "
              << orderStatusToString(order.status) << "\n";
    std::cout << " 주문번호   " << order.orderNo << "\n";
    std::cout << SEP << "\n";
}

// ── 오류 ──────────────────────────────────────────────────
void OrderView::ShowError(const std::string& msg) const {
    Color::set(Color::RED);
    std::cout << "\n [오류] " << msg << "\n";
    Color::reset();
}

// ── 입력 ──────────────────────────────────────────────────
std::string OrderView::InputSampleId() const {
    std::string id;
    while (true) {
        std::cout << " 시료 ID   > ";
        std::getline(std::cin, id);
        if (!id.empty()) return id;
        ShowError("시료 ID를 입력해 주세요.");
    }
}

std::string OrderView::InputCustomer() const {
    std::string name;
    while (true) {
        std::cout << " 고객명    > ";
        std::getline(std::cin, name);
        if (!name.empty()) return name;
        ShowError("고객명을 입력해 주세요.");
    }
}

int OrderView::InputQuantity() const {
    while (true) {
        std::cout << " 주문 수량 > ";
        int val;
        if (std::cin >> val) {
            clearInputBuffer();
            if (val >= 1) return val;
            ShowError("1 이상의 수량을 입력해 주세요.");
        } else {
            clearInputBuffer();
            ShowError("숫자를 입력해 주세요.");
        }
    }
}

char OrderView::InputYN() const {
    while (true) {
        std::cout << " 선택 > ";
        std::string line;
        std::getline(std::cin, line);
        if (!line.empty()) {
            char c = (char)std::toupper((unsigned char)line[0]);
            if (c == 'Y' || c == 'N') return c;
        }
        ShowError("Y 또는 N을 입력해 주세요.");
    }
}

int OrderView::InputSelection(int max) const {
    while (true) {
        std::cout << " 선택  (0: 뒤로) > ";
        int sel;
        if (std::cin >> sel) {
            clearInputBuffer();
            if (sel >= 0 && sel <= max) return sel;
        } else {
            clearInputBuffer();
        }
        ShowError("0~" + std::to_string(max) + " 사이의 번호를 입력해 주세요.");
    }
}
