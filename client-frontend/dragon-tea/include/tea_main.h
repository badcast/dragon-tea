#pragma once

// MSVC/MinGW Compiller
#if defined(_MSC_VER) || defined(WIN32)
#define API_EXPORT __declspec(dllexport)
#define API_IMPORT __declspec(dllimport)
#elif defined(__linux__)
// Linux/Unix GNU Compiller
#if defined(__GNUC__) && __GNUC__ >= 4
#define __GCCAPI__ __attribute__((visibility("default")))
#else
#define __GCCAPI__
#endif

#define API_EXPORT __GCCAPI__
#define API_IMPORT __GCCAPI__

#endif

#ifdef TEA_COMPILLING
#define TEA_API API_EXPORT
#else
#define TEA_API API_IMPORT
#endif

int TEA_API tea_main(int argc, char *argv[]);
