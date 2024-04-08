#include "task_imgui.h"
#include "vk_context.h"
#include "vk_debug.h"

#include "window_manager.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "ImGuiProfilerRenderer.h"

#include <iostream>

FImguiTask::FImguiTask(uint32_t WidthIn, uint32_t HeightIn, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, NumberOfSimultaneousSubmits, LogicalDevice)
{
    Name = "Imgui pipeline";

    PipelineStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
}

FImguiTask::~FImguiTask()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    for (auto Framebuffer : ImguiFramebuffers)
    {
        vkDestroyFramebuffer(LogicalDevice, Framebuffer, nullptr);
    }

    vkDestroyRenderPass(LogicalDevice, RenderPass, nullptr);

    vkDestroyDescriptorPool(LogicalDevice, DescriptorPool, nullptr);
}

void FImguiTask::Init()
{
    TIMING_MANAGER()->RegisterTiming(Name, NumberOfSimultaneousSubmits);
    bFirstCall = true;

    FGraphicsPipelineOptions ImguiPipelineOptions;

    ImguiPipelineOptions.RegisterColorAttachment(0, Outputs[0], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ATTACHMENT_LOAD_OP_LOAD);
    RenderPass = VK_CONTEXT().CreateRenderpass(LogicalDevice, ImguiPipelineOptions);

    V::SetName(LogicalDevice, RenderPass, Name);

    ImguiFramebuffers.resize(NumberOfSimultaneousSubmits);

    for(int i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        ImguiFramebuffers[i] = VK_CONTEXT().CreateFramebuffer(Width, Height, {Outputs[i]}, RenderPass, Name);
    }

    std::map<VkDescriptorType, uint32_t> PoolSizes =
            {
                    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
            };

    DescriptorPool = VK_CONTEXT().CreateFreeableDescriptorPool(PoolSizes, LogicalDevice, "V_ImGuiDescriptorPool");

    auto CheckResultFunction = [](VkResult Err)
    {
        if (Err == 0)
            return;
        std::cout << "[vulkan] Error: VkResult = " << Err << std::endl;
        if (Err < 0)
            abort();
    };

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& IO = ImGui::GetIO();
    IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(WINDOW(), true);
    ImGui_ImplVulkan_InitInfo InitInfo{};
    InitInfo.Instance = VK_CONTEXT().GetInstance();
    InitInfo.PhysicalDevice = VK_CONTEXT().GetPhysicalDevice();
    InitInfo.Device = LogicalDevice;
    InitInfo.QueueFamily = VK_CONTEXT().GetGraphicsQueueIndex();
    InitInfo.Queue = VK_CONTEXT().GetGraphicsQueue();
    InitInfo.DescriptorPool = DescriptorPool;
    InitInfo.RenderPass = RenderPass;
    InitInfo.MinImageCount = NumberOfSimultaneousSubmits;
    InitInfo.ImageCount = NumberOfSimultaneousSubmits;
    InitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    InitInfo.CheckVkResultFn = CheckResultFunction;
    ImGui_ImplVulkan_Init(&InitInfo);
}

void FImguiTask::UpdateDescriptorSets()
{
}

void FImguiTask::RecordCommands()
{
    CommandBuffers.resize(NumberOfSimultaneousSubmits);
}

VkSemaphore FImguiTask::Submit(VkQueue Queue, VkSemaphore WaitSemaphore, VkPipelineStageFlags PipelineStageFlags, VkFence WaitFence, VkFence SignalFence, uint32_t IterationIndex)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    auto StringToColor = [](const std::string& String)
    {
        static std::hash<std::string> HashingFunction;
        uint32_t Color = (uint32_t)HashingFunction(String);
        Color |= 255 << 24;
        return Color;
    };

    static ImGuiUtils::ProfilersWindow ProfilerData;
    if (!bFirstCall)
    {
        std::vector<std::string> Names;
        std::vector<float> Timings;
        float DeltaTime = 0;
        TIMING_MANAGER()->GetAllTimings(Names, Timings, DeltaTime, PreviousIterationIndex);
        std::vector<legit::ProfilerTask> GPUTasks(Timings.size());

        float StartTime = 0.f;
        float EndTime = 0.f;

        for (int i = 0; i < Names.size(); ++i)
        {
            EndTime = StartTime + Timings[i];
            GPUTasks[i] = {StartTime, EndTime, Names[i], StringToColor(Names[i])};
            StartTime = EndTime;
        }

        ProfilerData.gpuGraph.LoadFrameData(GPUTasks.data(), GPUTasks.size());

        ProfilerData.Render();
    }

    CommandBuffers[IterationIndex] = VK_CONTEXT().CommandBufferManager->BeginSingleTimeCommand();

    V::SetName(LogicalDevice, CommandBuffers[IterationIndex], Name);

    VK_CONTEXT().CommandBufferManager->RecordCommand([&, this](VkCommandBuffer)
    {
        {
            TIMING_MANAGER()->TimestampStart(Name, CommandBuffers[IterationIndex], IterationIndex);

            VkRenderPassBeginInfo RenderPassBeginInfo{};
            RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            RenderPassBeginInfo.renderPass = RenderPass;
            RenderPassBeginInfo.framebuffer = ImguiFramebuffers[IterationIndex % NumberOfSimultaneousSubmits];
            /// TODO: find a better way to pass extent
            RenderPassBeginInfo.renderArea.extent = {uint32_t(Width), uint32_t(Height)};
            RenderPassBeginInfo.clearValueCount = 0;
            vkCmdBeginRenderPass(CommandBuffers[IterationIndex], &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            TIMING_MANAGER()->TimestampEnd(Name, CommandBuffers[IterationIndex], IterationIndex);
        }

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), CommandBuffers[IterationIndex]);

        vkCmdEndRenderPass(CommandBuffers[IterationIndex]);
        vkEndCommandBuffer(CommandBuffers[IterationIndex]);
    });

    if (bFirstCall)
    {
        bFirstCall = false;
    }

    PreviousIterationIndex = IterationIndex;

    return FExecutableTask::Submit(Queue, WaitSemaphore, PipelineStageFlags, WaitFence, SignalFence, IterationIndex);
}