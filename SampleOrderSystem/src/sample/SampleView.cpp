#include "sample/SampleView.h"
#include "core/Color.h"
#include "core/Utils.h"
#include "core/Validator.h"
#include "core/UI.h"
#include <iostream>
#include <iomanip>

void SampleView::ShowMenu() const {
    std::cout << "\n";
    Color::set(Color::CYAN);
    std::cout << " [1] 시료 등록   [2] 시료 목록   [3] 시료 검색   [0] 뒤로\n";
    Color::reset();
    std::cout << UI::SEP_THIN << "\n";
    std::cout << " 선택 > ";
}

void SampleView::ShowSampleList(const std::vector<SampleData>& samples) const {
    std::cout << "\n";
    std::cout << " 등록 시료 목록  (총 " << samples.size() << "종)\n";
    std::cout << UI::SEP_THIN << "\n";

    if (samples.empty()) {
        std::cout << " 등록된 시료가 없습니다.\n";
        std::cout << UI::SEP_THIN << "\n";
        return;
    }

    Color::set(Color::CYAN);
    std::cout << " " << std::left
              << std::setw(9)  << "ID"
              << std::setw(28) << "시료명"
              << std::setw(10) << "생산시간"
              << std::setw(6)  << "수율"
              << "재고\n";
    Color::reset();
    std::cout << UI::SEP_THIN << "\n";

    for (const auto& s : samples) {
        std::cout << " " << std::left << std::setw(9) << s.id;
        std::cout << std::setw(28) << s.name;

        std::cout << std::right
                  << std::fixed << std::setprecision(1)
                  << std::setw(3) << s.productionTime << " min/ea";

        std::cout << std::fixed << std::setprecision(2)
                  << std::setw(6) << s.yield;

        if (s.stock == 0)      Color::set(Color::RED);
        else if (s.stock < 50) Color::set(Color::YELLOW);
        else                   Color::set(Color::GREEN);
        std::cout << std::right << std::setw(3) << s.stock << " ea";
        Color::reset();
        std::cout << "\n";
    }
    std::cout << UI::SEP_THIN << "\n";
}

void SampleView::ShowSearchResult(const std::vector<SampleData>& results,
                                  const std::string& keyword) const {
    std::cout << "\n";
    std::cout << " 검색 결과  \"" << keyword << "\"  (" << results.size() << "건)\n";
    ShowSampleList(results);
}

void SampleView::ShowRegistered(const SampleData& sample) const {
    std::cout << "\n";
    Color::set(Color::GREEN);
    std::cout << " 시료가 등록되었습니다.\n";
    Color::reset();
    std::cout << UI::SEP_THIN << "\n";
    std::cout << " ID      " << sample.id   << "\n";
    std::cout << " 이름    " << sample.name << "\n";
    std::cout << " 재고    0 ea (초기값)\n";
    std::cout << UI::SEP_THIN << "\n";
}

void SampleView::ShowError(const std::string& msg) const {
    Color::set(Color::RED);
    std::cout << "\n [오류] " << msg << "\n";
    Color::reset();
}

void SampleView::ShowSuccess(const std::string& msg) const {
    Color::set(Color::GREEN);
    std::cout << "\n [완료] " << msg << "\n";
    Color::reset();
}

std::string SampleView::InputSampleId() const {
    std::string id;
    while (true) {
        std::cout << " 시료 ID > ";
        std::getline(std::cin, id);
        if (!id.empty()) return id;
        ShowError("ID를 입력해 주세요.");
    }
}

std::string SampleView::InputName() const {
    std::string name;
    while (true) {
        std::cout << " 시료명  > ";
        std::getline(std::cin, name);
        if (Validator::isNonEmpty(name)) return name;
        ShowError("시료명을 입력해 주세요.");
    }
}

double SampleView::InputProductionTime() const {
    while (true) {
        std::cout << " 평균 생산시간 (min/ea) > ";
        double val;
        if (std::cin >> val) {
            clearInputBuffer();
            if (Validator::isPositiveDouble(val)) return val;
            ShowError("0보다 큰 값을 입력해 주세요.");
        } else {
            clearInputBuffer();
            ShowError("숫자를 입력해 주세요.");
        }
    }
}

double SampleView::InputYield() const {
    while (true) {
        std::cout << " 수율 (0.0 초과 ~ 1.0) > ";
        double val;
        if (std::cin >> val) {
            clearInputBuffer();
            if (Validator::isValidYield(val)) return val;
            ShowError("0.0 초과 1.0 이하의 값을 입력해 주세요.");
        } else {
            clearInputBuffer();
            ShowError("숫자를 입력해 주세요.");
        }
    }
}

std::string SampleView::InputSearchKeyword() const {
    std::string keyword;
    while (true) {
        std::cout << " 검색어 > ";
        std::getline(std::cin, keyword);
        if (!keyword.empty()) return keyword;
        ShowError("검색어를 입력해 주세요.");
    }
}
