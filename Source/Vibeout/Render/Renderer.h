// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once

template<typename T>
struct GetVkObjectType;
template<> struct GetVkObjectType<VkBuffer> { static inline VkObjectType _value = VK_OBJECT_TYPE_BUFFER; };
template<> struct GetVkObjectType<VkImage> { static inline  VkObjectType _value = VK_OBJECT_TYPE_IMAGE; };
template<> struct GetVkObjectType<VkImageView> { static inline  VkObjectType _value = VK_OBJECT_TYPE_IMAGE_VIEW; };
template<> struct GetVkObjectType<VkCommandBuffer> { static inline  VkObjectType _value = VK_OBJECT_TYPE_COMMAND_BUFFER; };
template<> struct GetVkObjectType<VkQueue> { static inline  VkObjectType _value = VK_OBJECT_TYPE_QUEUE; };
template<> struct GetVkObjectType<VkSemaphore> { static inline  VkObjectType _value = VK_OBJECT_TYPE_SEMAPHORE; };
template<> struct GetVkObjectType<VkFence> { static inline  VkObjectType _value = VK_OBJECT_TYPE_FENCE; };
template<> struct GetVkObjectType<VkShaderModule> { static inline  VkObjectType _value = VK_OBJECT_TYPE_SHADER_MODULE; };
template<> struct GetVkObjectType<VkPipeline> { static inline  VkObjectType _value = VK_OBJECT_TYPE_PIPELINE; };
template<> struct GetVkObjectType<VkPipelineLayout> { static inline  VkObjectType _value = VK_OBJECT_TYPE_PIPELINE_LAYOUT; };
template<> struct GetVkObjectType<VkRenderPass> { static inline  VkObjectType _value = VK_OBJECT_TYPE_RENDER_PASS; };
template<> struct GetVkObjectType<VkDescriptorPool> { static inline  VkObjectType _value = VK_OBJECT_TYPE_DESCRIPTOR_POOL; };
template<> struct GetVkObjectType<VkDescriptorSet> { static inline  VkObjectType _value = VK_OBJECT_TYPE_DESCRIPTOR_SET; };
template<> struct GetVkObjectType<VkDescriptorSetLayout> { static inline  VkObjectType _value = VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT; };
template<> struct GetVkObjectType<VkFramebuffer> { static inline  VkObjectType _value = VK_OBJECT_TYPE_FRAMEBUFFER; };
template<> struct GetVkObjectType<VkSampler> { static inline  VkObjectType _value = VK_OBJECT_TYPE_SAMPLER; };

class Renderer
{
public:
	Renderer(SDL_Window& window, bool& result);
	~Renderer();

private:
	static constexpr uint s_maxFramesInFlight = 2;

	struct SemaphoreGroup
	{
		VkSemaphore _imageAvailable = nullptr;
		VkSemaphore _renderFinished = nullptr;
		VkSemaphore _transferFinished = nullptr;
		VkSemaphore _traceFinished = nullptr;
		bool _traceSignaled = false;
	};
	struct CommandBufferGroup
	{
		uint32 _usedThisFrame = 0;
		uint32 _maxNbPerFrame = 0;
		std::vector<VkCommandBuffer> _commandBuffers[s_maxFramesInFlight];
		VkCommandPool _commandPool = nullptr;
	};

	bool Init();
	bool InitInstance();
	bool InitSurface();
	bool InitPhysicalDevice();
	bool InitQueueFamilies();
	bool InitLogicalDevice();
	bool InitVMA();
	bool InitCommandPools();
	bool InitSemaphores();
	bool InitFences();

	template <class T>
	void SetObjectName(const T& object, const char* name) { SetObjectName((uint64)object, GetVkObjectType<T>::_value, name); }
	void SetObjectName(uint64 object, VkObjectType objectType, const char* name);
	void BeginCommandsLabel(VkCommandBuffer commands, const char* name);
	void EndCommandsLabel(VkCommandBuffer commands);
	void InsertCommandsLabel(VkCommandBuffer commands, const char* name);
	void BeginQueueLabel(VkQueue queue, const char* name);
	void EndQueueLabel(VkQueue queue);
	void InsertQueueLabel(VkQueue queue, const char* label);

	// Context
	//----------------------------
	SDL_Window& _window;
	VkInstance _instance = nullptr;
	VkSurfaceKHR _surface = nullptr;
	VkPhysicalDevice _physicalDevice = nullptr;
	VkDevice _device = nullptr;
	uint32 _graphicsQueueFamilyIndex = uint32(-1);
	VkQueue _graphicsQueue = nullptr;
	VmaAllocator _vmaAllocator = nullptr;
	CommandBufferGroup _graphicsCommandBuffers;
	PFN_vkCmdBeginDebugUtilsLabelEXT _vkCmdBeginDebugUtilsLabelEXT = nullptr;
	PFN_vkCmdEndDebugUtilsLabelEXT _vkCmdEndDebugUtilsLabelEXT = nullptr;
	PFN_vkCmdInsertDebugUtilsLabelEXT _vkCmdInsertDebugUtilsLabelEXT = nullptr;
	PFN_vkQueueBeginDebugUtilsLabelEXT _vkQueueBeginDebugUtilsLabelEXT = nullptr;
	PFN_vkQueueEndDebugUtilsLabelEXT _vkQueueEndDebugUtilsLabelEXT = nullptr;
	PFN_vkQueueInsertDebugUtilsLabelEXT _vkQueueInsertDebugUtilsLabelEXT = nullptr;
	PFN_vkSetDebugUtilsObjectNameEXT _vkSetDebugUtilsObjectNameEXT = nullptr;
	PFN_vkSetDebugUtilsObjectTagEXT _vkSetDebugUtilsObjectTagEXT = nullptr;

	// Swap chain
	//----------------------------
	VkSwapchainKHR _swapchain = nullptr;
	uint32 _currentSwapChainImageIdx = 0;
	std::vector<VkImage> _swapChainImages;
	std::vector<VkImageView> _swapChainImageViews;

	// Frame
	//----------------------------
	uint32 _currentFrameInFlight = 0;
	SemaphoreGroup _semaphoreGroups[s_maxFramesInFlight];
	VkFence _fencesFrameSync[s_maxFramesInFlight] = {};

	bool _surfaceIsHDR = false;
	bool _surfaceIsVSYNC = false;
};
