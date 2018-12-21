

#include <TimeSeries.h>
#include "gtest/gtest.h"
#include <numeric>

namespace {

USER_DECLARE_TS(MyTimeSerie, double);

class STL_ALG_GenericTS : public ::testing::Test {
 protected:
  STL_ALG_GenericTS() {

  }

  virtual ~STL_ALG_GenericTS() {

  }

};

TEST(STL_ALG_GenericTS, CanBeFilledWithIota)
{
  auto s = MyTimeSerie{100};
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

TEST(STL_ALG_GenericTS, CanBeSortedByValues)
{
    auto s = MyTimeSerie{{1.,2,3.},{4.,3,2.}};
    std::sort(std::begin(s),std::end(s));
    std::cout << s.begin()->v();
    std::vector test{1.,2.};
}

TEST(STL_ALG_GenericTS, CanBeUsedWithGenerate)
{
    auto s = MyTimeSerie{100};
    std::generate(std::begin(s), std::end(s), [i=0.]()mutable {return std::pair<double,double>{i++,0.};});
    auto b = s.begin();
    EXPECT_EQ(b->t(),0.);
    EXPECT_EQ((b+99)->t(),99.);
}

TEST(STL_ALG_GenericTS, CanSwapTwoValues)
{
    auto s = MyTimeSerie{{1.,2},{4.,3}};
    auto exp = MyTimeSerie{{2.,1},{3.,4}};
    auto b = s.begin();
    std::swap(*b,*(b+1));
    EXPECT_EQ(*exp.begin(), *b);
    EXPECT_EQ(*(exp.begin()+1), *(b+1));
}

TEST(STL_ALG_GenericTS, CanBeUsedWithAdjDiff)
{
  auto s = MyTimeSerie{10};
  EXPECT_EQ(10, s.size());
  auto b = s.begin();
  auto e = s.end();
  std::iota(b,e,0.);
  double i=0.;
  for(const auto& slice:s)
  {
      EXPECT_EQ(i++, slice.v());
  }
  auto r = MyTimeSerie{10};
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
