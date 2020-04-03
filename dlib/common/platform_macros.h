#ifndef PLATFORM_MACROS_h_
#define PLATFORM_MACROS_h_

#define NS_DLIB_BEGIN			namespace dlib{
#define NS_DLIB_END				}

#define SAFE_DELETE(p)			{if(p) { delete (p); (p) = 0; }}
#define SAFE_DELETE_ARRAY(p)	{if(p) { delete[] (p); (p) = 0; }}
#define SAFE_FREE(p)			{if(p) { free(p); (p) = 0; }}

#endif // PLATFORM_MACROS_h_
