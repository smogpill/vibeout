// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once

#define IMAGE_BARRIER() \
	VkImageMemoryBarrier \
	{ \
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, \
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, \
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED \
	}

#define QUEUE_IMAGE_BARRIER(cmd_buf_, barrier_) \
	vkCmdPipelineBarrier(cmd_buf_, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, \
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier_)

#define BUFFER_BARRIER() \
	VkBufferMemoryBarrier \
	{ \
		.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER, \
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, \
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, \
	}

#define QUEUE_BUFFER_BARRIER(cmd_buf_, barrier_) \
	vkCmdPipelineBarrier(cmd_buf_, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, \
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 1, &barrier_, 0, nullptr)

#define CREATE_PIPELINE_LAYOUT(dev, layout, ...) \
	do { \
		VkPipelineLayoutCreateInfo pipeline_layout_info = { \
			.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, \
			__VA_ARGS__ \
		}; \
		VO_TRY(vkCreatePipelineLayout(dev, &pipeline_layout_info, nullptr, layout) == VK_SUCCESS); \
	} while(0) \

#define SHADER_STAGE(_module, _stage) \
	{ \
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, \
		.stage = _stage, \
		.module = _renderer.GetShaders()->GetShaderModule(ShaderModule::_module), \
		.pName = "main" \
	}

#define SHADER_STAGE_SPEC(_module, _stage, _spec) \
	{ \
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, \
		.stage = _stage, \
		.module = _renderer.GetShaders()->GetShaderModule(ShaderModule::_module), \
		.pName = "main", \
		.pSpecializationInfo = _spec, \
	}

void QueueImageBarrier(VkCommandBuffer commands, VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask);

/*
class _ScopeVkCommandsLabel
{
public:
	_ScopeVkCommandsLabel(VkCommandBuffer cmds, const char* label);
	~_ScopeVkCommandsLabel();
private:
	VkCommandBuffer _commandBuffer = nullptr;
};

#ifdef VO_DEBUG
#define VO_SCOPE_VK_CMD_LABEL(_commandBuffer_, _label_) _ScopeVkCommandsLabel coCONCAT(_scopeCommandBuffer_, __COUNTER__)(_commandBuffer_, _label_)
#else
#define VO_SCOPE_VK_CMD_LABEL(_commandBuffer_, _label_)
#endif
*/
#define VO_SCOPE_VK_CMD_LABEL(_commandBuffer_, _label_)
