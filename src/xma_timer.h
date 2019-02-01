#pragma once


namespace xma {

class Timer
{
public:  
    /* returns the time as the number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC). */
    static uint32_t GetTime();
    static uint64_t GetMsecs();    
};

}
