#pragma once
#ifndef DS_VERSION_H
#define DS_VERSION_H

// project link to github repo
#define PROJECT_LINK            L"https://github.com/ww-rm/DesktopSprite"

#define __STR(v) #v
#define _STR(v)  __STR(v)

#define __VERSTR(MAJOR, MINOR, PATCH, INNER)    _STR(MAJOR##.##MINOR##.##PATCH##.##INNER)
#define _VERSTR(MAJOR, MINOR, PATCH, INNER)     __VERSTR(MAJOR, MINOR, PATCH, INNER)

#define __SEMANTIC_VERSTR(MAJOR, MINOR, PATCH)  _STR(v##MAJOR##.##MINOR##.##PATCH)
#define _SEMANTIC_VERSTR(MAJOR, MINOR, PATCH)   __SEMANTIC_VERSTR(MAJOR, MINOR, PATCH)

// version num
#define MAJOR_VER               1
#define MINOR_VER               2
#define PATCH_VER               3
#define BUILD_VER               0

// version string vX.Y.Z
#define VERSION_STR             _SEMANTIC_VERSTR(MAJOR_VER, MINOR_VER, PATCH_VER)

// file version
#define FILE_MAJOR_VER          MAJOR_VER
#define FILE_MINOR_VER          MINOR_VER
#define FILE_PATCH_VER          PATCH_VER
#define FILE_BUILD_VER          BUILD_VER
#define FILE_VERSION_VAL        FILE_MAJOR_VER, FILE_MINOR_VER, FILE_PATCH_VER, FILE_BUILD_VER
#define FILE_VERSION_STR        _VERSTR(FILE_MAJOR_VER, FILE_MINOR_VER, FILE_PATCH_VER, FILE_BUILD_VER)

// product version
#define PRODUCT_MAJOR_VER       MAJOR_VER
#define PRODUCT_MINOR_VER       MINOR_VER
#define PRODUCT_PATCH_VER       PATCH_VER
#define PRODUCT_BUILD_VER       BUILD_VER
#define PRODUCT_VERSION_VAL     PRODUCT_MAJOR_VER, PRODUCT_MINOR_VER, PRODUCT_PATCH_VER, PRODUCT_BUILD_VER
#define PRODUCT_VERSION_STR     _VERSTR(PRODUCT_MAJOR_VER, PRODUCT_MINOR_VER, PRODUCT_PATCH_VER, PRODUCT_BUILD_VER)

#endif // !DS_VERSION_H
