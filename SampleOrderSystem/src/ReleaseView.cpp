#include "ReleaseView.h"
#include "Color.h"
#include "Utils.h"
#include "UI.h"
#include <iostream>
#include <iomanip>

void ReleaseView::ShowConfirmedList(const std::vector<OrderData>& orders) const {
    std::cout << "\n";
    std::cout << " 출고 대기 중인 주문 목록  (CONFIRMED)\n";
    std::cout << UI::SEP_THIN<< "\n";

    if (orders.empty()) {
        Color::set(Color::YELLOW);
        std::cout << " 출고 가능한 주문이 없습니다.\n";
        Color::reset();
        std::cout << UI::SEP_THIN<< "\n";
        return;
    }

    Color::set(Color::CYAN);
    std::cout << " " << std::left
              << std::setw(5)  << "번호"
              << std::setw(22) << "주문번호"
              << std::setw(14) << "고객"
              << std::setw(14) << "시료"
              << "수량\n";
    Color::reset();
    std::cout << UI::SEP_THIN<< "\n";

    int idx = 1;
    for (const auto& o : orders) {
        std::string num = "[" + std::to_string(idx++) + "]";
        std::cout << " " << std::left
                  << std::setw(5)  << num
                  << std::setw(22) << o.orderNo
                  << std::setw(14) << o.customer
                  << std::setw(14) << o.sampleName;
        std::cout << std::right << std::setw(4) << o.quantity << " ea\n";
    }
    std::cout << UI::SEP_THIN<< "\n";
}

void ReleaseView::ShowReleaseResult(const OrderData& order) const {
    std::cout << "\n";
    Color::set(Color::MAGENTA);
    std::cout << " 출고 완료.\n";
    Color::reset();
    std::cout << UI::SEP_THIN<< "\n";
    std::cout << " 주문번호   " << order.orderNo << "\n";
    std::cout << " 고객       " << order.customer << "\n";
    std::cout << " 시료       " << order.sampleName << "\n";
    std::cout << " 수량       " << order.quantity << " ea\n";
    std::cout << " 상태       ";
    Color::set(Color::MAGENTA);
    std::cout << "RELEASE\n";
    Color::reset();
    std::cout << UI::SEP_THIN<< "\n";
}

void ReleaseView::ShowError(const std::string& msg) const {
    Color::set(Color::RED);
    std::cout << "\n [오류] " << msg << "\n";
    Color::reset();
}

int ReleaseView::InputSelection(int max) const {
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
