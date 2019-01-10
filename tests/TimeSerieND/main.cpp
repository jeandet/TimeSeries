#include <TimeSeries.h>
#include "gtest/gtest.h"
#include <numeric>

namespace
{
  USER_DECLARE_TS_ND(MyTimeSerie2d, double,2);
  USER_DECLARE_TS_ND(MyTimeSerie3d, double,3);

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

} // namespace

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
