#include <stdio.h>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <string.h>
#include <list>

using namespace std;

struct OrderBook {
    static const int max_order_count = 10000000;
    static const int NULL_ID = -1;

    void* high_water_mark;
    void* buffer_start;
    void* buffer_end;

    struct Order {
        int value;
        int next_id;
        int prev_id;
        int side;
    };

    struct OrderBook_t {
        size_t count;
        int first_id;
        int last_id;
    };

    vector<int> freeList;
    bool* orderMap;
    OrderBook_t book;

    OrderBook() {
        book.count = 0;
        book.first_id = NULL_ID;
        book.last_id = NULL_ID;

        buffer_start = aligned_alloc(64, sizeof(Order) * max_order_count);
        memset(buffer_start, 0, sizeof(Order) * max_order_count);
        buffer_end = (char*)buffer_start + (sizeof(Order) * max_order_count);
        high_water_mark = buffer_start;

        orderMap = (bool*)aligned_alloc(64, max_order_count * sizeof(bool));
        memset(orderMap, 0, max_order_count * sizeof(bool));
    }

    ~OrderBook() {
        free(buffer_start);
        delete[] orderMap;
    }

    Order* resolve(int id) {
        if (id == NULL_ID) return nullptr;
        return ((Order*)buffer_start) + id;
    }

    int getOrderId(Order* order) {
        return (int)(order - (Order*)buffer_start);
    }

    void invalidate(Order* order) {
        order->next_id = NULL_ID;
        order->prev_id = NULL_ID;
    }

    bool addOrder(int value) {
        int new_id;
        if (!freeList.empty()) {
            new_id = freeList.back();
            freeList.pop_back();
        } else {
            if (high_water_mark >= buffer_end) return false;
            new_id = getOrderId((Order*)high_water_mark);
            high_water_mark = (char*)high_water_mark + sizeof(Order);
        }

        Order* new_order = resolve(new_id);
        new_order->value = value;
        new_order->next_id = NULL_ID;
        new_order->prev_id = NULL_ID;

        if (book.count == 0) {
            book.first_id = new_id;
            book.last_id = new_id;
        } else {
            Order* prev_order = resolve(book.last_id);
            prev_order->next_id = new_id;
            new_order->prev_id = book.last_id;
            book.last_id = new_id;
        }

        orderMap[new_id] = true;
        book.count++;
        return true;
    }

    bool consumeOrder() {
        if (book.first_id == NULL_ID) return false;
        int first_id = book.first_id;
        Order* first = resolve(first_id);

        if (book.count == 1) {
            book.first_id = NULL_ID;
            book.last_id = NULL_ID;
        } else {
            book.first_id = first->next_id;
            resolve(book.first_id)->prev_id = NULL_ID;
        }

        orderMap[first_id] = false;
        invalidate(first);
        freeList.push_back(first_id);
        book.count--;
        return true;
    }

    bool removeOrder(int order_id) {
        if (order_id < 0 || order_id >= max_order_count || !orderMap[order_id]) return false;

        Order* target = resolve(order_id);
        int next_id = target->next_id;
        int prev_id = target->prev_id;

        if (book.count == 1) {
            book.first_id = NULL_ID;
            book.last_id = NULL_ID;
        } else if (order_id == book.first_id) {
            book.first_id = next_id;
            resolve(next_id)->prev_id = NULL_ID;
        } else if (order_id == book.last_id) {
            book.last_id = prev_id;
            resolve(prev_id)->next_id = NULL_ID;
        } else {
            resolve(prev_id)->next_id = next_id;
            resolve(next_id)->prev_id = prev_id;
        }

        orderMap[order_id] = false;
        invalidate(target);
        freeList.push_back(order_id);
        book.count--;
        return true;
    }
};

int main(void) {
    Orderbook list();
    return 0;
}
