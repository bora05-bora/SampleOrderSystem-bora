#include "release/ReleaseController.h"
#include "core/Color.h"
#include "core/UI.h"
#include <iostream>

ReleaseController::ReleaseController(DataStore& dataStore)
    : dataStore_(dataStore) {}

void ReleaseController::Run() {
    std::cout << "\n";
    Color::set(Color::CYAN);
    std::cout << UI::SEP_THICK << "\n";
    std::cout << " [6] 출고 처리\n";
    std::cout << UI::SEP_THICK << "\n";
    Color::reset();

    auto confirmed = dataStore_.GetConfirmedOrders();
    view_.ShowConfirmedList(confirmed);

    if (confirmed.empty()) return;

    int sel = view_.InputSelection((int)confirmed.size());
    if (sel == 0) return;

    const OrderData& target = confirmed[sel - 1];
    bool ok = dataStore_.ReleaseOrder(target.id);

    if (ok) {
        auto released = dataStore_.FindOrderById(target.id);
        if (released) view_.ShowReleaseResult(*released);
    } else {
        view_.ShowError("출고 처리에 실패했습니다.");
    }
}
