#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"

#include "acceleration_structure_system.h"

#include "utils.h"

#include "task_update_tlas.h"

FUpdateTLASTask::FUpdateTLASTask(uint32_t WidthIn, uint32_t HeightIn, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, NumberOfSimultaneousSubmits, LogicalDevice)
{
    Name = "Update TLAS pipeline";

    PipelineStageFlags = VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
    QueueFlagsBits = VK_QUEUE_COMPUTE_BIT;
}

FUpdateTLASTask::~FUpdateTLASTask()
{
	RESOURCE_ALLOCATOR()->DestroyBuffer(ScratchBuffer);
}

void FUpdateTLASTask::Init()
{
    TIMING_MANAGER()->RegisterTiming(Name, NumberOfSimultaneousSubmits);
};

void FUpdateTLASTask::UpdateDescriptorSets()
{
};

void FUpdateTLASTask::RecordCommands()
{
    CommandBuffers.resize(NumberOfSimultaneousSubmits);

	VkAccelerationStructureGeometryInstancesDataKHR AccelerationStructureGeometryInstancesData = VK_CONTEXT()->GetAccelerationStructureGeometryInstancesData(ACCELERATION_STRUCTURE_SYSTEM()->DeviceBuffer);
	VkAccelerationStructureGeometryKHR AccelerationStructureGeometry = VK_CONTEXT()->GetAccelerationStructureGeometry(AccelerationStructureGeometryInstancesData);
	VkAccelerationStructureBuildGeometryInfoKHR AccelerationStructureBuildGeometry = VK_CONTEXT()->GetAccelerationStructureBuildGeometryInfo(AccelerationStructureGeometry, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR, VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
	AccelerationStructureBuildGeometry.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
	VkAccelerationStructureBuildSizesInfoKHR AccelerationStructureBuildSizesInfo = VK_CONTEXT()->GetAccelerationStructureBuildSizesInfo(AccelerationStructureBuildGeometry, ACCELERATION_STRUCTURE_SYSTEM()->InstanceCount);

	if (ScratchBuffer.BufferSize != AccelerationStructureBuildSizesInfo.buildScratchSize)
	{
		RESOURCE_ALLOCATOR()->DestroyBuffer(ScratchBuffer);
		ScratchBuffer = RESOURCE_ALLOCATOR()->CreateBuffer(AccelerationStructureBuildSizesInfo.buildScratchSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "V::TLAS_Scratch_Buffer");
	}

	AccelerationStructureBuildGeometry.srcAccelerationStructure = ACCELERATION_STRUCTURE_SYSTEM()->TLAS.AccelerationStructure;
	AccelerationStructureBuildGeometry.dstAccelerationStructure = ACCELERATION_STRUCTURE_SYSTEM()->TLAS.AccelerationStructure;
	AccelerationStructureBuildGeometry.scratchData.deviceAddress = VK_CONTEXT()->GetBufferDeviceAddressInfo(ScratchBuffer);

	VkAccelerationStructureBuildRangeInfoKHR AccelerationStructureBuildRangeInfo{};
	AccelerationStructureBuildRangeInfo.primitiveCount = ACCELERATION_STRUCTURE_SYSTEM()->InstanceCount;
	const VkAccelerationStructureBuildRangeInfoKHR* AccelerationStructureBuildRangeInfoPtr = &AccelerationStructureBuildRangeInfo;

    for (std::size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        CommandBuffers[i] = COMMAND_BUFFER_MANAGER()->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            TIMING_MANAGER()->TimestampStart(Name, CommandBuffer, i);

			VkMemoryBarrier MemoryBarrier{};
			MemoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
			MemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			MemoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
			vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1,
				&MemoryBarrier, 0, nullptr, 0, nullptr);


			V::vkCmdBuildAccelerationStructuresKHR(CommandBuffer, 1, &AccelerationStructureBuildGeometry, &AccelerationStructureBuildRangeInfoPtr);

            TIMING_MANAGER()->TimestampEnd(Name, CommandBuffer, i);
        }, QueueFlagsBits);

        V::SetName(LogicalDevice, CommandBuffers[i], Name);
    }
};
