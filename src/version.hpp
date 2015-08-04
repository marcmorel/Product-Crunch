#ifndef __VERSION_HPP
#define __VERSION_HPP
#endif

//increment this string before pushing your branch
#define __BINARY_CRUNCH_VERSION "1.2.7"


#ifdef __DEBUG__
    #define __BINARY_TYPE "debug"
#else
    #define __BINARY_TYPE "release"
#endif

#define __DETAIL__CRUNCH_VERSION "crunch version : " __BINARY_CRUNCH_VERSION " " __BINARY_TYPE " \n builded the " __DATE__ " at " __TIME__ "\n"
#define crunch_version() \
    std::cout << __DETAIL__CRUNCH_VERSION;
