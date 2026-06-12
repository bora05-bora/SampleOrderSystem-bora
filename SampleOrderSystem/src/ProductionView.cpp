#include "ProductionView.h"
#include "Color.h"
#include "Utils.h"
#include "UI.h"
#include <iostream>
#include <iomanip>

static std::string makeProgressBar(double pct) {
    int filled = std::min(20, (int)(pct / 100.0 * 20.0 + 0.5));
    std::string bar = "[";
    for (int i = 0; i < 20; i++)
        bar += (i < filled) ? "\xe2\x96\x88" : "\xe2\x96\x91";  // UTF-8: █ / ░
    bar += "]";
    return bar;
}

void ProductionView::ShowRunning(const std::vector<ProductionJob>& queue,
                                  double progressPct,
                                  const std::string& completionAt,
                                  const std::vector<std::string>& waitingCompletionTimes) const {
    const ProductionJob& cur = queue.front();

    std::cout << "\n";
    Color::set(Color::CYAN);
    std::cout << " -- 현재 처리 중 --------------------------------------------------\n";
    Color::reset();

    std::cout << " 주문번호   " << cur.orderNo   << "\n";
    std::cout << " 시료       " << cur.sampleName << "\n";
    std::cout << " 주문량     " << std::setw(5) << cur.quantity << " ea"
              << "   재고 "     << std::setw(5) << cur.stock    << " ea"
              << "   부족 "     << std::setw(5) << cur.shortage << " ea\n";
    std::cout << " 실생산량   " << cur.actualQty  << " ea\n";
    std::cout << " 시작 시각  " << cur.startedAt  << "\n";

    std::string bar = makeProgressBar(progressPct);
    if (progressPct >= 100.0) Color::set(Color::GREEN);
    else                      Color::set(Color::YELLOW);
    std::cout << " 진행도     " << bar
              << "  " << std::fixed << std::setprecision(1) << progressPct << "%\n";
    Color::reset();
    std::cout << " 완료 예정  " << completionAt << "\n";

    // 대기 목록
    if (queue.size() > 1) {
        std::cout << "\n";
        Color::set(Color::CYAN);
        std::cout << " -- 대기 중인 주문 (FIFO 순) -----------------------------------\n";
        Color::reset();

        for (size_t i = 1; i < queue.size(); i++) {
            const auto& j = queue[i];
            std::cout << " [" << i << "] " << j.orderNo << "  " << j.sampleName << "\n";
            std::cout << "     주문량 " << j.quantity  << " ea  "
                      << "부족 "        << j.shortage  << " ea  "
                      << "실생산량 "    << j.actualQty << " ea  "
                      << "예상완료 "    << waitingCompletionTimes[i - 1] << "\n";
        }
    }

    std::cout << UI::SEP_WIDE << "\n";
    Color::set(Color::YELLOW);
    std::cout << " * 진행도는 생산 시작 시각 기준 실시간 경과 시간으로 계산됩니다.\n";
    std::cout << " * FIFO 방식으로 처리됩니다.\n";
    Color::reset();
}

void ProductionView::ShowEmpty() const {
    std::cout << "\n";
    Color::set(Color::YELLOW);
    std::cout << " 현재 생산 중인 작업이 없습니다.\n";
    Color::reset();
    std::cout << UI::SEP_THIN<< "\n";
}

void ProductionView::ShowAutoComplete(const ProductionJob& job,
                                       int oldStock, int newStock,
                                       bool hasNext, const std::string& nextName) const {
    std::cout << "\n";
    Color::set(Color::GREEN);
    std::cout << " -- 생산 완료 감지 ------------------------------------------------\n";
    Color::reset();

    std::cout << " 주문번호   " << job.orderNo   << "\n";
    std::cout << " 시료       " << job.sampleName << "\n";
    std::cout << " 생산량     " << job.actualQty  << " ea\n";
    std::cout << "\n";
    std::cout << " 처리 결과\n";
    std::cout << " 재고 변화  " << oldStock << " ea -> " << newStock
              << " ea  (+" << job.actualQty << ")\n";
    std::cout << " 주문 상태  ";
    Color::set(Color::YELLOW);
    std::cout << "PRODUCING";
    Color::reset();
    std::cout << " -> ";
    Color::set(Color::GREEN);
    std::cout << "CONFIRMED";
    Color::reset();
    std::cout << "\n" << UI::SEP_THIN<< "\n";

    if (hasNext) {
        Color::set(Color::CYAN);
        std::cout << " 다음 작업  " << nextName << "  생산 시작\n";
        Color::reset();
    } else {
        std::cout << " 생산 큐가 비었습니다. 라인 상태: EMPTY\n";
    }
    std::cout << "\n";
}

void ProductionView::ShowError(const std::string& msg) const {
    Color::set(Color::RED);
    std::cout << "\n [오류] " << msg << "\n";
    Color::reset();
}

void ProductionView::InputBack() const {
    std::cout << "\n";
    Color::set(Color::CYAN);
    std::cout << " [0] 뒤로\n";
    Color::reset();
    std::cout << " 선택 > ";
    int sel;
    if (std::cin >> sel) clearInputBuffer();
    else                 clearInputBuffer();
}
