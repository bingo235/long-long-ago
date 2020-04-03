#ifndef PLATFORM_CONFIG_H__
#define PLATFORM_CONFIG_H__


#define DLIB_OS_LINUX			0x0001
#define DLIB_OS_WINDOWS			0x0002

#if defined(linux) || defined(__linux) || defined(__linux__) || defined(__TOS_LINUX__)
#define POCO_OS_FAMILY_UNIX 1
#define POCO_OS DLIB_OS_LINUX
#elif defined(_WIN32) || defined(_WIN64)
#define DLIB_OS DLIB_OS_WINDOWS
#endif

#if !defined(DLIB_OS)
#error "Unknown Platform."
#endif



#if defined(DLIB_OS_WINDOWS)

#ifndef WINVER
#define WINVER 0x0501
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0501
#endif

#if defined(_MSC_VER) && !defined(_MT)
#error Must compile with /MD, /MDd, /MT or /MTd
#endif


// Check debug/release settings consistency
#if defined(NDEBUG) && defined(_DEBUG)
#error Inconsistent build settings (check for /MD[d])
#endif

// Turn off some annoying warnings
#if defined(_MSC_VER)
#pragma warning(disable:4018)	// signed/unsigned comparison
#pragma warning(disable:4250)	// VC++ 11.0: inheriting from std stream classes produces C4250 warning;
// see <http://connect.microsoft.com/VisualStudio/feedback/details/733720/inheriting-from-std-fstream-produces-c4250-warning>
#pragma warning(disable:4251)	// ... needs to have dll-interface warning
#pragma warning(disable:4275)	// non dll-interface class 'std::exception' used as base for dll-interface class 'Poco::Exception'
#pragma warning(disable:4344)	// behavior change: use of explicit template arguments results in call to '...' but '...' is a better match
#pragma warning(disable:4351)	// new behavior: elements of array '...' will be default initialized
#pragma warning(disable:4355)	// 'this' : used in base member initializer list
#pragma warning(disable:4675)	// resolved overload was found by argument-dependent lookup
#pragma warning(disable:4996)	// VC++ 8.0 deprecation warnings
#endif

#endif


#endif // PLATFORM_CONFIG_H__

