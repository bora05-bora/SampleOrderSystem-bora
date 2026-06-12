#include "monitor/MonitorView.h"
#include "core/Color.h"
#include "core/Utils.h"
#include "core/UI.h"
#include <iostream>
#include <iomanip>

static void printStatusLabel(StockStatus s) {
    switch (s) {
    case StockStatus::Plenty:
        Color::set(Color::GREEN);
        std::cout << "여유";
        break;
    case StockStatus::Short:
        Color::set(Color::YELLOW);
        std::cout << "부족";
        break;
    case StockStatus::Depleted:
        Color::set(Color::RED);
        std::cout << "고갈";
        break;
    }
    Color::reset();
}

void MonitorView::ShowMonitor(const OrderStatusSummary& summary,
                               const std::vector<SampleData>& samples,
                               const DataStore& ds) const {
    // ── 주문 현황 ──────────────────────────────────────────
    std::cout << "\n";
    Color::set(Color::CYAN);
    std::cout << " ── 주문 현황 ──────────────────────────────────────────\n";
    Color::reset();

    auto printCount = [](const char* label, int count, WORD color) {
        std::cout << "  " << std::left << std::setw(12) << label;
        Color::set(color);
        std::cout << std::right << std::setw(4) << count << " 건\n";
        Color::reset();
    };

    printCount("RESERVED",  summary.reserved,  Color::BLUE);
    printCount("PRODUCING", summary.producing, Color::YELLOW);
    printCount("CONFIRMED", summary.confirmed, Color::GREEN);
    printCount("RELEASE",   summary.release,   Color::MAGENTA);

    int total = summary.reserved + summary.producing + summary.confirmed + summary.release;
    std::cout << UI::SEP_THIN<< "\n";
    std::cout << "  " << std::left << std::setw(12) << "합계";
    Color::set(Color::CYAN);
    std::cout << std::right << std::setw(4) << total << " 건\n";
    Color::reset();

    // ── 재고 현황 ──────────────────────────────────────────
    std::cout << "\n";
    Color::set(Color::CYAN);
    std::cout << " ── 재고 현황 ──────────────────────────────────────────\n";
    Color::reset();

    if (samples.empty()) {
        Color::set(Color::YELLOW);
        std::cout << "  등록된 시료가 없습니다.\n";
        Color::reset();
    } else {
        Color::set(Color::CYAN);
        std::cout << "  " << std::left
                  << std::setw(6)  << "ID"
                  << std::setw(22) << "시료명"
                  << std::setw(10) << "재고"
                  << "상태\n";
        Color::reset();
        std::cout << UI::SEP_THIN<< "\n";

        for (const auto& s : samples) {
            StockStatus status = ds.GetStockStatus(s.id);
            std::cout << "  " << std::left
                      << std::setw(6)  << s.id
                      << std::setw(22) << s.name;
            Color::set(Color::CYAN);
            std::cout << std::right << std::setw(6) << s.stock << " ea  ";
            Color::reset();
            printStatusLabel(status);
            std::cout << "\n";
        }
    }
    std::cout << UI::SEP_THIN<< "\n";
}

void MonitorView::InputBack() const {
    std::cout << "\n [0] 뒤로 > ";
    std::string line;
    std::getline(std::cin, line);
}
