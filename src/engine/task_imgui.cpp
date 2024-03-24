#include "task_imgui.h"
#include "vk_context.h"
#include "vk_debug.h"

#include "window_manager.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "ImGuiProfilerRenderer.h"

#include <iostream>
#include <iomanip>

FImguiTask::FImguiTask(uint32_t WidthIn, uint32_t HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, Context, NumberOfSimultaneousSubmits, LogicalDevice)
{
    Name = "Imgui pipeline";
}

FImguiTask::~FImguiTask()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    FreeSyncObjects();
}

void FImguiTask::Init()
{
    Context->TimingManager->RegisterTiming(Name, NumberOfSimultaneousSubmits);

    auto& Context = GetContext();
    FGraphicsPipelineOptions ImguiPipelineOptions;

    ImguiPipelineOptions.RegisterColorAttachment(0, Outputs[0], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ATTACHMENT_LOAD_OP_LOAD);
    RenderPass = Context.CreateRenderpass(LogicalDevice, ImguiPipelineOptions);

    V::SetName(LogicalDevice, RenderPass, "V_ImGuiRenderPass");

    ImguiFramebuffers.resize(NumberOfSimultaneousSubmits);

    for(int i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        ImguiFramebuffers[i] = Context.CreateFramebuffer(Width, Height, {Outputs[i]}, RenderPass, "V_Imgui_fb_" + std::to_string(i));
    }

    std::map<VkDescriptorType, uint32_t> PoolSizes =
            {
                    { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                    { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
            };

    DescriptorPool = Context.CreateDescriptorPool(PoolSizes, LogicalDevice, "V_ImGuiDescriptorPool");

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

    ImGui_ImplGlfw_InitForVulkan(WINDOW_MANAGER()->GetWindow(), false);
    ImGui_ImplVulkan_InitInfo InitInfo{};
    InitInfo.Instance = Context.GetInstance();
    InitInfo.PhysicalDevice = Context.GetPhysicalDevice();
    InitInfo.Device = LogicalDevice;
    InitInfo.QueueFamily = Context.GetGraphicsQueueIndex();
    InitInfo.Queue = Context.GetGraphicsQueue();
    InitInfo.DescriptorPool = DescriptorPool;
    InitInfo.MinImageCount = NumberOfSimultaneousSubmits;
    InitInfo.ImageCount = NumberOfSimultaneousSubmits;
    InitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    InitInfo.CheckVkResultFn = CheckResultFunction;
    ImGui_ImplVulkan_Init(&InitInfo, RenderPass);
    CreateSyncObjects();
}

void FImguiTask::UpdateDescriptorSets()
{
}

void FImguiTask::RecordCommands()
{
    CommandBuffers.resize(NumberOfSimultaneousSubmits);
}

void FImguiTask::Cleanup()
{
    Inputs.clear();
    Outputs.clear();

    for (auto Framebuffer : ImguiFramebuffers)
    {
        vkDestroyFramebuffer(LogicalDevice, Framebuffer, nullptr);
    }

    for (auto& CommandBuffer : CommandBuffers)
    {
        Context->CommandBufferManager->FreeCommandBuffer(CommandBuffer);
    }

    vkDestroyRenderPass(LogicalDevice, RenderPass, nullptr);

    Context->DescriptorSetManager->Reset(Name);

    vkDestroyDescriptorPool(LogicalDevice, DescriptorPool, nullptr);
}

VkSemaphore FImguiTask::Submit(VkQueue Queue, VkSemaphore WaitSemaphore, VkFence WaitFence, VkFence SignalFence, int IterationIndex)
{
    static bool bFirstCall = true;

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Info", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar);
    ImGui::SetWindowPos({0.f, 0.f});
    ImGui::SetWindowSize({600, 300});
    ImGui::TextColored({0.f, 1.f, 0.f, 1.f}, "Test text");
    ImGui::End();

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
        Context->TimingManager->GetAllTimings(Names, Timings, DeltaTime, PreviousIterationIndex);
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

    auto& Context = GetContext();

    CommandBuffers[IterationIndex] = Context.CommandBufferManager->BeginSingleTimeCommand();

    V::SetName(LogicalDevice, CommandBuffers[IterationIndex], "V_ImguiCommandBuffer" + std::to_string(IterationIndex % NumberOfSimultaneousSubmits));

    Context.CommandBufferManager->RecordCommand([&, this](VkCommandBuffer)
    {
        {
            Context.TimingManager->TimestampStart(Name, CommandBuffers[IterationIndex], IterationIndex);

            VkRenderPassBeginInfo RenderPassBeginInfo{};
            RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            RenderPassBeginInfo.renderPass = RenderPass;
            RenderPassBeginInfo.framebuffer = ImguiFramebuffers[IterationIndex % NumberOfSimultaneousSubmits];
            /// TODO: find a better way to pass extent
            RenderPassBeginInfo.renderArea.extent = {uint32_t(Width), uint32_t(Height)};
            RenderPassBeginInfo.clearValueCount = 0;
            vkCmdBeginRenderPass(CommandBuffers[IterationIndex], &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            Context.TimingManager->TimestampEnd(Name, CommandBuffers[IterationIndex], IterationIndex);
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

    return FExecutableTask::Submit(Queue, WaitSemaphore, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, WaitFence, SignalFence, IterationIndex);
}