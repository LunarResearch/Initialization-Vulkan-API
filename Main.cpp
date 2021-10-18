#include"vkApi.h"


#ifdef _UNICODE
#define tWinMain wWinMain
#else
#define tWinMain WinMain
#endif // _UNICODE


int APIENTRY tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PTCHAR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	vkApi vk;
	{
		VkCommandPool CommandPool = VK_NULL_HANDLE;
		VkCommandPoolCreateInfo CommandPoolCreateInfo{};
		CommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		CommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		CommandPoolCreateInfo.queueFamilyIndex = vk.QueueFamilyIndex;
		CommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		vkCreateCommandPool(vk.Device, &CommandPoolCreateInfo, nullptr, &CommandPool);

		VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;
		VkCommandBufferAllocateInfo CommandBufferAllocateInfo{};
		CommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		CommandBufferAllocateInfo.commandPool = CommandPool;
		CommandBufferAllocateInfo.commandBufferCount = 1;
		CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		vkAllocateCommandBuffers(vk.Device, &CommandBufferAllocateInfo, &CommandBuffer);

		VkSemaphore Semaphore = VK_NULL_HANDLE;
		VkSemaphoreCreateInfo SemaphoreCreateInfo{};
		SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		vkCreateSemaphore(vk.Device, &SemaphoreCreateInfo, nullptr, &Semaphore);

		vk.BeginRender();

		VkCommandBufferBeginInfo CommandBufferBeginInfo{};
		CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		CommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo);

		VkRect2D RenderArea{};
		RenderArea.extent = vk.SurfaceCapabilities.currentExtent;

		std::array<VkClearValue, 2> ClearValues{};
		ClearValues[1].color.float32[0] = 0.4f;
		ClearValues[1].color.float32[1] = 0.6f;
		ClearValues[1].color.float32[2] = 0.9f;
		ClearValues[1].color.float32[3] = 1.0f;

		VkRenderPassBeginInfo RenderPassBeginInfo{};
		RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		RenderPassBeginInfo.renderPass = vk.RenderPass;
		RenderPassBeginInfo.framebuffer = vk.Framebuffer[vk.ActiveSwapchainImageId];
		RenderPassBeginInfo.renderArea = RenderArea;
		RenderPassBeginInfo.clearValueCount = static_cast<uint32_t>(ClearValues.size());
		RenderPassBeginInfo.pClearValues = ClearValues.data();
		vkCmdBeginRenderPass(CommandBuffer, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdEndRenderPass(CommandBuffer);

		vkEndCommandBuffer(CommandBuffer);

		VkSubmitInfo SubmitInfo{};
		SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		SubmitInfo.commandBufferCount = 1;
		SubmitInfo.pCommandBuffers = &CommandBuffer;
		SubmitInfo.signalSemaphoreCount = 1;
		SubmitInfo.pSignalSemaphores = &Semaphore;
		vkQueueSubmit(vk.Queue, 1, &SubmitInfo, VK_NULL_HANDLE);

		vk.EndRender({ Semaphore });

		vkQueueWaitIdle(vk.Queue);

		vkDestroySemaphore(vk.Device, Semaphore, nullptr);
		vkDestroyCommandPool(vk.Device, CommandPool, nullptr);
	}
	UpdateWindow(vk.hWnd);

	MSG msg{};
	while (msg.message != WM_QUIT)
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

	return static_cast<int32_t>(msg.wParam);
}
