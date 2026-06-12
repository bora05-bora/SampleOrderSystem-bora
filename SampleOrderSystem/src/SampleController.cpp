#include "SampleController.h"
#include "Color.h"
#include "Utils.h"
#include <iostream>

SampleController::SampleController(DataStore& dataStore)
    : dataStore_(dataStore) {}

void SampleController::Run() {
    while (true) {
        std::cout << "\n";
        Color::set(Color::CYAN);
        std::cout << "============================================================\n";
        std::cout << " [1] 시료 관리\n";
        std::cout << "============================================================\n";
        Color::reset();

        view_.ShowMenu();

        int choice;
        if (!(std::cin >> choice)) {
            clearInputBuffer();
            view_.ShowError("숫자를 입력해 주세요.");
            continue;
        }
        clearInputBuffer();

        switch (choice) {
        case 1: registerSample(); break;
        case 2: listSamples();    break;
        case 3: searchSamples();  break;
        case 0: return;
        default:
            view_.ShowError("유효한 메뉴 번호를 입력해 주세요. (0~3)");
        }
    }
}

void SampleController::registerSample() {
    std::cout << "\n";
    Color::set(Color::CYAN);
    std::cout << " ── 시료 등록 ──\n";
    Color::reset();

    SampleData sample;
    sample.id             = dataStore_.GenerateSampleId();
    sample.name           = view_.InputName();
    sample.productionTime = view_.InputProductionTime();
    sample.yield          = view_.InputYield();
    sample.stock          = 0;

    dataStore_.AddSample(sample);
    view_.ShowRegistered(sample);
}

void SampleController::listSamples() {
    view_.ShowSampleList(dataStore_.GetSamples());
}

void SampleController::searchSamples() {
    std::cout << "\n";
    Color::set(Color::CYAN);
    std::cout << " ── 시료 검색 ──\n";
    Color::reset();

    std::string keyword = view_.InputSearchKeyword();
    auto results = dataStore_.FindSamplesByName(keyword);

    if (results.empty()) {
        view_.ShowError("\"" + keyword + "\" 에 해당하는 시료가 없습니다.");
        return;
    }
    view_.ShowSearchResult(results, keyword);
}
