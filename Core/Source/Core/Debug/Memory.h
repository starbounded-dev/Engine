#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <mutex>

namespace Core::Debug
{
    // Memory allocation category
    enum class MemoryCategory
    {
        Unknown = 0,
        Texture,
        Buffer,
        Shader,
        Mesh,
        Framebuffer,
        Other
    };

    // Memory allocation info
    struct AllocationInfo
    {
        void* Address = nullptr;
        size_t Size = 0;
        MemoryCategory Category = MemoryCategory::Unknown;
        std::string Tag;
        const char* File = nullptr;
        int Line = 0;
    };

    // Memory statistics per category
    struct MemoryStats
    {
        size_t TotalAllocated = 0;
        size_t TotalFreed = 0;
        size_t CurrentUsage = 0;
        size_t AllocationCount = 0;
        size_t FreeCount = 0;
    };

    // GPU memory statistics (queried from OpenGL)
    struct GPUMemoryStats
    {
        // Total GPU memory (KB)
        uint64_t TotalMemoryKB = 0;
        
        // Available GPU memory (KB)
        uint64_t AvailableMemoryKB = 0;
        
        // Current GPU memory usage (KB)
        uint64_t CurrentUsageKB = 0;
        
        // Dedicated video memory (KB) - for NVIDIA
        uint64_t DedicatedVideoMemoryKB = 0;
        
        // Total available memory (KB) - for NVIDIA
        uint64_t TotalAvailableMemoryKB = 0;
        
        // Current available memory (KB) - for NVIDIA
        uint64_t CurrentAvailableMemoryKB = 0;
        
        // Eviction count - for NVIDIA
        uint64_t EvictionCount = 0;
        
        // Evicted memory (KB) - for NVIDIA
        uint64_t EvictedMemoryKB = 0;
    };

    // Memory tracking system
    class Memory
    {
    public:
        // Track allocation
        static void TrackAllocation(void* address, size_t size, MemoryCategory category, 
                                   const std::string& tag = "", const char* file = nullptr, int line = 0);
        
        // Track deallocation
        static void TrackDeallocation(void* address);
        
        // Get memory statistics
        static MemoryStats GetStats(MemoryCategory category);
        static MemoryStats GetTotalStats();
        
        // Get GPU memory statistics
        static GPUMemoryStats GetGPUStats();
        
        // Get all allocations for a category
        static std::unordered_map<void*, AllocationInfo> GetAllocations(MemoryCategory category);
        
        // Clear all tracking data
        static void Clear();
        
        // Get category name
        static const char* GetCategoryName(MemoryCategory category);
        
        // Print memory report
        static void PrintReport();

    private:
        static std::unordered_map<void*, AllocationInfo>& GetAllocationMap();
        static std::mutex& GetMutex();
    };

    // Helper macros for tracking
    #define TRACK_ALLOC(ptr, size, category, tag) \
        Core::Debug::Memory::TrackAllocation(ptr, size, category, tag, __FILE__, __LINE__)
    
    #define TRACK_FREE(ptr) \
        Core::Debug::Memory::TrackDeallocation(ptr)
}
