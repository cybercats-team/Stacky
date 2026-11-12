#pragma once

#include "StdInt.h"

class StackAlloc_Fixtures;

template <class Item, uint16_t Capacity>
class StackAlloc {
    friend class StackAlloc_Fixtures;
public:
    StackAlloc() {
        for (uint16_t index = 0; index < Capacity; ++index) {
            m_next_block[index] = index;
        }
    }

    uint16_t Available() {
        return Capacity - m_allocated;
    }

    Item* Alloc() {
        if (Capacity == m_allocated) {
            return nullptr;
        }

        uint16_t block_ptr = m_next_block[m_alloc_ptr];
        Advance(m_alloc_ptr);
        m_allocated++;

        return &m_blocks[block_ptr];
    }

    void Free(Item* item_ptr) {
        // WARN: UB if free twice or more
        uint16_t index = item_ptr - &m_blocks[0];

        if (m_free_ptr != index) {
            uint16_t tmp = m_next_block[index];

            m_next_block[index] = m_next_block[m_free_ptr];
            m_next_block[m_free_ptr] = tmp;
        }

        Advance(m_free_ptr);
        m_allocated--;
    }
private:
    uint16_t m_allocated = 0;
    uint16_t m_alloc_ptr = 0;
    uint16_t m_free_ptr = 0;
    uint16_t m_next_block[Capacity]{0};
    Item m_blocks[Capacity]{};

    inline static void Advance(uint16_t& ptr) {
        if (Capacity > ++ptr) {
            return;
        }

        ptr = 0;
    }
};