#pragma once

#include <boost/mpl/apply.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/greater.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/multiplies.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/push_front.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/preprocessor/repetition/enum.hpp>

#define __LUT_SET_INDEX(z, n, sequence) \
    mpl::at_c<sequence, n>::type::value

#define __GENERATE_LUT_IMPL(function, tableType, tableName, tableSize) \
\
template <typename sequence, typename size> \
struct __compileTable_##function##_##tableSize: \
    mpl::if_<mpl::greater<size, mpl::int_<0> >, \
        __compileTable_##function##_##tableSize< \
            typename mpl::push_front<sequence, \
                typename mpl::apply< \
                    function<mpl::_>, \
                    size \
                >::type>::type, \
            typename size::prior \
        >, \
        sequence \
    >::type \
{}; \
\
typedef __compileTable_##function##_##tableSize< \
    mpl::vector<>, \
    mpl::int_<tableSize> \
>::type __compiledTable_##function##_##tableSize; \
\
static const tableType tableName[] = { \
            BOOST_PP_ENUM( \
                          tableSize, \
                          __LUT_SET_INDEX, \
                          __compiledTable_##function##_##tableSize \
                         ) \
}

#define GENERATE_LUT(function, tableType, tableName, tableSize) \
    __GENERATE_LUT_IMPL(function, tableType, tableName, tableSize)

