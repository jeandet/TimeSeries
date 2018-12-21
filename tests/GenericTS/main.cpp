#include <TimeSeries.h>
#include "gtest/gtest.h"
#include <numeric>

namespace {

USER_DECLARE_TS(MyTimeSerie, double);


class AGenericTS : public ::testing::Test {
 protected:
  AGenericTS() {

  }

  virtual ~AGenericTS() {

  }

};

TEST(AGenericTS, CanBeDefaultConstructed)
{
  auto s = MyTimeSerie();
}

TEST(AGenericTS, CanBeCopyConstructed)
{
    //this has at least to buidl
    auto s = MyTimeSerie({0.,1.,2.},{33.,22.,11.});
    auto s2 = s;
    EXPECT_EQ(3, s.size());
    EXPECT_EQ(3, s2.size());
}

TEST(AGenericTS, CanSetByPair)
{
    auto s = MyTimeSerie({0.,1.,2.},{33.,22.,11.});
    auto it = s.begin();
    *it = {11., 99.};
    EXPECT_EQ(11., it->t());
    EXPECT_EQ(99., it->v());
}

TEST(AGenericTS, CanSetTimeWithSecondStrongType)
{
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
