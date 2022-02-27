#include "gwn/gwn.h"
#include <assert.h>

gwn::Logger::ptr g_logger = gwn_LOG_ROOT();

void test_assert() {
    gwn_LOG_INFO(g_logger) << gwn::BacktraceToString(10);
    //gwn_ASSERT(0==0);
    gwn_ASSERT2(0 == 1, "abcdef xx");
}

int main(int argc, char** argv) {
    test_assert();

    // int arr[] = {1,3,5,7,9,11};

    // gwn_LOG_INFO(g_logger) << gwn::BinarySearch(arr, sizeof(arr) / sizeof(arr[0]), 0);
    // gwn_LOG_INFO(g_logger) << gwn::BinarySearch(arr, sizeof(arr) / sizeof(arr[0]), 1);
    // gwn_LOG_INFO(g_logger) << gwn::BinarySearch(arr, sizeof(arr) / sizeof(arr[0]), 4);
    // gwn_LOG_INFO(g_logger) << gwn::BinarySearch(arr, sizeof(arr) / sizeof(arr[0]), 13);
    // gwn_ASSERT(gwn::BinarySearch(arr, sizeof(arr) / sizeof(arr[0]), 0) == -1);
    // gwn_ASSERT(gwn::BinarySearch(arr, sizeof(arr) / sizeof(arr[0]), 1) == 0);
    // gwn_ASSERT(gwn::BinarySearch(arr, sizeof(arr) / sizeof(arr[0]), 2) == -2);
    // gwn_ASSERT(gwn::BinarySearch(arr, sizeof(arr) / sizeof(arr[0]), 3) == 1);
    // gwn_ASSERT(gwn::BinarySearch(arr, sizeof(arr) / sizeof(arr[0]), 4) == -3);
    // gwn_ASSERT(gwn::BinarySearch(arr, sizeof(arr) / sizeof(arr[0]), 5) == 2);
    // gwn_ASSERT(gwn::BinarySearch(arr, sizeof(arr) / sizeof(arr[0]), 6) == -4);
    // gwn_ASSERT(gwn::BinarySearch(arr, sizeof(arr) / sizeof(arr[0]), 7) == 3);
    // gwn_ASSERT(gwn::BinarySearch(arr, sizeof(arr) / sizeof(arr[0]), 8) == -5);
    // gwn_ASSERT(gwn::BinarySearch(arr, sizeof(arr) / sizeof(arr[0]), 9) == 4);
    // gwn_ASSERT(gwn::BinarySearch(arr, sizeof(arr) / sizeof(arr[0]), 10) == -6);
    // gwn_ASSERT(gwn::BinarySearch(arr, sizeof(arr) / sizeof(arr[0]), 11) == 5);
    // gwn_ASSERT(gwn::BinarySearch(arr, sizeof(arr) / sizeof(arr[0]), 12) == -7);
    return 0;
}
