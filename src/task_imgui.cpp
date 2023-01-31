#include "task_imgui.h"

#include "vk_context.h"

#include "vk_debug.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include <iostream>

FImguiTask::FImguiTask(int WidthIn, int HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
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

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(Context.GetWindow(), true);
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

    {
        Context.CommandBufferManager->RunSingletimeCommand(ImGui_ImplVulkan_CreateFontsTexture);
    }

    CreateSyncObjects();
}

void FImguiTask::UpdateDescriptorSets()
{
}

void FImguiTask::RecordCommands()
{
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
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Info", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar);
    ImGui::SetWindowPos({0.f, 0.f});
    ImGui::TextColored({0.f, 1.f, 0.f, 1.f}, "Test text");
    ImGui::End();

    auto& Context = GetContext();

    auto CommandBuffer = Context.CommandBufferManager->BeginSingleTimeCommand();

    V::SetName(LogicalDevice, CommandBuffer, "V_ImguiCommandBuffer" + std::to_string(IterationIndex % NumberOfSimultaneousSubmits));

    Context.CommandBufferManager->RecordCommand([&, this](VkCommandBuffer)
    {
        {
            VkRenderPassBeginInfo RenderPassBeginInfo{};
            RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            RenderPassBeginInfo.renderPass = RenderPass;
            RenderPassBeginInfo.framebuffer = ImguiFramebuffers[IterationIndex % NumberOfSimultaneousSubmits];
            /// TODO: find a better way to pass extent
            RenderPassBeginInfo.renderArea.extent = {uint32_t(Width), uint32_t(Height)};
            RenderPassBeginInfo.clearValueCount = 0;
            vkCmdBeginRenderPass(CommandBuffer, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        }

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), CommandBuffer);

        vkCmdEndRenderPass(CommandBuffer);
        vkEndCommandBuffer(CommandBuffer);
    });

    VkSubmitInfo SubmitInfo{};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore ImguiWaitSemaphores[] = {WaitSemaphore};
    VkPipelineStageFlags WaitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    SubmitInfo.waitSemaphoreCount = 1;
    SubmitInfo.pWaitSemaphores = ImguiWaitSemaphores;
    SubmitInfo.pWaitDstStageMask = WaitStages;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &CommandBuffer;

    VkSemaphore ImguiSignalSemaphores[] = {SignalSemaphores[IterationIndex]};
    SubmitInfo.signalSemaphoreCount = 1;
    SubmitInfo.pSignalSemaphores = ImguiSignalSemaphores;

    if(WaitFence != VK_NULL_HANDLE)
    {
        vkWaitForFences(LogicalDevice, 1, &WaitFence, VK_TRUE, UINT64_MAX);
    }

    if (SignalFence != VK_NULL_HANDLE)
    {
        vkResetFences(LogicalDevice, 1, &SignalFence);
    }

    if (vkQueueSubmit(Context.GetGraphicsQueue(), 1, &SubmitInfo, SignalFence) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit ImGui draw command buffer!");
    }

    return SignalSemaphores[IterationIndex];
}