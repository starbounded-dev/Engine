#include "Memory.h"
#include "Profiler.h"
#include <glad/glad.h>
#include <iostream>
#include <iomanip>

namespace Core::Debug
{
    std::unordered_map<void*, AllocationInfo>& Memory::GetAllocationMap()
    {
        static std::unordered_map<void*, AllocationInfo> s_Allocations;
        return s_Allocations;
    }

    std::mutex& Memory::GetMutex()
    {
        static std::mutex s_Mutex;
        return s_Mutex;
    }

    void Memory::TrackAllocation(void* address, size_t size, MemoryCategory category, 
                                const std::string& tag, const char* file, int line)
    {
        if (!address) return;

        std::lock_guard<std::mutex> lock(GetMutex());
        
        AllocationInfo info;
        info.Address = address;
        info.Size = size;
        info.Category = category;
        info.Tag = tag;
        info.File = file;
        info.Line = line;
        
        GetAllocationMap()[address] = info;
    }

    void Memory::TrackDeallocation(void* address)
    {
        if (!address) return;

        std::lock_guard<std::mutex> lock(GetMutex());
        GetAllocationMap().erase(address);
    }

    MemoryStats Memory::GetStats(MemoryCategory category)
    {
        std::lock_guard<std::mutex> lock(GetMutex());
        
        MemoryStats stats;
        for (const auto& [addr, info] : GetAllocationMap())
        {
            if (info.Category == category)
            {
                stats.CurrentUsage += info.Size;
                stats.AllocationCount++;
            }
        }
        
        return stats;
    }

    MemoryStats Memory::GetTotalStats()
    {
        std::lock_guard<std::mutex> lock(GetMutex());
        
        MemoryStats stats;
        for (const auto& [addr, info] : GetAllocationMap())
        {
            stats.CurrentUsage += info.Size;
            stats.AllocationCount++;
        }
        
        return stats;
    }

    GPUMemoryStats Memory::GetGPUStats()
    {
        // Define OpenGL extension constants (if not already defined)
        #ifndef GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX
        #define GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX    0x9047
        #define GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX 0x9048
        #define GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX 0x9049
        #define GL_GPU_MEMORY_INFO_EVICTION_COUNT_NVX      0x904A
        #define GL_GPU_MEMORY_INFO_EVICTED_MEMORY_NVX      0x904B
        #endif
        
        #ifndef GL_VBO_FREE_MEMORY_ATI
        #define GL_VBO_FREE_MEMORY_ATI                     0x87FB
        #define GL_TEXTURE_FREE_MEMORY_ATI                 0x87FC
        #define GL_RENDERBUFFER_FREE_MEMORY_ATI            0x87FD
        #endif
        
        GPUMemoryStats stats;
        
        // Try NVIDIA specific extension (NVX_gpu_memory_info)
        GLint nvidiaSupported = 0;
        glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &nvidiaSupported);
        
        if (nvidiaSupported > 0)
        {
            GLint totalMemoryKB = 0;
            GLint availableMemoryKB = 0;
            
            glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &totalMemoryKB);
            glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &availableMemoryKB);
            
            stats.DedicatedVideoMemoryKB = totalMemoryKB;
            stats.CurrentAvailableMemoryKB = availableMemoryKB;
            stats.TotalMemoryKB = totalMemoryKB;
            stats.AvailableMemoryKB = availableMemoryKB;
            stats.CurrentUsageKB = totalMemoryKB - availableMemoryKB;
            
            // Eviction stats
            GLint evictionCount = 0;
            GLint evictedMemory = 0;
            glGetIntegerv(GL_GPU_MEMORY_INFO_EVICTION_COUNT_NVX, &evictionCount);
            glGetIntegerv(GL_GPU_MEMORY_INFO_EVICTED_MEMORY_NVX, &evictedMemory);
            
            stats.EvictionCount = evictionCount;
            stats.EvictedMemoryKB = evictedMemory;
        }
        // Try ATI/AMD specific extension (ATI_meminfo)
        else
        {
            GLint memInfo[4] = {0};
            glGetIntegerv(GL_VBO_FREE_MEMORY_ATI, memInfo);
            
            if (memInfo[0] > 0)
            {
                stats.AvailableMemoryKB = memInfo[0];
                stats.TotalMemoryKB = memInfo[0]; // Approximate
                stats.CurrentUsageKB = 0; // Can't easily determine
            }
        }
        
        return stats;
    }

    std::unordered_map<void*, AllocationInfo> Memory::GetAllocations(MemoryCategory category)
    {
        std::lock_guard<std::mutex> lock(GetMutex());
        
        std::unordered_map<void*, AllocationInfo> result;
        for (const auto& [addr, info] : GetAllocationMap())
        {
            if (info.Category == category)
            {
                result[addr] = info;
            }
        }
        
        return result;
    }

    void Memory::Clear()
    {
        std::lock_guard<std::mutex> lock(GetMutex());
        GetAllocationMap().clear();
    }

    const char* Memory::GetCategoryName(MemoryCategory category)
    {
        switch (category)
        {
            case MemoryCategory::Texture:      return "Texture";
            case MemoryCategory::Buffer:       return "Buffer";
            case MemoryCategory::Shader:       return "Shader";
            case MemoryCategory::Mesh:         return "Mesh";
            case MemoryCategory::Framebuffer:  return "Framebuffer";
            case MemoryCategory::Other:        return "Other";
            default:                           return "Unknown";
        }
    }

    void Memory::PrintReport()
    {
        std::lock_guard<std::mutex> lock(GetMutex());
        
        std::cout << "\n========== Memory Report ==========\n";
        
        MemoryStats categories[(int)MemoryCategory::Other + 1];
        
        // Collect stats for each category
        for (const auto& [addr, info] : GetAllocationMap())
        {
            int idx = (int)info.Category;
            categories[idx].CurrentUsage += info.Size;
            categories[idx].AllocationCount++;
        }
        
        // Print stats
        for (int i = 0; i <= (int)MemoryCategory::Other; i++)
        {
            if (categories[i].AllocationCount > 0)
            {
                std::cout << std::setw(15) << GetCategoryName((MemoryCategory)i) << ": "
                         << std::setw(10) << (categories[i].CurrentUsage / 1024) << " KB  ("
                         << categories[i].AllocationCount << " allocations)\n";
            }
        }
        
        // GPU stats
        GPUMemoryStats gpuStats = GetGPUStats();
        if (gpuStats.TotalMemoryKB > 0)
        {
            std::cout << "\n========== GPU Memory ==========\n";
            std::cout << "Total:      " << (gpuStats.TotalMemoryKB / 1024) << " MB\n";
            std::cout << "Used:       " << (gpuStats.CurrentUsageKB / 1024) << " MB\n";
            std::cout << "Available:  " << (gpuStats.AvailableMemoryKB / 1024) << " MB\n";
        }
        
        std::cout << "===================================\n\n";
    }
}
