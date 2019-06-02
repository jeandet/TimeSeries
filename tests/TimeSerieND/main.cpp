#include "gtest/gtest.h"
#include <TimeSeries.h>
#include <numeric>

namespace
{
  USER_DECLARE_TS_ND(MyTimeSerie2d, double, 2);
  USER_DECLARE_TS_ND(MyTimeSerie3d, double, 3);

  class ATimeSerieND : public ::testing::Test
  {
  protected:
    ATimeSerieND() {}

    virtual ~ATimeSerieND() {}
  };

  TEST(ATimeSerieND, CanBeDefaultConstructed)
  {
    auto s2d = MyTimeSerie2d();
    auto s3d = MyTimeSerie3d();
  }

  TEST(ATimeSerieND, CanCompareIterators)
  {
    auto s   = MyTimeSerie2d({0., 1., 2.}, {33., 22., 11.}, {3, 1});
    auto it1 = s.begin();
    EXPECT_EQ(it1, s.begin());
    EXPECT_NE(it1, s.begin() + 1);
  }

  TEST(ATimeSerieND, CanSetShape)
  {
    {
      auto s = MyTimeSerie2d({100, 10});
      EXPECT_EQ(s.size(), 100);
      EXPECT_EQ(s.size(1), 10);
      EXPECT_EQ(std::vector<std::size_t>({100, 10}), s.shape());
    }
    {
      auto s = MyTimeSerie3d({100, 10, 5});
      EXPECT_EQ(s.size(), 100);
      EXPECT_EQ(s.size(1), 10);
      EXPECT_EQ(s.size(2), 5);
      EXPECT_EQ(std::vector<std::size_t>({100, 10, 5}), s.shape());
      EXPECT_EQ((std::array<std::size_t, 2>({10, 5})), s.begin()->shape());
      EXPECT_EQ((std::array<std::size_t, 1>({5})), s.begin()->begin()->shape());
    }
  }

  TEST(ATimeSerieND, CanSetRawElement)
  {
    auto s     = MyTimeSerie3d({100, 10, 5});
    s[0][0][0] = 123.321;
    EXPECT_DOUBLE_EQ(s[0][0][0], 123.321);
  }

  TEST(ATimeSerieND, HasConsistentSliceShape)
  {
    auto s = MyTimeSerie3d({100, 10, 5});
    auto b = *s.begin();
    EXPECT_EQ((std::array<std::size_t, 2>({10, 5})), b.shape());
    EXPECT_EQ((std::array<std::size_t, 1>({5})), b.begin()->shape());
  }

  TEST(ATimeSerieND, HasConsistentItemIndexes)
  {
    {
      std::vector<double> data(100 * 10);
      std::vector<double> x(100);
      std::generate(std::begin(data), std::end(data), []() {
        static double i = 0;
        return i++;
      });
      std::generate(std::begin(x), std::end(x), []() {
        static double i = 0;
        return i++;
      });
      auto s       = MyTimeSerie2d(x, data, {100, 10});
      double value = 0.;
      for(std::size_t i = 0; i < s.shape()[0]; i++)
      {
        for(std::size_t j = 0; j < s.shape()[1]; j++)
        {
          EXPECT_EQ(value++, s[i][j]);
        }
      }
    }
    {
      std::vector<double> data(100 * 10 * 5);
      std::vector<double> x(100);
      std::generate(std::begin(data), std::end(data), []() {
        static double i = 0;
        return i++;
      });
      std::generate(std::begin(x), std::end(x), []() {
        static double i = 0;
        return i++;
      });
      auto s       = MyTimeSerie3d(x, data, {100, 10, 5});
      double value = 0.;
      for(std::size_t i = 0; i < s.shape()[0]; i++)
      {
        auto b1 = *(s.begin() + i);
        for(std::size_t j = 0; j < s.shape()[1]; j++)
        {
          auto b2 = *(b1.begin() + j);
          for(std::size_t k = 0; k < s.shape()[2]; k++)
          {
            auto b3 = *(b2.begin() + k);
            EXPECT_EQ(value, s[i][j][k]);
            EXPECT_EQ(b3.v(), value++);
          }
        }
      }
    }
  }

  TEST(ATimeSerieND, CanBackInsertValues)
  {
    auto s    = MyTimeSerie2d({0., 1., 2.}, {33., 22., 11.}, {3, 1});
    auto dest = MyTimeSerie2d({0, 1}); // TODO not sure about this...
    std::copy(std::begin(s), std::end(s), std::back_inserter(dest));
    EXPECT_TRUE(std::equal(std::begin(s), std::end(s), std::begin(dest)));
  }

  TEST(ATimeSerieND, CanPushBackValues)
  {
    auto s    = MyTimeSerie2d({0., 1., 2.}, {33., 22., 11.}, {3, 1});
    auto dest = MyTimeSerie2d({0, 1}); // TODO not sure about this...
    for(auto& v : s)
    {
      dest.push_back(v);
    }
    EXPECT_TRUE(std::equal(std::begin(s), std::end(s), std::begin(dest)));
  }

  TEST(ATimeSerieND, CanCompyIteratorsValues)
  {
    auto s  = MyTimeSerie2d({0., 1., 2.}, {33., 22., 11.}, {3, 1});
    auto it = s.begin();
    auto v  = *it;
    auto v2 = v;
    EXPECT_EQ(v, *it);
    EXPECT_EQ(v, v2);
    auto v3 = std::move(v);
    EXPECT_EQ(v3, v2);
  }

  template<typename T> void m_swap(T& t1, T& t2)
  {
    T temp = std::move(t1); // or T temp(std::move(t1));
    t1     = std::move(t2);
    t2     = std::move(temp);
  }

  TEST(ATimeSerieND, CanSwapTwoValuesStepByStep)
  {
    auto s   = MyTimeSerie2d({0., 1.}, {33., 22.}, {2, 1});
    auto exp = MyTimeSerie2d({1., 0.}, {22., 33.}, {2, 1});
    auto b   = s.begin();
    m_swap(*b, *(b + 1));
    EXPECT_EQ(*exp.begin(), *b);
    EXPECT_EQ(*(exp.begin() + 1), *(b + 1));
  }

  TEST(ATimeSerieND, CanSwapTwoValues)
  {
    auto s   = MyTimeSerie2d({0., 1., 2.}, {33., 22., 11.}, {3, 1});
    auto exp = MyTimeSerie2d({1., 0., 2.}, {22., 33., 11.}, {3, 1});
    auto b   = s.begin();
    std::swap(*b, *(b + 1));
    EXPECT_EQ(*exp.begin(), *b);
    EXPECT_EQ(*(exp.begin() + 1), *(b + 1));
  }

  TEST(ATimeSerieND, CanBeSortedByIndex)
  {
    auto s = MyTimeSerie2d{
        {6., 5., 4., 3., 2., 1.}, {1., 2, 3., 4., 5., 6.}, {6, 1}};
    auto sorted = MyTimeSerie2d{
        {1., 2, 3., 4., 5., 6.}, {6., 5., 4., 3., 2., 1.}, {6, 1}};
    std::sort(std::begin(s.byIndex()), std::end(s.byIndex()));
    EXPECT_TRUE(std::equal(std::begin(s), std::end(s), std::begin(sorted)));
  }

} // namespace

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
