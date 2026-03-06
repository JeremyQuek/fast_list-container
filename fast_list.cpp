#include <stdio.h>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <string.h>
#include <list>

#include "fast_list.h"

OrderBook::OrderBook() {
    book.count = 0;
    book.first_order = NULL_ID;
    book.last_order = NULL_ID;
    book.max_order_count = 10000000;

    buffer_start = aligned_alloc(64, sizeof(Order) *book.max_order_count);
    memset(buffer_start, 0, sizeof(Order) *book.max_order_count);
    buffer_end = (char*)buffer_start + (sizeof(Order) *book.max_order_count);
    high_water_mark = buffer_start;

    orderMap = (bool*)aligned_alloc(64,book.max_order_count * sizeof(bool));
    memset(orderMap, 0,book.max_order_count * sizeof(bool));
}

OrderBook::OrderBook(size_t max_order_size) {
    book.count = 0;
    book.first_order = NULL_ID;
    book.last_order = NULL_ID;
    book.max_order_count = max_order_size;

    buffer_start = aligned_alloc(64, sizeof(Order) *book.max_order_count);
    memset(buffer_start, 0, sizeof(Order) *book.max_order_count);
    buffer_end = (char*)buffer_start + (sizeof(Order) *book.max_order_count);
    high_water_mark = buffer_start;

    orderMap = (bool*)aligned_alloc(64,book.max_order_count * sizeof(bool));
    memset(orderMap, 0,book.max_order_count * sizeof(bool));
}


OrderBook::~OrderBook() {
    free(buffer_start);
    free(orderMap);
}

OrderBook::Order* OrderBook::resolveAddress(int id) {
    if (id == NULL_ID) return nullptr;
    return ((Order*)buffer_start) + id;
}

int OrderBook::getOrderId(OrderBook::Order* order) {
    return (int)(order - (Order*)buffer_start);
}

void OrderBook::invalidateOrder(OrderBook::Order* order) {
    order->next_order = NULL_ID;
    order->prev_order = NULL_ID;
}

bool OrderBook::addOrder(int value) {
    int new_id;
    if (!freeList.empty()) {
        new_id = freeList.back();
        freeList.pop_back();
    } else {
        if (high_water_mark >= buffer_end) return false;
        new_id = getOrderId((Order*)high_water_mark);
        high_water_mark = (char*)high_water_mark + sizeof(Order);
    }

    OrderBook::Order* new_order = resolveAddress(new_id);
    new_order->value = value;
    new_order->next_order = NULL_ID;
    new_order->prev_order = NULL_ID;

    if (book.count == 0) {
        book.first_order = new_id;
        book.last_order = new_id;
    } else {
        Order* prev_order = resolveAddress(book.last_order);
        prev_order->next_order = new_id;
        new_order->prev_order = book.last_order;
        book.last_order = new_id;
    }

    orderMap[new_id] = true;
    book.count++;
    return true;
}

bool OrderBook::consumeOrder() {
    if (book.first_order == NULL_ID) return false;
    int first_order = book.first_order;
    OrderBook::Order* first = resolveAddress(first_order);

    if (book.count == 1) {
        book.first_order = NULL_ID;
        book.last_order = NULL_ID;
    } else {
        book.first_order = first->next_order;
        resolveAddress(book.first_order)->prev_order = NULL_ID;
    }

    orderMap[first_order] = false;
    invalidateOrder(first);
    freeList.push_back(first_order);
    book.count--;
    return true;
}

bool OrderBook::removeOrder(int order_id) {
    if (order_id < 0 || order_id >=book.max_order_count || !orderMap[order_id]) return false;

    OrderBook::Order* target = resolveAddress(order_id);
    int next_order = target->next_order;
    int prev_order = target->prev_order;

    if (book.count == 1) {
        book.first_order = NULL_ID;
        book.last_order = NULL_ID;
    } else if (order_id == book.first_order) {
        book.first_order = next_order;
        resolveAddress(next_order)->prev_order = NULL_ID;
    } else if (order_id == book.last_order) {
        book.last_order = prev_order;
        resolveAddress(prev_order)->next_order = NULL_ID;
    } else {
        resolveAddress(prev_order)->next_order = next_order;
        resolveAddress(next_order)->prev_order = prev_order;
    }

    orderMap[order_id] = false;
    invalidateOrder(target);
    freeList.push_back(order_id);
    book.count--;
    return true;
}