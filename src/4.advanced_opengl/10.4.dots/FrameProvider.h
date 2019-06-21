#ifndef __FRAMEPROVIDER_H__
#define __FRAMEPROVIDER_H__

#include <vector>

#include <SQLiteCpp/SQLiteCpp.h>

class FrameProvider  {
    
    private:
        SQLite::Database m_Db;
        SQLite::Statement m_TimestampsQuery;
        SQLite::Statement m_FrameQuery;
        
    public:
        FrameProvider(const char* filename);
        std::vector<uint32_t> GetTimestamps();
        size_t GetFrame(uint32_t timestamp, float* buffer, size_t size);
};

#endif // __FRAMEPROVIDER_H__

