#ifndef TYPES_H__
#define TYPES_H__

namespace dlib{

#if defined(_MSC_VER)
    // Windows/Visual C++
    typedef signed char            	int8_t;
    typedef unsigned char          	uint8_t;
    typedef signed short           	int16_t;
    typedef unsigned short         	uint16_t;
    typedef signed int             	int32_t;
    typedef unsigned int           	uint32_t;
    typedef signed __int64         	int64_t;
    typedef unsigned __int64       	uint64_t;
#if defined(_WIN64)
#define DLIB_PTR_IS_64_BIT 1
    typedef signed __int64			intptr_t;
    typedef unsigned __int64		uintptr_t;
#else
    typedef signed long				intptr_t;
    typedef unsigned long			uintptr_t;
#endif
#define DLIB_HAVE_INT64 1
#endif

}

#endif // TYPES_H__
