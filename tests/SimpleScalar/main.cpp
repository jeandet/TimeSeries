

#include <TimeSeries.h>
#include "gtest/gtest.h"
#include <numeric>

namespace {

class ASimpleScalar : public ::testing::Test {
 protected:
  ASimpleScalar() {

  }

  virtual ~ASimpleScalar() {

  }

};

TEST(ASimpleScalar, CanBeDefaultConstructed) 
{
  auto s = TimeSeries::ScalarTs();
}

TEST(ASimpleScalar, CanBeCopyConstructed) 
{
  auto s = TimeSeries::ScalarTs();
  auto s2 = s;
}

TEST(ASimpleScalar, CanBeFilledWithIota) 
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

TEST(ASimpleScalar, CanBeUsedWithAdjDiff)
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
