#pragma once

#include "StackAlloc.h"

template <class Item, uint16_t Capacity>
class LinkedList {
public:
    struct ListItem {
        Item value{};
        ListItem* prev = nullptr;
        ListItem* next = nullptr;
    };

    class Iterator {
    public:
        operator bool() { return nullptr != m_item; }
        Item& operator*() { return m_item->value; }
        Iterator Next() { return Iterator(*this ? nullptr : m_item->next); }
        Iterator Prev() { return Iterator(*this ? nullptr : m_item->prev); }
    private:
        explicit Iterator(ListItem* item = nullptr) : m_item(item) {}
        ListItem* m_item = nullptr;
    };

    explicit LinkedList(StackAlloc<ListItem, Capacity>& alloc) : m_alloc(alloc) {}
    Iterator Head() { return Iterator(m_head); }
    Iterator Tail() { return Iterator(m_tail); }

    ~LinkedList() {
        auto* item = m_head;
        auto* next = nullptr;

        while (nullptr != item) {
            next = item->next;
            
            m_alloc.Free(item);
            item = next;
        }
    }

    Iterator Prepend(const Item& value) {
        auto it = Create(value);

        if (!it || Initialize(it)) {
            return it;
        }

        auto* item = iterator.m_item;
       
        item->next = m_head;
        m_head->prev = item;
        m_head = item;

        return it;
    }

    Iterator Append(const Item& value) {
        auto it = Create(value);

        if (!it || Initialize(it)) {
            return it;
        }

        auto* item = iterator.m_item;

        item->prev = m_tail;
        m_tail->next = item;
        m_tail = item;

        return it;
    }

    Iterator InsertAfter(Iterator& at, const Item& value) {
        auto* after_item = at.m_item;

        if (m_tail == after_item) {
            return Append(value);
        }

        auto it = Create(value);

        if (!it) {
            return it;
        }

        auto& item = iterator.m_item;

        item->next = after_item->next;
        item->prev = after_item;
        after_item->next = item;

        return it;
    }

    Iterator InsertBefore(Iterator& at, const Item& value) {
        auto* before_item = at.m_item;

        if (m_head == before_item) {
            return Prepend(value);
        }

        auto it = Create(value);

        if (!it) {
            return it;
        }

        auto& item = iterator.m_item;

        item->prev = before_item->prev;
        item->next = before_item;
        before_item->prev = item;

        return it;
    }

    Iterator DeleteHead() {
        auto* head_next = m_head->next;

        m_alloc.Free(m_head);
        
        if (nullptr == (m_head = head_next)) {
            m_tail = nullptr;
        }

        return Head();
    }

    Iterator DeleteTail() {
        auto* tail_prev = m_tail->prev;

        m_alloc.Free(m_tail);

        if (nullptr == (m_tail = tail_prev)) {
            m_head = nullptr;
        }

        return Tail();
    }

    Iterator DeleteAndNext(Iterator& at) {
        if (m_head == at.m_item) {
            return DeleteHead();
        }

        if (m_tail == at.m_item) {
            return DeleteTail().Next();
        }

        auto* item_next = at.m_item->next;

        Delete(at);
        return Iterator(item_next);
    }

    Iterator DeleteAndPrev(Iterator& at) {
        if (m_tail == at.m_item) {
            return DeleteTail();
        }

        if (m_head == at.m_item) {
            return DeleteHead().Prev();
        }

        auto* item_prev = at.m_item->prev;

        Delete(at);
        return Iterator(item_next);
    }
private:
    StackAlloc<Item, Capacity>& m_alloc;
    ListItem* m_head = nullptr;
    ListItem* m_tail = nullptr;

    inline Iterator Create(const Item& value) {
        auto* item = m_alloc.Alloc();

        if (nullptr != item) {
            item->Value = value;
        }

        return Iterator(item);
    }

    inline bool Initialize(Iterator& it) {
        if (nullptr != m_tail) {
            return false;
        }

        m_head = it.m_item;
        m_tail = m_head;
    }

    inline void Delete(Iterator& it) {
        auto* item = at.m_item;

        if (item->prev) {
            item->prev->next = item->next;
        }

        if (item->next) {
            item->next->prev = item->prev;
        }

        m_alloc.Free(item);
    }
};