#pragma once


#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>
#include <vector>
#include <array>


#define DISCRETE_GPU 0
#define INTEGRATED_GPU 1


class vkApi
{
public:
	vkApi();
	~vkApi();

	void BeginRender();
	void EndRender(std::vector<VkSemaphore>);

	HWND hWnd = nullptr;
	HINSTANCE hInstance = nullptr;

	VkDevice Device = VK_NULL_HANDLE;
	VkQueue Queue = VK_NULL_HANDLE;
	VkRenderPass RenderPass = VK_NULL_HANDLE;

	VkSurfaceCapabilitiesKHR SurfaceCapabilities{};
	VkWin32SurfaceCreateInfoKHR Win32SurfaceCreateInfo{};

	std::vector<VkFramebuffer> Framebuffer;

	uint32_t QueueFamilyIndex = 0;
	uint32_t ActiveSwapchainImageId = UINT32_MAX;

private:
	void CreateInstance();
	void DestroyInstance();

	void CreateDevice();
	void DestroyDevice();

	void ConstructWindow();

	void CreateSurface();
	void DestroySurface();

	void CreateSwapchain();
	void DestroySwapchain();

	void CreateImageView();
	void DestroyImageView();

	void CreateDepthStencilImage();
	void DestroyDepthStencilImage();

	void CreateRenderPass();
	void DestroyRenderPass();

	void CreateFramebuffer();
	void DestroyFramebuffer();

	void CreateFence();
	void DestroyFence();

	VkInstance Instance = VK_NULL_HANDLE;
	VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
	VkSurfaceKHR Surface = VK_NULL_HANDLE;
	VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
	VkImage DepthStencilImage = VK_NULL_HANDLE;
	VkImageView DepthStencilImageView = VK_NULL_HANDLE;
	VkDeviceMemory DepthStencilImageMemory = VK_NULL_HANDLE;
	VkFence SwapchainImageAvailable = VK_NULL_HANDLE;

	VkSurfaceFormatKHR SurfaceFormat{};
	VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties{};

	std::vector<const char*> InstanceExtension;
	std::vector<const char*> DeviceExtension;

	std::vector<VkImage> SwapchainImage;
	std::vector<VkImageView> SwapchainImageView;

	VkFormat DepthStencilFormat = VK_FORMAT_UNDEFINED;
	bool DepthStencilAvalible = false;

	uint32_t SwapchainImageCount = 2;
};

