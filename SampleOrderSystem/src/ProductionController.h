#pragma once
#include <string>
#include "DataStore.h"
#include "ProductionView.h"

class ProductionController {
public:
    explicit ProductionController(DataStore& dataStore);
    void Run();

private:
    DataStore&     dataStore_;
    ProductionView view_;

    // 완료된 작업을 자동 처리. 처리가 발생하면 true 반환.
    bool tryAutoComplete();

};
