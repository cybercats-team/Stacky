#include "pch.h"
#include <cstring>
#include "Stacky/StackAlloc.h"

class StackAlloc_Fixtures : public ::testing::Test {
protected:
    static constexpr const uint16_t kMaxCapacity = 0xffff;

    template <class Item, uint16_t Capacity>
    uint16_t GetAllocated(StackAlloc<Item, Capacity>& alloc) {
        return alloc.m_allocated;
    }

    template <class Item, uint16_t Capacity>
    uint16_t GetAllocPtr(StackAlloc<Item, Capacity>& alloc) {
        return alloc.m_alloc_ptr;
    }

    template <class Item, uint16_t Capacity>
    uint16_t GetFreePtr(StackAlloc<Item, Capacity>& alloc) {
        return alloc.m_free_ptr;
    }

    template <class Item, uint16_t Capacity>
    uint16_t const* GetIndexPtr(StackAlloc<Item, Capacity>& alloc) {
        return alloc.m_next_block;
    }

    template <class Item, uint16_t Capacity>
    Item const* GetDataPtr(StackAlloc<Item, Capacity>& alloc) {
        return alloc.m_blocks;
    }
};

namespace StackAlloc_Tests {
    using SmallString = char[5];
    
    struct DefaultConstructible {
        uint16_t value = 0;
        void* ptr = nullptr;
    };

    // Valid scenarios
    TEST_F(StackAlloc_Fixtures, ValidScenarios_Ctor_ShouldPrefillIndices) {
        StackAlloc<uint16_t, 10> allocator{};

        for (uint16_t i = 0; i < 10; ++i) { 
            EXPECT_EQ(*(GetIndexPtr(allocator) + i), i);
        }        
    }

    TEST_F(StackAlloc_Fixtures, ValidScenarios_Alloc_ShouldReturnNonNullPtr) {
        StackAlloc<uint16_t, 10> allocator{};

        EXPECT_NE(allocator.Alloc(), nullptr);
    }

    TEST_F(StackAlloc_Fixtures, ValidScenarios_Alloc_ShouldDerefPtrReturned) {
        StackAlloc<uint16_t, 10> allocator{};

        *allocator.Alloc() = 42;

        EXPECT_EQ(*GetDataPtr(allocator), 42);
    }

    TEST_F(StackAlloc_Fixtures, ValidScenarios_Alloc_ShouldAllocateFromStart) {
        StackAlloc<uint16_t, 10> allocator{};

        EXPECT_EQ(GetDataPtr(allocator), allocator.Alloc());
    }

    TEST_F(StackAlloc_Fixtures, ValidScenarios_Alloc_ShouldDecreaseAvailable) {
        StackAlloc<uint16_t, 10> allocator{};

        uint16_t avail_before = allocator.Available();
        allocator.Alloc();

        EXPECT_EQ(avail_before - allocator.Available(), 1);
    }

    TEST_F(StackAlloc_Fixtures, ValidScenarios_Alloc_ShouldAdvanceAllocPtr) {
        StackAlloc<uint16_t, 10> allocator{};

        uint16_t ptr_before = GetAllocPtr(allocator);
        allocator.Alloc();

        EXPECT_EQ(GetAllocPtr(allocator) - ptr_before, 1);
    }

    TEST_F(StackAlloc_Fixtures, ValidScenarios_Alloc_ShouldAllocBlockFromIndex) {
        StackAlloc<uint16_t, 10> allocator{};
        for (uint16_t i = 0; i < 5; ++i) { allocator.Alloc(); }

        uint16_t expected_block = *(GetIndexPtr(allocator) + 5);
        uint16_t allocated_block = allocator.Alloc() - GetDataPtr(allocator);

        EXPECT_EQ(allocated_block, expected_block);
    }

    TEST_F(StackAlloc_Fixtures, ValidScenarios_Free_ShouldIncreaseAvailable) {
        StackAlloc<uint16_t, 10> allocator{};
        uint16_t* last_allocated = allocator.Alloc();

        uint16_t avail_before = allocator.Available();
        allocator.Free(last_allocated);

        EXPECT_EQ(allocator.Available() - avail_before, 1);
    }
    TEST_F(StackAlloc_Fixtures, ValidScenarios_Free_ShouldAdvanceFreePtr) {
        StackAlloc<uint16_t, 10> allocator{};
        uint16_t* last_allocated = nullptr;

        for (uint16_t i = 0; i < 5; ++i) { last_allocated = allocator.Alloc(); }

        uint16_t ptr_before = GetFreePtr(allocator);
        allocator.Free(last_allocated);

        EXPECT_EQ(GetFreePtr(allocator) - ptr_before, 1);
    }

    TEST_F(StackAlloc_Fixtures, ValidScenarios_Free_ShouldSwapBlocksAtIndex) {
        StackAlloc<uint16_t, 10> allocator{};
        uint16_t* last_allocated = nullptr;

        for (uint16_t i = 0; i < 5; ++i) { last_allocated = allocator.Alloc(); }

        uint16_t first_block = *GetIndexPtr(allocator);
        uint16_t fifth_block = *(GetIndexPtr(allocator) + 4);

        allocator.Free(last_allocated);

        EXPECT_EQ(*GetIndexPtr(allocator), fifth_block);
        EXPECT_EQ(*(GetIndexPtr(allocator) + 4), first_block);
    }    

    TEST_F(StackAlloc_Fixtures, ValidScenarios_ShouldHandleSmallStrings) {
        StackAlloc<SmallString, 10> allocator{};

        std::strcpy(*allocator.Alloc(), "ABCD");
        EXPECT_STREQ(*GetDataPtr(allocator), "ABCD");
    }

    TEST_F(StackAlloc_Fixtures, ValidScenarios_ShouldDefaultConstructibleStructs) {
        StackAlloc<DefaultConstructible, 10> allocator{};

        DefaultConstructible* struc = allocator.Alloc();
        struc->value = 123;
        struc->ptr = struc;

        EXPECT_EQ(GetDataPtr(allocator)->value, 123);
        EXPECT_EQ(GetDataPtr(allocator)->ptr, GetDataPtr(allocator));
    }

    // Error handling
    TEST_F(StackAlloc_Fixtures, ErrorHandling_Alloc_ShouldReturnNullPtrIfNoSpaceLeft) {
        StackAlloc<uint16_t, 10> allocator{};
        for (uint16_t i = 0; i < 10; ++i) { allocator.Alloc(); }

        EXPECT_EQ(allocator.Alloc(), nullptr);
    }

    // Edge cases
    TEST_F(StackAlloc_Fixtures, EdgeCases_Alloc_ShouldHandleMaxUint16Space) {
        StackAlloc<uint16_t, kMaxCapacity> allocator{};

        for (uint16_t i = 0; i < kMaxCapacity; ++i) { 
            EXPECT_NE(allocator.Alloc(), nullptr); 
        }

        EXPECT_EQ(allocator.Alloc(), nullptr);
    }

    TEST_F(StackAlloc_Fixtures, EdgeCases_Free_ShouldNotChangeIndexIfReleasedAtCurrentFreePtr) {
        StackAlloc<uint16_t, 10> allocator{};
        
        allocator.Alloc();
        uint16_t* second_allocated = allocator.Alloc();

        uint16_t* last_allocated = nullptr;
        for (uint16_t i = 0; i < 3; ++i) { last_allocated = allocator.Alloc(); }

        uint16_t expected_block = *(GetIndexPtr(allocator) + 1);
        allocator.Free(last_allocated);
        allocator.Free(second_allocated);

        EXPECT_EQ(*(GetIndexPtr(allocator) + 1), expected_block);
    }

    TEST_F(StackAlloc_Fixtures, EdgeCases_Free_ShouldWrapAroundAllocPtr) {
        StackAlloc<uint16_t, 10> allocator{};

        uint16_t* last_allocated = nullptr;
        for (uint16_t i = 0; i < 10; ++i) { last_allocated = allocator.Alloc(); }

        EXPECT_EQ(GetAllocPtr(allocator), 0);        
    }

    TEST_F(StackAlloc_Fixtures, EdgeCases_Free_ShouldAllocFromStartAfterWrappedAround) {
        StackAlloc<uint16_t, 10> allocator{};

        uint16_t* last_allocated = nullptr;
        for (uint16_t i = 0; i < 10; ++i) { last_allocated = allocator.Alloc(); }
        allocator.Free(last_allocated);

        uint16_t expected_block = *GetIndexPtr(allocator);
        last_allocated = allocator.Alloc();

        EXPECT_EQ(GetAllocPtr(allocator), 1);
        EXPECT_EQ(last_allocated - GetDataPtr(allocator), expected_block);
    }

    TEST_F(StackAlloc_Fixtures, EdgeCases_Free_ShouldWrapAroundFreePtr) {
        StackAlloc<uint16_t, 10> allocator{};

        for (uint16_t i = 0; i < 10; ++i) { allocator.Free(allocator.Alloc()); }

        EXPECT_EQ(GetAllocPtr(allocator), 0);
        EXPECT_EQ(GetFreePtr(allocator), 0);
    }
}
