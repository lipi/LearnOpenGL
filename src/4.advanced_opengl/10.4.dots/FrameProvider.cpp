

#include <cstdlib>

#include <sqlite3.h>

#include "FrameProvider.h"


FrameProvider::FrameProvider(const char* filename)
    : m_Db(filename),
      m_TimestampsQuery(m_Db, "SELECT timestamp FROM keyframe"),
      m_FrameQuery(m_Db, "SELECT frame FROM keyframe WHERE timestamp = :timestamp")
{
    printf( "Opened %s, found %u frames\n", filename, (uint32_t)GetTimestamps().size());
}

std::vector<uint32_t> FrameProvider::GetTimestamps() {
    std::vector<uint32_t> timestamps;
    
    // Loop to execute the query step by step, to get one a row of results at a time
    while (m_TimestampsQuery.executeStep()) {
        timestamps.push_back(m_TimestampsQuery.getColumn(0));
    }
    m_TimestampsQuery.reset();
    return timestamps;
}

size_t FrameProvider::GetFrame(uint32_t timestamp, float* buffer, size_t size) {
    const void* blobData = NULL;
    size_t blobSize;
    size_t actualBytes;
    
    m_FrameQuery.bind(":timestamp", timestamp);
    if (m_FrameQuery.executeStep()) {
        SQLite::Column colBlob = m_FrameQuery.getColumn(0);
        blobData = colBlob.getBlob();
        // printf("blobData: %p\n", blobData);
        blobSize = colBlob.getBytes();
        actualBytes =  size * sizeof(float) < blobSize ? size * sizeof(float) : blobSize;
        // for (int i = 0; i < 16; i++) {
        //     printf("%02x ", ((unsigned char*)blobData)[i]);
        // }
        // printf("\n");

#if 1
        memcpy((void*)buffer, blobData, actualBytes);
#else
        std::pair<float, float> world;
        float* floatData = (float*)blobData;
        size_t floats = actualBytes / sizeof(float);
        printf("converting %zu floats (%zu locations)\n", floats, floats/2);
        for (size_t i = 0; i < floats; i += 2) {
            world = m_LocationConverter.MapToWorldCoordinate( floatData[i], floatData[i+1]);
            buffer[i] = world.second * width + width/2;
            buffer[i+1] = world.first * height + height/2;
            // if ( i < 10) {
            //     printf("lat/lon %f,%f --> world %f,%f --> x/y %f,%f\n",
            //            floatData[i], floatData[i+1],
            //            world.first, world.second,
            //            buffer[i], buffer[i+1]);
            // }
        }
#endif
        // printf("blobData: %p\n", blobData);
        // sqlite3_blob_close((sqlite3_blob*)blobData);
    }
    m_FrameQuery.clearBindings();
    m_FrameQuery.reset();
    return actualBytes / sizeof(float);
}
