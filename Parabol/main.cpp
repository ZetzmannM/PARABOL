#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include <string>

#include "ChannelPrintStream.h"
#include "Surface.h"
#include "Timer.h"
#include "Env.h"
#include "inc_setting.h"

#include <thread>

#ifdef ENV_WINDOWS
#include <Windows.h>
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
#endif

#include "VkUtils.h"

#ifdef ENV_WINDOWS
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
#else
int main() {
#endif
	{
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

		TimeHandler handl = TimeHandler(true, 60, 20);
		
		Surface::Window w = Surface::Window({ 100,300,100,300 }, NULL, {}, "TestWindow");
		VK::VulkanInstanceInfo inf;
		inf.window = &w;
		inf.appInfo.name = "Test App";

		VK::VulkanInterface::vkInit(inf);

		VK::QueueSet queues{ &VK::VulkanInterface::instance(), {true, false, false, false} };
		VK::CommandPoolSet cmdPools{ &VK::VulkanInterface::instance(), {true, false, false, false} };	
		VK::RenderSubmitSynchronizationSet set[2];
		set[0] = VK::VulkanInterface::instance().swapChain->syncPrimitiveForFrame(0);
		set[1] = VK::VulkanInterface::instance().swapChain->syncPrimitiveForFrame(1);
		VkCommandBuffer buffer;

		VkCommandBufferAllocateInfo allocInf = {};
		allocInf.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInf.pNext = nullptr;
		allocInf.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInf.commandBufferCount = 1;
		allocInf.commandPool = cmdPools.graphics;
		
		vkAllocateCommandBuffers(VK::VulkanInterface::instance().dev->hndl, &allocInf, &buffer);

		VkCommandBufferBeginInfo begInf = {};
		begInf.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begInf.pNext = nullptr;
		begInf.pInheritanceInfo = nullptr;
		begInf.flags = 0;

		vkBeginCommandBuffer(buffer, &begInf);
		vkEndCommandBuffer(buffer);

		uint32 frame = 0;

		//++++ TEST OVER ++++ 
		while (!glfwWindowShouldClose(w.getGLFWHandle())) {
			handl.start();
			// Imagine this is a second thread *whistle*

			frame = VK::VulkanInterface::nextFrame();

			glfwSwapBuffers(w.getGLFWHandle());

			set[frame].waitForRender->wait();
			set[frame].waitForRender->reset();

			PRINT_DEBUG(std::to_string(frame));
			
			VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			
			VkSubmitInfo submitInf = {};
			submitInf.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInf.pNext = nullptr;
			submitInf.pCommandBuffers = &buffer;
			submitInf.commandBufferCount = 1;
			submitInf.pSignalSemaphores = set[frame].signal;
			submitInf.signalSemaphoreCount = 1;
			submitInf.waitSemaphoreCount = 1;
			submitInf.pWaitSemaphores = set[frame].waitFor;
			submitInf.pWaitDstStageMask = &stageMask;

			vkQueueSubmit(queues.graphics, 1, &submitInf, *(set[frame].signalFence));

			//
			handl.stop();
			handl.vsync();


			glfwPollEvents();
		}
	}
	glfwTerminate();
	VK::VulkanInterface::destroyInstance();
	
	return 0;
}
