

#include <TimeSeries.h>
#include "gtest/gtest.h"
#include <numeric>

namespace {

class STL_ALG_ASimpleScalar : public ::testing::Test {
 protected:
  STL_ALG_ASimpleScalar() {

  }

  virtual ~STL_ALG_ASimpleScalar() {

  }

};

TEST(STL_ALG_ASimpleScalar, CanBeFilledWithIota)
{
  auto s = TimeSeries::ScalarTs{100};
  EXPECT_EQ(100, s.size());
  auto b = s.begin();
  auto e = s.end();
  std::iota(b,e,0.);
  double i=0.;
  for(const auto& slice:s)
  {
      EXPECT_EQ(i++, slice.v());
  }
}

TEST(STL_ALG_ASimpleScalar, CanBeSortedByValues)
{
    auto s = TimeSeries::ScalarTs{{1.,2,3.},{4.,3,2.}};
    std::sort(std::begin(s),std::end(s));
    std::cout << s.begin()->v();
}

TEST(STL_ALG_ASimpleScalar, CanSwapTwoValues)
{
    auto s = TimeSeries::ScalarTs{{1.,2},{4.,3}};
    //std::swap(*s.begin(),*(s.begin()+1));
    std::cout << s.begin()->v();
}

TEST(STL_ALG_ASimpleScalar, CanBeUsedWithAdjDiff)
{
  auto s = TimeSeries::ScalarTs{10};
  EXPECT_EQ(10, s.size());
  auto b = s.begin();
  auto e = s.end();
  std::iota(b,e,0.);
  double i=0.;
  for(const auto& slice:s)
  {
      EXPECT_EQ(i++, slice.v());
  }
  auto r = TimeSeries::ScalarTs{10};
  auto o = r.begin();
  std::adjacent_difference(b,e,o);
  EXPECT_EQ(0., r.begin()->v());
  for(const auto& slice:r[{1,r.size()}])
  {
      EXPECT_EQ(1., slice.v());
  }
}

}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
