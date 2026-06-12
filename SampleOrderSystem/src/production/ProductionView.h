#pragma once
#include <vector>
#include <string>
#include "core/Models.h"

class ProductionView {
public:
    // 생산라인 RUNNING 상태 표시
    void ShowRunning(const std::vector<ProductionJob>& queue,
                     double progressPct,
                     const std::string& completionAt,
                     const std::vector<std::string>& waitingCompletionTimes) const;

    // 생산라인 EMPTY 상태 표시
    void ShowEmpty() const;

    // 자동 완료 처리 결과 표시
    void ShowAutoComplete(const ProductionJob& job,
                          int oldStock, int newStock,
                          bool hasNext, const std::string& nextName) const;

    void ShowError(const std::string& msg) const;

    // [0] 뒤로
    void InputBack() const;
};
