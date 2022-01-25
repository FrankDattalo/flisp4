#ifndef DEBUG_HH__
#define DEBUG_HH__

#include <iostream>
#include <iomanip>

#ifndef NDEBUG
#define DEBUG(msg) std::cerr << "[DEBUG] " << std::setw(25) << __FILE__ << "@" << std::setw(15) << __FUNCTION__ << ":" << std::setw(4) << __LINE__ << " - " << msg;
#define IS_DEBUG_ENABLED() true
#else
#define DEBUG(msg) /* DEBUG(msg) */
#define IS_DEBUG_ENABLED() false
#endif

#define DEBUGLN(msg) DEBUG(msg << std::endl);

#endif // DEBUG_H__
