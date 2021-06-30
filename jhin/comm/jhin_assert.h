#pragma once

#include <cassert>
#include <type_traits>
#include <string>

// type check missing for macro
// so here we introduce static_assert
// JHIN_ASSERT_BOOL(true);
#define JHIN_ASSERT_BOOL(cmp) \
        do { static_assert(std::is_same_v<decltype(cmp), bool>); assert(cmp); } while (false)

// JHIN_ASSERT_STR("error");
#define JHIN_ASSERT_STR(str) \
        do { static_assert(std::is_same_v<std::decay_t<decltype(str)>, const char *>); assert(!static_cast<bool>(str)); } while (false)

namespace jhin
{
namespace comm
{
    /*
    void JhinAssert(const char* AssertInfo)
    {
        assert(!static_cast<bool>(AssertInfo));
    }

    void JhinAssert(bool Assert)
    {
        assert(Assert);
    }
    */

}   /* namespace comm */
}   /* namespace jhin */

