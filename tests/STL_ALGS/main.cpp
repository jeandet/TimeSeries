

#include "gtest/gtest.h"
#include <TimeSeries.h>
#include <numeric>

namespace
{
  USER_DECLARE_TS(MyTimeSerie, double);

  class STL_ALG_GenericTS : public ::testing::Test
  {
  protected:
    STL_ALG_GenericTS() {}

    virtual ~STL_ALG_GenericTS() {}
  };

  TEST(STL_ALG_GenericTS, CanBeFilledWithIota)
  {
    auto s = MyTimeSerie{100};
    EXPECT_EQ(100, s.size());
    auto b = s.begin();
    auto e = s.end();
    std::iota(b, e, 0.);
    double i = 0.;

    for(const auto& slice : s)
    {
      EXPECT_EQ(i++, slice.v());
    }
  }

  TEST(STL_ALG_GenericTS, CanBeUseMoveBackward)
  {
    auto s = MyTimeSerie{{2., 1, 3., 4., 5., 6.}, {5., 6., 4., 3., 2., 1.}};
    auto sorted =
        MyTimeSerie{{6., 5., 4., 3., 2., 1.}, {1., 2, 3., 4., 5., 6.}};
    std::move_backward(std::begin(s), std::end(s) - 1, std::end(s));
    EXPECT_DOUBLE_EQ((std::end(s) - 1)->v(), 2.);
    EXPECT_DOUBLE_EQ((std::end(s) - 2)->v(), 3.);
  }

  TEST(STL_ALG_GenericTS, CanBeSortedByValues)
  {
    auto s      = MyTimeSerie(100);
    auto sorted = MyTimeSerie(100);

    std::generate(std::begin(s), std::end(s), [t = 1., v = s.size()]() mutable {
      return std::pair<double, double>{t++, v--};
    });
    std::generate(std::begin(sorted), std::end(sorted),
                  [t = s.size(), v = 1.]() mutable {
                    return std::pair<double, double>{t--, v++};
                  });

    std::sort(std::begin(s), std::end(s));
    EXPECT_TRUE(std::equal(std::begin(s), std::end(s), std::begin(sorted)));
  }

  TEST(STL_ALG_GenericTS, CanBeSortedByIndex)
  {
    auto s = MyTimeSerie{{6., 5., 4., 3., 2., 1.}, {1., 2, 3., 4., 5., 6.}};
    auto sorted =
        MyTimeSerie{{1., 2, 3., 4., 5., 6.}, {6., 5., 4., 3., 2., 1.}};
    std::sort(std::begin(s.byIndex()), std::end(s.byIndex()));
    EXPECT_TRUE(std::equal(std::begin(s), std::end(s), std::begin(sorted)));
  }

  TEST(STL_ALG_GenericTS, CanBeUsedWithGenerate)
  {
    auto s = MyTimeSerie{100};
    std::generate(std::begin(s), std::end(s), [i = 0.]() mutable {
      return std::pair<double, double>{i++, 0.};
    });
    auto b = s.begin();
    EXPECT_EQ(b->t(), 0.);
    EXPECT_EQ((b + 99)->t(), 99.);
  }

  TEST(STL_ALG_GenericTS, CanBackInsertCopy)
  {
    auto exp = MyTimeSerie{{6., 5., 4., 3., 2., 1.}, {1., 2, 3., 4., 5., 6.}};
    auto s   = MyTimeSerie{};
    std::copy(std::begin(exp), std::end(exp), std::back_inserter(s));
    EXPECT_TRUE(std::equal(std::begin(s), std::end(s), std::begin(exp)));
  }

  TEST(STL_ALG_GenericTS, CanSwapTwoValues)
  {
    auto s   = MyTimeSerie{{1., 2}, {4., 3}};
    auto exp = MyTimeSerie{{2., 1}, {3., 4}};
    auto b   = s.begin();
    std::swap(*b, *(b + 1));
    EXPECT_EQ(*exp.begin(), *b);
    EXPECT_EQ(*(exp.begin() + 1), *(b + 1));
  }

  TEST(STL_ALG_GenericTS, CanBeUsedWithAdjDiff)
  {
    auto s = MyTimeSerie{10};
    EXPECT_EQ(10, s.size());
    auto b = s.begin();
    auto e = s.end();
    std::iota(b, e, 0.);
    double i = 0.;
    for(const auto& slice : s)
    {
      EXPECT_EQ(i++, slice.v());
    }
    auto r = MyTimeSerie{10};
    auto o = r.begin();
    std::adjacent_difference(b, e, o);
    EXPECT_EQ(0., r.begin()->v());
    for(const auto& slice : r[{1, r.size()}])
    {
      EXPECT_DOUBLE_EQ(1., slice.v());
    }
  }

} // namespace

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
