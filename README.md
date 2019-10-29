[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![CPP17](https://img.shields.io/badge/Language-C++17-blue.svg)]()
[![Build Status](https://travis-ci.org/jeandet/TimeSeries.svg?branch=master)](https://travis-ci.org/jeandet/TimeSeries)
[![Total alerts](https://img.shields.io/lgtm/alerts/g/jeandet/TimeSeries.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/jeandet/TimeSeries/alerts/)
[![Tests](https://hephaistos.lpp.polytechnique.fr/teamcity/app/rest/builds/buildType:(id:TimeSeries_Tests)/statusIcon)](https://hephaistos.lpp.polytechnique.fr/teamcity/viewType.html?buildTypeId=TimeSeries_Tests&guest=1)
[![Sanitizers](https://hephaistos.lpp.polytechnique.fr/teamcity/app/rest/builds/buildType:(id:TimeSeries_Sanitizers)/statusIcon)](https://hephaistos.lpp.polytechnique.fr/teamcity/viewType.html?buildTypeId=TimeSeries_Sanitizers&guest=1)
[![Benchmarks](https://hephaistos.lpp.polytechnique.fr/teamcity/app/rest/builds/buildType:(id:TimeSeries_Benchmarks)/statusIcon)](https://hephaistos.lpp.polytechnique.fr/teamcity/viewType.html?buildTypeId=TimeSeries_Benchmarks&guest=1)


| GCC7              | Clang7            | Clang6            |
|-------------------|-------------------|-------------------|
| [![Build1][1]][5] | [![Build2][2]][5] | [![Build3][3]][5] |

[1]: https://travis-matrix-badges.herokuapp.com/repos/jeandet/TimeSeries/branches/master/1
[2]: https://travis-matrix-badges.herokuapp.com/repos/jeandet/TimeSeries/branches/master/2
[3]: https://travis-matrix-badges.herokuapp.com/repos/jeandet/TimeSeries/branches/master/3
[5]: https://travis-ci.org/jeandet/TimeSeries

# TimeSeries library
A simple C++ time-series library.

## Example Code

```CPP
#include <numeric>
#include <TimeSeries.h>


USER_DECLARE_TS(MyDoubleTs, double);
USER_DECLARE_TS_ND(MyTimeSerie3d, double, 3);

int main()
{
    auto s = MyDoubleTs{100}; // length 100
    std::generate(std::begin(s), std::end(s), [i = 0.]() mutable {
          return std::pair<double, double>{i, i*2.};
          i++
        });

    auto first_value = s[0]; // = 0.
    auto second_value = s[1]; // = 2.
    
    auto s2 = MyTimeSerie3d({100, 10, 5}); //shape 100,10,5
    
    return 0;
}

```


## Installation
### As Meson subproject 
From your project sources:
```bash
mkdir -p subprojects
cat > subprojects/TimeSeries.wrap <<EOF
[wrap-git]
directory = TimeSeries
url = https://github.com/jeandet/TimeSeries.git
revision = master
EOF

```
### From sources
Usually using it a project submodule is enough, if you really want to install it on your system:

```bash
meson build
cd build
ninja
sudo ninja install
```bash
**Uninstalling**:
```
sudo ninja uninstall
```

