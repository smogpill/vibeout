// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Shared/Base.h"
class Shaders; class Textures; class Buffers; class VertexBuffer; class PathTracer;
class Denoiser; class Bloom; class ToneMapping; class Draw; class Game; struct GlobalUniformBuffer;

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
	Renderer(SDL_Window& window, Game& game, bool& result);
	~Renderer();

	void Render();

	VkDevice GetDevice() const { return _device; }
	template <class T>
	void SetObjectName(const T& object, const char* name) { SetObjectName((uint64)object, GetVkObjectType<T>::_value, name); }
	void SetObjectName(uint64 object, VkObjectType objectType, const char* name);
	bool GetMemoryType(uint32 memReqTypeBits, VkMemoryPropertyFlags memProps, uint32& outMemType) const;
	VkDeviceSize GetAvailableVideoMemory() const;
	bool AllocateGPUMemory(VkMemoryRequirements memReq, VkDeviceMemory* memory);

private:
	friend class Textures;
	friend class Buffers;
	friend class PathTracer;
	friend class Denoiser;
	friend class Bloom;
	friend class ToneMapping;
	friend class VertexBuffer;
	friend class Draw;

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
		std::vector<VkCommandBuffer> _commandBuffers[maxFramesInFlight];
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
	bool InitSwapChain();
	void ShutdownSwapChain();
	bool InitPipelines();
	void ShutdownPipelines();

	bool Recreate();

	void UpdateScreenImagesSize();
	VkExtent2D GetScreenImageExtent() const;
	VkExtent2D GetRenderExtent() const;
	void EvaluateAASettings();

	bool BeginRender();
	bool RenderContent();
	void EndRender();

	bool UpdateUBO();

	VkCommandBuffer BeginCommandBuffer(CommandBufferGroup& group);
	bool SubmitCommandBuffer(VkCommandBuffer cmd_buf, VkQueue queue, int wait_semaphore_count, VkSemaphore* wait_semaphores,
		VkPipelineStageFlags* wait_stages, int signal_semaphore_count, VkSemaphore* signal_semaphores, VkFence fence);
	bool SubmitCommandBufferSimple(VkCommandBuffer cmd_buf, VkQueue queue);
	void WaitIdle(VkQueue queue, CommandBufferGroup& group);
	void ResetCommandBuffers(CommandBufferGroup& group);

	void BeginCommandsLabel(VkCommandBuffer commands, const char* name);
	void EndCommandsLabel(VkCommandBuffer commands);
	void InsertCommandsLabel(VkCommandBuffer commands, const char* name);
	void BeginQueueLabel(VkQueue queue, const char* name);
	void EndQueueLabel(VkQueue queue);
	void InsertQueueLabel(VkQueue queue, const char* label);
	std::pair<uint32, uint32> GetSize() const;

	// Context
	//----------------------------
	SDL_Window& _window;
	Game& _game;

	// State
	//----------------------------
	bool _running = false;

	// Options
	//----------------------------
	bool _antiAliasing = false;
	bool _wantHDR = false;
	bool _wantVSYNC = false;
	bool _wantDenoising = false;
	bool _wantBloom = false;
	bool _wantToneMapping = true;
	uint _nbBounceRays = 0;

	// Graphics instance
	//----------------------------
	VkInstance _instance = nullptr;
	VkSurfaceKHR _surface = nullptr;
	VkPhysicalDevice _physicalDevice = nullptr;
	VkDevice _device = nullptr;
	uint32 _graphicsQueueFamilyIndex = uint32(-1);
	VkQueue _graphicsQueue = nullptr;
	VmaAllocator _vmaAllocator = nullptr;
	CommandBufferGroup _graphicsCommandBuffers;
	VkPhysicalDeviceMemoryProperties _memProperties = {};
	PFN_vkCmdBeginDebugUtilsLabelEXT _vkCmdBeginDebugUtilsLabelEXT = nullptr;
	PFN_vkCmdEndDebugUtilsLabelEXT _vkCmdEndDebugUtilsLabelEXT = nullptr;
	PFN_vkCmdInsertDebugUtilsLabelEXT _vkCmdInsertDebugUtilsLabelEXT = nullptr;
	PFN_vkQueueBeginDebugUtilsLabelEXT _vkQueueBeginDebugUtilsLabelEXT = nullptr;
	PFN_vkQueueEndDebugUtilsLabelEXT _vkQueueEndDebugUtilsLabelEXT = nullptr;
	PFN_vkQueueInsertDebugUtilsLabelEXT _vkQueueInsertDebugUtilsLabelEXT = nullptr;
	PFN_vkSetDebugUtilsObjectNameEXT _vkSetDebugUtilsObjectNameEXT = nullptr;
	PFN_vkSetDebugUtilsObjectTagEXT _vkSetDebugUtilsObjectTagEXT = nullptr;
	bool _surfaceIsHDR = false;
	bool _surfaceIsVSYNC = false;

	// Swap chain
	//----------------------------
	VkSwapchainKHR _swapchain = nullptr;
	uint _currentSwapChainImageIdx = 0;
	uint _waitForIdleFrames = 0;
	std::vector<VkImage> _swapChainImages;
	std::vector<VkImageView> _swapChainImageViews;
	VkSurfaceFormatKHR _surfaceFormat = {};
	VkPresentModeKHR _presentMode = VK_PRESENT_MODE_MAX_ENUM_KHR;
	VkExtent2D _extentScreenImages = {};
	VkExtent2D _extentRender = {};
	VkExtent2D _extentRenderPrev = {};
	VkExtent2D _extentUnscaled = {};
	VkExtent2D _extentTAAImages = {};
	VkExtent2D _extentTAAOutput = {};

	// Data
	//----------------------------
	GlobalUniformBuffer* _ubo = nullptr;

	// Frame
	//----------------------------
	uint32 _currentFrameInFlight = 0;
	uint32 _nbAccumulatedFrames = 0;
	uint32 _frameCounter = 0;
	SemaphoreGroup _semaphoreGroups[maxFramesInFlight];
	VkFence _fencesFrameSync[maxFramesInFlight] = {};
	bool _frameReady = false;
	bool _temporalFrameIsValid = true;

	// Managers
	//----------------------------
	Shaders* _shaders = nullptr;
	Textures* _textures = nullptr;
	Buffers* _buffers = nullptr;
	VertexBuffer* _vertexBuffer = nullptr;
	PathTracer* _pathTracer = nullptr;
	Denoiser* _denoiser = nullptr;
	Bloom* _bloom = nullptr;
	ToneMapping* _toneMapping = nullptr;
	Draw* _draw = nullptr;
};
