// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "Utils.h"

void QueueImageBarrier(VkCommandBuffer commands, VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask)
{
	VkImageSubresourceRange subresource_range =
	{
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1
	};

	VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange = subresource_range;
	barrier.srcAccessMask = srcAccessMask;
	barrier.dstAccessMask = dstAccessMask;
	barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	vkCmdPipelineBarrier(commands, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

/*
_ScopeVkCommandsLabel::_ScopeVkCommandsLabel(VkCommandBuffer cmds, const char* label)
    : _commandBuffer(cmds)
{
    RenderContext* context = RenderContext::instance;
    if (context)
        context->BeginCommandsLabel(cmds, label);
}

_ScopeVkCommandsLabel::~_ScopeVkCommandsLabel()
{
    RenderContext* context = RenderContext::instance;
    if (context)
        context->EndCommandsLabel(_commandBuffer);
}
*/