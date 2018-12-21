

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
    //this has at least to buidl
    auto s = TimeSeries::ScalarTs({0.,1.,2.},{33.,22.,11.});
    auto s2 = s;
    *s.begin() = 0.;
    EXPECT_EQ(3, s.size());
    EXPECT_EQ(3, s2.size());
    EXPECT_EQ(33., s2.begin()->v());
}

TEST(ASimpleScalar, CanSetByPair)
{
    //this has at least to buidl
    auto s = TimeSeries::ScalarTs({0.,1.,2.},{33.,22.,11.});
    auto s2 = s;
    *s.begin() = 0.;
    EXPECT_EQ(3, s.size());
    EXPECT_EQ(3, s2.size());
    EXPECT_EQ(33., s2.begin()->v());
}

TEST(ASimpleScalar, CanSetTimeWithSecondStrongType)
{
    //this has at least to buidl
    auto s = TimeSeries::ScalarTs({0.,1.,2.},{33.,22.,11.});
    auto it = s.begin()+1;
    *(it) = TimeSeries::Second(10.);
    EXPECT_EQ(10.,it->t());
    EXPECT_EQ(22.,it->v());
}


}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
