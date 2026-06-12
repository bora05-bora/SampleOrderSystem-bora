#include "repository/DataStore.h"

DataStore::DataStore(const std::string& dataDir)
    : sampleRepo_(dataDir), orderRepo_(dataDir), productionRepo_(dataDir) {}

void DataStore::Load() {
    sampleRepo_.Load();
    orderRepo_.Load();
    productionRepo_.Load();
}

// ── Samples ──────────────────────────────────────────────

const std::vector<SampleData>& DataStore::GetSamples() const                               { return sampleRepo_.GetAll(); }
std::optional<SampleData>      DataStore::FindSampleById(const std::string& id) const      { return sampleRepo_.FindById(id); }
std::vector<SampleData>        DataStore::FindSamplesByName(const std::string& kw) const   { return sampleRepo_.FindByName(kw); }
bool                           DataStore::ExistsSampleId(const std::string& id) const      { return sampleRepo_.Exists(id); }
void                           DataStore::AddSample(const SampleData& s)                   { sampleRepo_.Add(s); }
void                           DataStore::SaveSamples()                                    { sampleRepo_.Save(); }
std::string                    DataStore::GenerateSampleId()                               { return sampleRepo_.GenerateId(); }
bool                           DataStore::UpdateSampleStock(const std::string& id, int d)  { return sampleRepo_.UpdateStock(id, d); }
int                            DataStore::GetTotalStock() const                            { return sampleRepo_.GetTotalStock(); }

// ── Orders ───────────────────────────────────────────────

const std::vector<OrderData>& DataStore::GetOrders() const                                 { return orderRepo_.GetAll(); }
std::vector<OrderData>        DataStore::GetReservedOrders() const                         { return orderRepo_.GetByStatus(OrderStatus::Reserved); }
std::vector<OrderData>        DataStore::GetConfirmedOrders() const                        { return orderRepo_.GetByStatus(OrderStatus::Confirmed); }
std::optional<OrderData>      DataStore::FindOrderById(int id) const                       { return orderRepo_.FindById(id); }
void                          DataStore::AddOrder(const OrderData& o)                      { orderRepo_.Add(o); }
void                          DataStore::UpdateOrder(const OrderData& o)                   { orderRepo_.Update(o); }
bool DataStore::ReleaseOrder(int orderId) {
    auto order = orderRepo_.FindById(orderId);
    if (!order) return false;
    bool ok = orderRepo_.Release(orderId);
    if (ok && order->producingBased)
        sampleRepo_.UpdateStock(order->sampleId, -order->quantity);
    return ok;
}
void                          DataStore::SaveOrders()                                      { orderRepo_.Save(); }
int                           DataStore::NextOrderId()                                     { return orderRepo_.NextId(); }

// ── Monitoring ────────────────────────────────────────────

OrderStatusSummary DataStore::GetOrderStatusSummary() const { return orderRepo_.GetStatusSummary(); }

StockStatus DataStore::GetStockStatus(const std::string& sampleId) const {
    auto sample = sampleRepo_.FindById(sampleId);
    if (!sample) return StockStatus::Depleted;
    if (sample->stock == 0) return StockStatus::Depleted;

    int reservedTotal = 0;
    for (const auto& o : orderRepo_.GetAll())
        if (o.sampleId == sampleId && o.status == OrderStatus::Reserved)
            reservedTotal += o.quantity;

    return (sample->stock < reservedTotal) ? StockStatus::Short : StockStatus::Plenty;
}

// ── Production ────────────────────────────────────────────

const std::vector<ProductionJob>& DataStore::GetProductionQueue() const                    { return productionRepo_.GetQueue(); }
void                              DataStore::AddProductionJob(const ProductionJob& j)      { productionRepo_.Enqueue(j); }
bool                              DataStore::PopProductionJob(ProductionJob& out)          { return productionRepo_.Dequeue(out); }
void                              DataStore::SetFrontStartedAt(const std::string& dt)     { productionRepo_.SetFrontStartedAt(dt); }
void                              DataStore::SaveProduction()                              { productionRepo_.Save(); }
int                               DataStore::GetQueuedQuantityForSample(const std::string& id) const { return productionRepo_.GetQueuedQuantityForSample(id); }
