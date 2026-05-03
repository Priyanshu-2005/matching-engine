// ============================================================================
// Test Suite: CancelLookup
// Tests: register, get, deregister, out-of-bounds, capacity
// ============================================================================
#include <gtest/gtest.h>
#include <memory_resource>
#include "data/CancelLookup.hpp"
#include "data/Order.hpp"
#include "memory/EngineMemory.hpp"

using namespace matching_engine::data;
using namespace matching_engine::memory;

class CancelLookupTest : public ::testing::Test {
protected:
    static constexpr size_t MAX_ORDERS = 1000;

    EngineMemory memory{1024 * 1024 * 5}; // 5 MB
    std::pmr::polymorphic_allocator<std::byte> alloc{memory.get_allocator()};
    CancelLookup cancel{MAX_ORDERS, alloc};

    Order make_order(uint64_t id) {
        Order o;
        o.id = id;
        o.price = 15000;
        o.quantity = 100;
        o.side = Side::Buy;
        o.next = nullptr;
        o.prev = nullptr;
        return o;
    }
};

TEST_F(CancelLookupTest, Capacity) {
    EXPECT_EQ(cancel.capacity(), MAX_ORDERS);
}

TEST_F(CancelLookupTest, RegisterAndGet) {
    Order o = make_order(42);
    cancel.register_order(&o);

    Order* found = cancel.get_order(42);
    EXPECT_EQ(found, &o);
    EXPECT_EQ(found->id, 42u);
}

TEST_F(CancelLookupTest, GetUnregisteredReturnsNull) {
    EXPECT_EQ(cancel.get_order(0), nullptr);
    EXPECT_EQ(cancel.get_order(999), nullptr);
}

TEST_F(CancelLookupTest, DeregisterNullsSlot) {
    Order o = make_order(10);
    cancel.register_order(&o);
    EXPECT_NE(cancel.get_order(10), nullptr);

    cancel.deregister_order(10);
    EXPECT_EQ(cancel.get_order(10), nullptr);
}

TEST_F(CancelLookupTest, OutOfBoundsGetReturnsNull) {
    EXPECT_EQ(cancel.get_order(MAX_ORDERS), nullptr);
    EXPECT_EQ(cancel.get_order(MAX_ORDERS + 1000), nullptr);
}

TEST_F(CancelLookupTest, OutOfBoundsRegisterDoesNotCrash) {
    Order o = make_order(MAX_ORDERS + 1);
    // Should be a no-op, not a crash
    cancel.register_order(&o);
    EXPECT_EQ(cancel.get_order(MAX_ORDERS + 1), nullptr);
}

TEST_F(CancelLookupTest, OutOfBoundsDeregisterDoesNotCrash) {
    // Should be a no-op, not a crash
    cancel.deregister_order(MAX_ORDERS + 1);
}

TEST_F(CancelLookupTest, RegisterMultipleOrders) {
    Order orders[10];
    for (int i = 0; i < 10; i++) {
        orders[i] = make_order(i);
        cancel.register_order(&orders[i]);
    }

    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(cancel.get_order(i), &orders[i]);
    }
}

TEST_F(CancelLookupTest, ReregisterOverwritesPrevious) {
    Order o1 = make_order(5);
    Order o2 = make_order(5);

    cancel.register_order(&o1);
    EXPECT_EQ(cancel.get_order(5), &o1);

    cancel.register_order(&o2);
    EXPECT_EQ(cancel.get_order(5), &o2); // Overwritten
}

TEST_F(CancelLookupTest, DeregisterIdempotent) {
    Order o = make_order(7);
    cancel.register_order(&o);
    cancel.deregister_order(7);
    cancel.deregister_order(7); // Double deregister should not crash
    EXPECT_EQ(cancel.get_order(7), nullptr);
}

TEST_F(CancelLookupTest, BoundaryOrderId) {
    // Order ID 0 — valid index
    Order o = make_order(0);
    cancel.register_order(&o);
    EXPECT_EQ(cancel.get_order(0), &o);

    // Order ID MAX_ORDERS - 1 — last valid index
    Order o_last = make_order(MAX_ORDERS - 1);
    cancel.register_order(&o_last);
    EXPECT_EQ(cancel.get_order(MAX_ORDERS - 1), &o_last);
}
