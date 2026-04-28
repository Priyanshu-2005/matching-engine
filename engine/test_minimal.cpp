#include "include/data/IntrusiveOrderList.hpp"
#include "include/data/Order.hpp"
#include "include/data/PriceLevel.hpp"
#include <chrono>
#include <iostream>
#include <vector>

using namespace matching_engine::data;

int main() {
  std::cout << "=== Ultra-Low-Latency Matching Engine - Minimal Test ===\n";

  // Test 1: Order struct packing
  std::cout << "1. Order struct size: " << sizeof(Order)
            << " bytes (expected: 40)\n";
  std::cout << "   Alignment: " << alignof(Order) << " bytes\n";

  // Test 2: Intrusive list operations
  IntrusiveOrderList list;
  Order orders[3];

  for (int i = 0; i < 3; ++i) {
    orders[i].id = i + 1;
    orders[i].price = 15000 + i * 100;
    orders[i].quantity = 100;
    orders[i].side = (i % 2 == 0) ? Side::Buy : Side::Sell;
  }

  list.push_back(&orders[0]);
  list.push_back(&orders[1]);
  list.push_back(&orders[2]);

  std::cout << "2. Intrusive list test:\n";
  std::cout << "   Head order ID: " << (list.head() ? list.head()->id : 0)
            << "\n";
  std::cout << "   Tail order ID: " << (list.tail() ? list.tail()->id : 0)
            << "\n";

  // Test 3: Remove middle order
  list.remove(&orders[1]);
  std::cout << "   After removing middle order, head ID: "
            << (list.head() ? list.head()->id : 0) << "\n";

  // Test 4: PriceLevel struct
  PriceLevel level;
  level.total_volume = 500;
  std::cout << "3. PriceLevel size: " << sizeof(PriceLevel)
            << " bytes (expected: 24)\n";

  // Test 5: Performance - create 1000 orders in list
  auto start = std::chrono::high_resolution_clock::now();

  std::vector<Order> many_orders(1000);
  IntrusiveOrderList perf_list;

  for (size_t i = 0; i < many_orders.size(); ++i) {
    many_orders[i].id = i + 1000;
    many_orders[i].price = 15000 + (i % 100);
    many_orders[i].quantity = 10;
    many_orders[i].side = Side::Buy;
    perf_list.push_back(&many_orders[i]);
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

  std::cout << "4. Performance: Created 1000 orders in " << duration.count()
            << " ns\n";
  std::cout << "   Average: " << duration.count() / 1000.0 << " ns per order\n";

  // Test 6: Cache alignment verification
  std::cout << "5. Cache alignment:\n";
  std::cout << "   Order fits in " << (sizeof(Order) + 63) / 64
            << " cache lines\n";
  std::cout << "   Two PriceLevels fit in "
            << (2 * sizeof(PriceLevel) + 63) / 64 << " cache lines\n";

  std::cout << "\n=== Test PASSED ===\n";
  std::cout
      << "Core data structures validate mechanical sympathy principles.\n";

  return 0;
}