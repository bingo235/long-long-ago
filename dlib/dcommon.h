#ifndef dcommon_h__
#define dcommon_h__

#include "platform_config.h"
#include "platform_macros.h"

#include "dtypes.h"

#include <sstream>
#include <iostream>
#define NETLOG_ERROR(MSG)	do{std::ostringstream ss;ss<<MSG<<std::endl;std::cout<<ss.str();}while(0)
#define FUN_FILE_LINE		" ;"<<__FUNCTION__<<"(); File:"<<__FILE__<<" Line:"<<__LINE__
#endif // dcommon_h__
