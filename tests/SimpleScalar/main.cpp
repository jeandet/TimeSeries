

#include "gtest/gtest.h"
#include <TimeSeries.h>
#include <math.h>
#include <numeric>

namespace
{
  class ASimpleScalar : public ::testing::Test
  {
  protected:
    ASimpleScalar() {}

    virtual ~ASimpleScalar() {}
  };

  TEST(ASimpleScalar, CanBeDefaultConstructed)
  {
    auto s = TimeSeries::ScalarTs();
  }

  TEST(ASimpleScalar, CanBeCopyConstructed)
  {
    // this has at least to buidl
    auto s     = TimeSeries::ScalarTs({0., 1., 2.}, {33., 22., 11.});
    auto s2    = s;
    *s.begin() = 0.;
    EXPECT_EQ(3, s.size());
    EXPECT_EQ(3, s2.size());
    EXPECT_EQ(33., s2.begin()->v());
  }

  TEST(ASimpleScalar, CanSetByPair)
  {
    // this has at least to buidl
    auto s     = TimeSeries::ScalarTs({0., 1., 2.}, {33., 22., 11.});
    auto s2    = s;
    *s.begin() = 0.;
    EXPECT_EQ(3, s.size());
    EXPECT_EQ(3, s2.size());
    EXPECT_EQ(33., s2.begin()->v());
  }

  TEST(ASimpleScalar, CanSetTimeWithSecondStrongType)
  {
    // this has at least to buidl
    auto s  = TimeSeries::ScalarTs({0., 1., 2.}, {33., 22., 11.});
    auto it = s.begin() + 1;
    *(it)   = TimeSeries::Second(10.);
    EXPECT_EQ(10., it->t());
    EXPECT_EQ(22., it->v());
  }

  TEST(ASimpleScalar, CanComputeModOnVectorTs)
  {
    TimeSeries::ScalarTs s{100};
    TimeSeries::VectorTs v{100};
    std::generate(std::begin(v), std::end(v), [i = 0.]() mutable {
      i += 0.01;
      auto x = sin(i) * sin(i);
      auto y = cos(i) * sin(i);
      auto z = cos(i);
      return std::pair{i, TimeSeries::VectorTs::value_type{x, y, z}};
    });
    std::transform(std::begin(v), std::end(v), std::begin(s),
                   [](const auto& item) {
                     const auto& [x, y, z] = item.v();
                     auto mod              = sqrt(x * x + y * y + z * z);
                     return std::pair{item.t(), mod};
                   });
    auto min = std::min_element(std::begin(s), std::end(s));
    auto max = std::max_element(std::begin(s), std::end(s));
    EXPECT_EQ(1., min->v());
    EXPECT_EQ(1., max->v());
  }

} // namespace

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
