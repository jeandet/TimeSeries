

#include "gtest/gtest.h"
#include <TimeSeries.h>
#include <math.h>
#include <numeric>

// http://www.cplusplus.com/reference/iterator/
namespace
{
  class ATSIteratorValue : public ::testing::Test
  {
  protected:
    ATSIteratorValue() {}

    virtual ~ATSIteratorValue() {}
  };

  TEST(ATSIteratorValue, CanBeConstructed)
  {
    auto s = TimeSeries::ScalarTs({0., 1., 2.}, {33., 22., 11.});
    auto v = *std::begin(s);
    EXPECT_EQ(v, 33.);
  }

  TEST(ATSIteratorValue, ValueDoesntModifyTS)
  {
    auto s = TimeSeries::ScalarTs({0., 1., 2.}, {33., 22., 11.});
    auto v = *std::begin(s);
    v      = 44.;
    EXPECT_EQ(v, 44.);
    EXPECT_EQ(*std::begin(s), 33.);
  }

  TEST(ATSIteratorValue, RefModifiesTS)
  {
    auto s         = TimeSeries::ScalarTs({0., 1., 2.}, {33., 22., 11.});
    *std::begin(s) = 44.;
    EXPECT_EQ(*std::begin(s), 44.);
  }

} // namespace

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
