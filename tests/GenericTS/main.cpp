#include "gtest/gtest.h"
#include <TimeSeries.h>
#include <numeric>

namespace
{
  USER_DECLARE_TS(MyTimeSerie, double);

  class AGenericTS : public ::testing::Test
  {
  protected:
    AGenericTS() {}

    virtual ~AGenericTS() {}
  };

  TEST(AGenericTS, CanBeDefaultConstructed) { auto s = MyTimeSerie(); }

  TEST(AGenericTS, CanBeCopyConstructed)
  {
    // this has at least to buidl
    auto s  = MyTimeSerie({0., 1., 2.}, {33., 22., 11.});
    auto s2 = s;
    EXPECT_EQ(3, s.size());
    EXPECT_EQ(3, s2.size());
  }

  TEST(AGenericTS, CanSetByPair)
  {
    auto s  = MyTimeSerie({0., 1., 2.}, {33., 22., 11.});
    auto it = s.begin();
    *it     = std::pair{11., 99.};
    EXPECT_EQ(11., it->t());
    EXPECT_EQ(99., it->v());
  }

  TEST(AGenericTS, CanSetTimeWithSecondStrongType)
  {
    auto s  = TimeSeries::ScalarTs({0., 1., 2.}, {33., 22., 11.});
    auto it = s.begin() + 1;
    *(it)   = TimeSeries::Second(10.);
    EXPECT_EQ(10., it->t());
    EXPECT_EQ(22., it->v());
  }

  TEST(AGenericTS, CanCompareIterators)
  {
    auto s   = TimeSeries::ScalarTs({0., 1., 2.}, {33., 22., 11.});
    auto it1 = s.begin();
    EXPECT_EQ(it1, s.begin());
    EXPECT_NE(it1, s.begin() + 1);
  }

  TEST(AGenericTS, CanCompyIteratorsValues)
  {
    auto s  = TimeSeries::ScalarTs({0., 1., 2.}, {33., 22., 11.});
    auto it = s.begin();
    auto v  = *it;
    auto v2 = v;
    EXPECT_EQ(v, *it);
    EXPECT_EQ(v, v2);
    auto v3 = std::move(v);
    EXPECT_EQ(v3, v2);
  }

  TEST(AGenericTS, CanSetValueFromIterators)
  {
    auto s  = TimeSeries::ScalarTs({0., 1., 2.}, {33., 22., 11.});
    auto it = s.begin();
    EXPECT_EQ(it->v(), 33.);
    *it = 1234.;
    EXPECT_EQ(it->v(), 1234.);
  }

  TEST(AGenericTS, CanRangeBasedLoopIterators)
  {
    auto s = TimeSeries::ScalarTs({0., 1., 2.}, {33., 22., 11.});
    for(auto& v : s)
    {
      v = 1111.;
    }
    for(const auto& v : s)
    {
      EXPECT_EQ(v.v(), 1111.);
    }
  }

} // namespace

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
