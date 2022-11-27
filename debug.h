#ifndef __DEBUG_H__
#define __DEBUG_H__
#ifdef DEBUG
    #define DBG(...) printf(__VA_ARGS__)
#else
    #define DBG(...)
#endif
#endif