#pragma once
#include <vector>
#include <string>
#include <optional>
#include "Models.h"
#include "SampleRepository.h"
#include "OrderRepository.h"
#include "ProductionRepository.h"

class DataStore {
public:
    explicit DataStore(const std::string& dataDir);
    void Load();

    // Samples
    const std::vector<SampleData>& GetSamples() const;
    std::optional<SampleData>      FindSampleById(const std::string& id) const;
    std::vector<SampleData>        FindSamplesByName(const std::string& keyword) const;
    bool                           ExistsSampleId(const std::string& id) const;
    void                           AddSample(const SampleData& sample);
    void                           SaveSamples();
    std::string                    GenerateSampleId();
    bool                           UpdateSampleStock(const std::string& sampleId, int delta);

    // Orders
    const std::vector<OrderData>& GetOrders() const;
    std::vector<OrderData>        GetReservedOrders() const;
    std::vector<OrderData>        GetConfirmedOrders() const;
    std::optional<OrderData>      FindOrderById(int id) const;
    void                          AddOrder(const OrderData& order);
    void                          UpdateOrder(const OrderData& order);
    bool                          ReleaseOrder(int orderId);
    void                          SaveOrders();
    int                           NextOrderId();

    // Monitoring
    OrderStatusSummary            GetOrderStatusSummary() const;
    StockStatus                   GetStockStatus(const std::string& sampleId) const;

    // Production queue
    const std::vector<ProductionJob>& GetProductionQueue() const;
    void                              AddProductionJob(const ProductionJob& job);
    bool                              PopProductionJob(ProductionJob& out);
    void                              SetFrontStartedAt(const std::string& dt);
    void                              SaveProduction();
    int                               GetQueuedQuantityForSample(const std::string& sampleId) const;

    int GetTotalStock() const;

private:
    SampleRepository     sampleRepo_;
    OrderRepository      orderRepo_;
    ProductionRepository productionRepo_;
};
