#include "MainController.h"
#include "Color.h"
#include "Utils.h"
#include <iostream>
#include <iomanip>
#include <ctime>

static std::string currentDateTime() {
    std::time_t now = std::time(nullptr);
    struct tm   ts  = {};
    localtime_s(&ts, &now);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &ts);
    return buf;
}

MainController::MainController(DataStore& dataStore)
    : dataStore_(dataStore), sampleCtrl_(dataStore) {}

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
        case 1: sampleCtrl_.Run(); break;
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
    std::cout << "============================================================\n";
    std::cout << "       반도체 시료 생산주문관리 시스템   S-Semi\n";
    std::cout << "============================================================\n";
    Color::reset();
    std::cout << " 시스템 현황   " << currentDateTime() << "\n";
    std::cout << "------------------------------------------------------------\n";
}

void MainController::showSummary() const {
    const auto& samples   = dataStore_.GetSamples();
    int         totalStock = dataStore_.GetTotalStock();

    std::cout << " 등록 시료  ";
    Color::set(Color::CYAN);
    std::cout << std::setw(4) << samples.size() << " 종";
    Color::reset();
    std::cout << "      총 재고  ";
    Color::set(Color::CYAN);
    std::cout << std::setw(7) << totalStock << " ea\n";
    Color::reset();
    std::cout << "------------------------------------------------------------\n";
}

void MainController::showMenu() const {
    Color::set(Color::CYAN);
    std::cout << " [1] 시료 관리\n";
    Color::reset();
    Color::set(Color::GRAY);
    std::cout << " [2] 시료 주문          (준비 중)\n";
    std::cout << " [3] 주문 승인/거절     (준비 중)\n";
    std::cout << " [4] 생산라인 조회      (준비 중)\n";
    std::cout << " [5] 모니터링           (준비 중)\n";
    std::cout << " [6] 출고 처리          (준비 중)\n";
    Color::reset();
    std::cout << " [0] 종료\n";
    std::cout << "------------------------------------------------------------\n";
}
