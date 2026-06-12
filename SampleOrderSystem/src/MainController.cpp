#include "MainController.h"
#include "core/Color.h"
#include "core/UI.h"
#include "core/Utils.h"
#include "core/DateTimeUtils.h"
#include <iostream>
#include <iomanip>

MainController::MainController(DataStore& dataStore)
    : dataStore_(dataStore), sampleCtrl_(dataStore),
      orderCtrl_(dataStore), productionCtrl_(dataStore),
      monitorCtrl_(dataStore), releaseCtrl_(dataStore) {}

void MainController::Run() {
    while (true) {
        showHeader();
        showSummary();
        showMenu();

        int choice;
        std::cout << " 선택 > ";
        if (!(std::cin >> choice)) {
            clearInputBuffer();
            Color::set(Color::RED);
            std::cout << "\n [오류] 숫자를 입력해 주세요.\n";
            Color::reset();
            continue;
        }
        clearInputBuffer();

        switch (choice) {
        case 1: sampleCtrl_.Run();             break;
        case 2: orderCtrl_.PlaceOrder();       break;
        case 3: orderCtrl_.ProcessApprovals(); break;
        case 4: productionCtrl_.Run();         break;
        case 5: monitorCtrl_.Run();            break;
        case 6: releaseCtrl_.Run();            break;
        case 0:
            std::cout << "\n 시스템을 종료합니다.\n\n";
            return;
        default:
            Color::set(Color::RED);
            std::cout << "\n [오류] 유효한 메뉴 번호를 입력해 주세요.\n";
            Color::reset();
        }
    }
}

void MainController::showHeader() const {
    std::cout << "\n";
    Color::set(Color::CYAN);
    std::cout << UI::SEP_THICK << "\n";
    std::cout << "       반도체 시료 생산주문관리 시스템   S-Semi\n";
    std::cout << UI::SEP_THICK << "\n";
    Color::reset();
    std::cout << " 시스템 현황   " << DateTimeUtils::nowDateTime() << "\n";
    std::cout << UI::SEP_THIN << "\n";
}

void MainController::showSummary() const {
    const auto& samples    = dataStore_.GetSamples();
    int         totalStock = dataStore_.GetTotalStock();
    int         reserved   = dataStore_.GetOrderStatusSummary().reserved;

    std::cout << " 등록 시료  ";
    Color::set(Color::CYAN);
    std::cout << std::setw(4) << samples.size() << " 종";
    Color::reset();
    std::cout << "      총 재고  ";
    Color::set(Color::CYAN);
    std::cout << std::setw(7) << totalStock << " ea";
    Color::reset();
    std::cout << "      대기 주문  ";
    if (reserved > 0) Color::set(Color::YELLOW);
    else              Color::set(Color::CYAN);
    std::cout << std::setw(3) << reserved << " 건\n";
    Color::reset();
    std::cout << UI::SEP_THIN << "\n";
}

void MainController::showMenu() const {
    Color::set(Color::CYAN);
    std::cout << " [1] 시료 관리\n";
    std::cout << " [2] 시료 주문\n";
    std::cout << " [3] 주문 승인/거절\n";
    std::cout << " [4] 생산라인 조회\n";
    Color::reset();
    std::cout << " [5] 모니터링\n";
    std::cout << " [6] 출고 처리\n";
    std::cout << " [0] 종료\n";
    std::cout << UI::SEP_THIN << "\n";
}
