#pragma once
#include <algorithm>
#include <cmath>

namespace ProductionCalc {

    inline int calcEffectiveStock(int stock, int queuedQty) {
        return std::max(0, stock - queuedQty);
    }

    inline int calcShortage(int quantity, int effectiveStock) {
        return std::max(0, quantity - effectiveStock);
    }

    inline int calcActualQty(int shortage, double yield) {
        if (shortage <= 0) return 0;
        return (int)std::ceil((double)shortage / (yield * 0.9));
    }

    inline double calcTotalTime(double productionTime, int actualQty) {
        return productionTime * actualQty;
    }

}
