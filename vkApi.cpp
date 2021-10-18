#include "vkApi.h"


vkApi::vkApi()
{
	CreateInstance();
	CreateDevice();
	ConstructWindow();
	CreateSurface();
	CreateSwapchain();
	CreateImageView();
	CreateDepthStencilImage();
	CreateRenderPass();
	CreateFramebuffer();
	CreateFence();
}


vkApi::~vkApi()
{
	vkQueueWaitIdle(Queue);

	DestroyFence();
	DestroyFramebuffer();
	DestroyRenderPass();
	DestroyDepthStencilImage();
	DestroyImageView();
	DestroySwapchain();
	DestroySurface();
	DestroyDevice();
	DestroyInstance();
}


void vkApi::BeginRender()
{
	vkAcquireNextImageKHR(Device, Swapchain, UINT64_MAX, VK_NULL_HANDLE, SwapchainImageAvailable, &ActiveSwapchainImageId);
	vkWaitForFences(Device, 1, &SwapchainImageAvailable, VK_TRUE, UINT64_MAX);
	vkResetFences(Device, 1, &SwapchainImageAvailable);
	vkQueueWaitIdle(Queue);
}


void vkApi::EndRender(std::vector<VkSemaphore> WaitSemaphore)
{
	VkResult Result = VkResult::VK_RESULT_MAX_ENUM;

	VkPresentInfoKHR PresentInfo{};
	PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	PresentInfo.waitSemaphoreCount = static_cast<uint32_t>(WaitSemaphore.size());
	PresentInfo.pWaitSemaphores = WaitSemaphore.data();
	PresentInfo.swapchainCount = 1;
	PresentInfo.pSwapchains = &Swapchain;
	PresentInfo.pImageIndices = &ActiveSwapchainImageId;
	PresentInfo.pResults = &Result;

	vkQueuePresentKHR(Queue, &PresentInfo);
}


void vkApi::CreateInstance()
{
	VkApplicationInfo ApplicationInfo{};
	ApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	ApplicationInfo.apiVersion = VK_API_VERSION_1_1;
	ApplicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	ApplicationInfo.pApplicationName = "vkProjectGame";
	ApplicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	ApplicationInfo.pEngineName = "GameEngineSDK";

	VkInstanceCreateInfo InstanceCreateInfo{};
	InstanceExtension.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	InstanceExtension.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	InstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	InstanceCreateInfo.pApplicationInfo = &ApplicationInfo;
	InstanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(InstanceExtension.size());
	InstanceCreateInfo.ppEnabledExtensionNames = InstanceExtension.data();

	vkCreateInstance(&InstanceCreateInfo, nullptr, &Instance);
}


void vkApi::DestroyInstance() {
	vkDestroyInstance(Instance, nullptr);
}


void vkApi::CreateDevice()
{
	{
		uint32_t PhysicalDeviceCount = 0;
		vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, nullptr);
		std::vector<VkPhysicalDevice> PhysicalDevices(PhysicalDeviceCount);
		vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, PhysicalDevices.data());

		PhysicalDevice = PhysicalDevices[DISCRETE_GPU]; // DISCRETE_GPU or INTEGRATED_GPU
	}

	{
		uint32_t QueueFamilyPropertyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyPropertyCount, nullptr);
		std::vector<VkQueueFamilyProperties> QueueFamilyProperties(QueueFamilyPropertyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyPropertyCount, QueueFamilyProperties.data());
		for (uint32_t i = 0; i < QueueFamilyPropertyCount; ++i)
			if (QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				QueueFamilyIndex = i;
	}

	vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &PhysicalDeviceMemoryProperties);

	float QueuePriorities[] = { 1.0f };

	VkDeviceQueueCreateInfo DeviceQueueCreateInfo{};
	DeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	DeviceQueueCreateInfo.queueFamilyIndex = QueueFamilyIndex;
	DeviceQueueCreateInfo.queueCount = 1;
	DeviceQueueCreateInfo.pQueuePriorities = QueuePriorities;

	VkDeviceCreateInfo DeviceCreateInfo{};
	DeviceExtension.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	DeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	DeviceCreateInfo.queueCreateInfoCount = 1;
	DeviceCreateInfo.pQueueCreateInfos = &DeviceQueueCreateInfo;
	DeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtension.size());
	DeviceCreateInfo.ppEnabledExtensionNames = DeviceExtension.data();

	vkCreateDevice(PhysicalDevice, &DeviceCreateInfo, nullptr, &Device);

	vkGetDeviceQueue(Device, QueueFamilyIndex, 0, &Queue);
}


void vkApi::DestroyDevice() {
	vkDestroyDevice(Device, nullptr);
}


INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}


void vkApi::ConstructWindow()
{
	hInstance = GetModuleHandle(nullptr);

	WNDCLASS wc{};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = TEXT("MainWindowClass");

	RegisterClass(&wc);

	hWnd = CreateWindow(wc.lpszClassName, TEXT("grProjectGame"), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	ShowWindow(hWnd, SW_SHOW);
}


void vkApi::CreateSurface()
{
	Win32SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	Win32SurfaceCreateInfo.hinstance = hInstance;
	Win32SurfaceCreateInfo.hwnd = hWnd;

	vkCreateWin32SurfaceKHR(Instance, &Win32SurfaceCreateInfo, nullptr, &Surface);

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, Surface, &SurfaceCapabilities);

	{
		uint32_t SurfaceFormatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &SurfaceFormatCount, nullptr);
		std::vector<VkSurfaceFormatKHR> SurfaceFormats(SurfaceFormatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &SurfaceFormatCount, SurfaceFormats.data());
		if (SurfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
			SurfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
			SurfaceFormat.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		}
		else SurfaceFormat = SurfaceFormats[0];
	}
}


void vkApi::DestroySurface() {
	vkDestroySurfaceKHR(Instance, Surface, nullptr);
}


void vkApi::CreateSwapchain()
{
	if (SwapchainImageCount < SurfaceCapabilities.minImageCount + 1)
		SwapchainImageCount = SurfaceCapabilities.minImageCount + 1;
	if (SurfaceCapabilities.maxImageCount > 0)
		if (SwapchainImageCount > SurfaceCapabilities.maxImageCount)
			SwapchainImageCount = SurfaceCapabilities.maxImageCount;

	VkPresentModeKHR PresentMode = VK_PRESENT_MODE_FIFO_KHR;

	{
		uint32_t PresentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModeCount, nullptr);
		std::vector<VkPresentModeKHR> PresentModes(PresentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModeCount, PresentModes.data());
		for (auto _T : PresentModes)
			if (_T == VK_PRESENT_MODE_MAILBOX_KHR)
				PresentMode = _T;
	}

	VkSwapchainCreateInfoKHR SwapchainCreateInfo{};
	SwapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	SwapchainCreateInfo.surface = Surface;
	SwapchainCreateInfo.minImageCount = SwapchainImageCount;
	SwapchainCreateInfo.imageFormat = SurfaceFormat.format;
	SwapchainCreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
	SwapchainCreateInfo.imageExtent.width = SurfaceCapabilities.currentExtent.width;
	SwapchainCreateInfo.imageExtent.height = SurfaceCapabilities.currentExtent.height;
	SwapchainCreateInfo.imageArrayLayers = 1;
	SwapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	SwapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	SwapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	SwapchainCreateInfo.presentMode = PresentMode;
	SwapchainCreateInfo.clipped = VK_TRUE;

	vkCreateSwapchainKHR(Device, &SwapchainCreateInfo, nullptr, &Swapchain);

	vkGetSwapchainImagesKHR(Device, Swapchain, &SwapchainImageCount, nullptr);
}


void vkApi::DestroySwapchain() {
	vkDestroySwapchainKHR(Device, Swapchain, nullptr);
}


void vkApi::CreateImageView()
{
	SwapchainImage.resize(SwapchainImageCount);
	SwapchainImageView.resize(SwapchainImageCount);

	vkGetSwapchainImagesKHR(Device, Swapchain, &SwapchainImageCount, SwapchainImage.data());

	for (uint32_t i = 0; i < SwapchainImageCount; ++i) {
		VkImageViewCreateInfo ImageViewCreateInfo{};
		ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ImageViewCreateInfo.image = SwapchainImage[i];
		ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ImageViewCreateInfo.format = SurfaceFormat.format;
		ImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		ImageViewCreateInfo.subresourceRange.levelCount = 1;
		ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		ImageViewCreateInfo.subresourceRange.layerCount = 1;

		vkCreateImageView(Device, &ImageViewCreateInfo, nullptr, &SwapchainImageView[i]);
	}
}


void vkApi::DestroyImageView() {
	for (auto _T : SwapchainImageView)
		vkDestroyImageView(Device, _T, nullptr);
}


void vkApi::CreateDepthStencilImage()
{
	{
		std::vector<VkFormat> format{
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D16_UNORM
		};

		for (auto _T : format) {
			VkFormatProperties FormatProperties{};
			vkGetPhysicalDeviceFormatProperties(PhysicalDevice, _T, &FormatProperties);
			if (FormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
				DepthStencilFormat = _T;
		}
		if ((DepthStencilFormat == VK_FORMAT_D32_SFLOAT_S8_UINT) ||
			(DepthStencilFormat == VK_FORMAT_D24_UNORM_S8_UINT) ||
			(DepthStencilFormat == VK_FORMAT_D16_UNORM_S8_UINT) ||
			(DepthStencilFormat == VK_FORMAT_S8_UINT))
			DepthStencilAvalible = true;
	}

	VkImageCreateInfo ImageCreateInfo{};
	ImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	ImageCreateInfo.format = DepthStencilFormat;
	ImageCreateInfo.extent.width = SurfaceCapabilities.currentExtent.width;
	ImageCreateInfo.extent.height = SurfaceCapabilities.currentExtent.height;
	ImageCreateInfo.extent.depth = 1;
	ImageCreateInfo.mipLevels = 1;
	ImageCreateInfo.arrayLayers = 1;
	ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	ImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	ImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	vkCreateImage(Device, &ImageCreateInfo, nullptr, &DepthStencilImage);

	VkMemoryRequirements MemoryRequirements{};
	vkGetImageMemoryRequirements(Device, DepthStencilImage, &MemoryRequirements);

	uint32_t MemoryIndex = UINT32_MAX;
	auto MemoryPropertyFlagBits = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	for (uint32_t i = 0; i < PhysicalDeviceMemoryProperties.memoryTypeCount; ++i)
		if (MemoryRequirements.memoryTypeBits & (1 << i))
			if ((PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & MemoryPropertyFlagBits) == MemoryPropertyFlagBits) {
				MemoryIndex = i;
				break;
			}

	VkMemoryAllocateInfo MemoryAllocateInfo{};
	MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	MemoryAllocateInfo.allocationSize = MemoryRequirements.size;
	MemoryAllocateInfo.memoryTypeIndex = MemoryIndex;

	vkAllocateMemory(Device, &MemoryAllocateInfo, nullptr, &DepthStencilImageMemory);
	vkBindImageMemory(Device, DepthStencilImage, DepthStencilImageMemory, 0);

	VkImageViewCreateInfo ImageViewCreateInfo{};
	ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ImageViewCreateInfo.image = DepthStencilImage;
	ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	ImageViewCreateInfo.format = DepthStencilFormat;
	ImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | (DepthStencilAvalible ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
	ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	ImageViewCreateInfo.subresourceRange.levelCount = 1;
	ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	ImageViewCreateInfo.subresourceRange.layerCount = 1;

	vkCreateImageView(Device, &ImageViewCreateInfo, nullptr, &DepthStencilImageView);
}


void vkApi::DestroyDepthStencilImage() {
	vkDestroyImageView(Device, DepthStencilImageView, nullptr);
	vkFreeMemory(Device, DepthStencilImageMemory, nullptr);
	vkDestroyImage(Device, DepthStencilImage, nullptr);
}


void vkApi::CreateRenderPass()
{
	std::array<VkAttachmentDescription, 2> Attachments{};
	{
		Attachments[0].format = DepthStencilFormat;
		Attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		Attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		Attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		Attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		Attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		Attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		Attachments[0].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}
	{
		Attachments[1].format = SurfaceFormat.format;
		Attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		Attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		Attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		Attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		Attachments[1].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}

	VkAttachmentReference DepthStencilAttachment{};
	DepthStencilAttachment.attachment = 0;
	DepthStencilAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	std::array<VkAttachmentReference, 1> InputAttachments{};
	InputAttachments[0].attachment = 1;
	InputAttachments[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	std::array<VkSubpassDescription, 1> Subpasses{};
	Subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	Subpasses[0].pDepthStencilAttachment = &DepthStencilAttachment;
	Subpasses[0].colorAttachmentCount = static_cast<uint32_t>(InputAttachments.size());
	Subpasses[0].pColorAttachments = InputAttachments.data();

	VkRenderPassCreateInfo RenderPassCreateInfo{};
	RenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	RenderPassCreateInfo.attachmentCount = static_cast<uint32_t>(Attachments.size());
	RenderPassCreateInfo.pAttachments = Attachments.data();
	RenderPassCreateInfo.subpassCount = static_cast<uint32_t>(Subpasses.size());
	RenderPassCreateInfo.pSubpasses = Subpasses.data();;

	vkCreateRenderPass(Device, &RenderPassCreateInfo, nullptr, &RenderPass);
}


void vkApi::DestroyRenderPass() {
	vkDestroyRenderPass(Device, RenderPass, nullptr);
}


void vkApi::CreateFramebuffer()
{
	Framebuffer.resize(SwapchainImageCount);

	for (uint32_t i = 0; i < SwapchainImageCount; ++i)
	{
		std::array<VkImageView, 2> Attachments{};
		Attachments[0] = DepthStencilImageView;
		Attachments[1] = SwapchainImageView[i];

		VkFramebufferCreateInfo FramebufferCreateInfo{};
		FramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		FramebufferCreateInfo.renderPass = RenderPass;
		FramebufferCreateInfo.attachmentCount = static_cast<uint32_t>(Attachments.size());
		FramebufferCreateInfo.pAttachments = Attachments.data();
		FramebufferCreateInfo.width = SurfaceCapabilities.currentExtent.width;
		FramebufferCreateInfo.height = SurfaceCapabilities.currentExtent.height;
		FramebufferCreateInfo.layers = 1;

		vkCreateFramebuffer(Device, &FramebufferCreateInfo, nullptr, &Framebuffer[i]);
	}
}


void vkApi::DestroyFramebuffer() {
	for (auto _T : Framebuffer)
		vkDestroyFramebuffer(Device, _T, nullptr);
}


void vkApi::CreateFence()
{
	VkFenceCreateInfo FenceCreateInfo{};
	FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	vkCreateFence(Device, &FenceCreateInfo, nullptr, &SwapchainImageAvailable);
}


void vkApi::DestroyFence() {
	vkDestroyFence(Device, SwapchainImageAvailable, nullptr);
}
