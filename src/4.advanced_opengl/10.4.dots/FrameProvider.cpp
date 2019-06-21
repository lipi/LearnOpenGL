

#include <cstdlib>

#include <sqlite3.h>
#include "spdlog/spdlog.h"

#include "FrameProvider.h"


FrameProvider::FrameProvider(const char* filename)
    : m_Db(filename),
      m_TimestampsQuery(m_Db, "SELECT timestamp FROM keyframe"),
      m_FrameQuery(m_Db, "SELECT frame FROM keyframe WHERE timestamp = :timestamp")
{
    spdlog::info("Opened %s, found %u frames", filename, (uint32_t)GetTimestamps().size());
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

size_t FrameProvider::GetFrame(uint32_t timestamp, glm::vec3* buffer, size_t numLocations) {
    const void* blobData = NULL;
    size_t blobSize;
    size_t locationSize = sizeof(float) * 2;
    
    m_FrameQuery.bind(":timestamp", timestamp);
    if (m_FrameQuery.executeStep()) {
        SQLite::Column colBlob = m_FrameQuery.getColumn(0);
        blobData = colBlob.getBlob();
        // printf("blobData: %p\n", blobData);
        blobSize = colBlob.getBytes();
        if (blobSize / locationSize < numLocations) {
            numLocations = blobSize / locationSize;
        }

        float* floatData = (float*)blobData;
         spdlog::debug("converting %zu floats (%zu locations)\n", 2 * numLocations, numLocations);
        for (size_t i = 0; i < numLocations; i++) {
            buffer[i] = glm::vec3(floatData[2 * i], floatData[2 * i+1], 0);
        }
    }
    m_FrameQuery.clearBindings();
    m_FrameQuery.reset();
    return numLocations;
}
