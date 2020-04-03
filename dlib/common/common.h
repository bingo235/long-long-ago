#ifndef COMMON_H__
#define COMMON_H__

#include "std_head.h"
#include "types.h"
#include "platform_config.h"
#include "platform_macros.h"


////log
#include <sstream>
#include <iostream>
#define NETLOG_ERROR(MSG)	do{std::ostringstream ss;ss<<MSG<<std::endl;std::cout<<ss.str();}while(0)
#define FUN_FILE_LINE		" ;"<<__FUNCTION__<<"(); File:"<<__FILE__<<" Line:"<<__LINE__

#endif // COMMON_H__
