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

#define VK_INSTANCE VK::VulkanInterface::instance()

#include "VkUtils.h"

VkCommandBuffer buffer;

struct Render {
	wrap_ptr<VK::QueueSet> queues;
	wrap_ptr<VK::CommandPoolSet> cmdPools;
	volatile uint32 indx = 0;
	wrap_ptr<arr_ptr<VK::RenderSubmitSynchronizationPtrSet>> syncs;

	bool running = true;

	std::thread thr;

	Render()  {
	}
	
	void start() {
		queues = new VK::QueueSet{ &VK::VulkanInterface::instance(), {true, false, false, false} };
		cmdPools = new VK::CommandPoolSet{ &VK::VulkanInterface::instance(), {true, false, false, false} };

		thr = std::thread(&Render::render, this);
	}

	~Render() {
		thr.join();
	}
	
	void render() {
		while (running) {
			syncs->operator[](indx).waitBeforeRender->wait();
			PRINT_DEBUG("Render Main " + std::to_string(indx));

			VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

			VkSubmitInfo submitInf = {};
			submitInf.pNext = nullptr;
			submitInf.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInf.waitSemaphoreCount = 1;
			submitInf.pWaitSemaphores = syncs->operator[](indx).waitFor;
			submitInf.pWaitDstStageMask = waitStages;
			submitInf.signalSemaphoreCount = 1;
			submitInf.pSignalSemaphores = syncs->operator[](indx).signal;
			submitInf.commandBufferCount = 1;
			submitInf.pCommandBuffers = &buffer;

			vkQueueSubmit(this->queues->graphics, 1, &submitInf, *syncs->operator[](indx).signalFence);
			syncs->operator[](indx).signalAfterSubmit->signal();
		}
	}

};

wrap_ptr<VK::QueueSet> queues;
wrap_ptr<VK::CommandPoolSet> cmdPools;

volatile uint32 indx;
wrap_ptr<arr_ptr<VK::RenderSubmitSynchronizationPtrSet>> syncs;
void render();

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

		inf.devProf.queueProfile = VK::DeviceQueueCount(2,1,0,0);

		inf.swapChainProfile.swapchainImageSize = 2;

		inf.renderProfile.includeMainThread = true;
		inf.renderProfile.mainThreadRender = &render;

		inf.renderProfile.threadGroups = { {0}, {1} };

		Render rendr;

		inf.renderProfile.renderThreads.push_back({ &indx, &syncs });
		inf.renderProfile.renderThreads.push_back({&rendr.indx, &rendr.syncs});

		VK::VulkanInterface::vkInit(inf);

		rendr.start();

		queues = new VK::QueueSet{ &VK::VulkanInterface::instance(), {true, false, false, false} };
		cmdPools = new VK::CommandPoolSet{ &VK::VulkanInterface::instance(), {true, false, false, false} };	

		VkCommandBufferAllocateInfo allocInf = {};
		allocInf.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInf.pNext = nullptr;
		allocInf.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInf.commandBufferCount = 1;
		allocInf.commandPool = cmdPools->graphics;
		
		vkAllocateCommandBuffers(VK::VulkanInterface::instance().dev->hndl, &allocInf, &buffer);

		VkCommandBufferBeginInfo begInf = {};
		begInf.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begInf.pNext = nullptr;
		begInf.pInheritanceInfo = nullptr;
		begInf.flags = 0;

		vkBeginCommandBuffer(buffer, &begInf);
		vkEndCommandBuffer(buffer);

		//++++ TEST OVER ++++ 
		while (!glfwWindowShouldClose(w.getGLFWHandle())) {
			handl.start();
			VK::VulkanInterface::nextFrame();
			handl.stop();
			handl.vsync();

			glfwPollEvents();
		}
	
		rendr.running = false;
		VK::VulkanInterface::destroyInstance();
	}
	glfwTerminate();

	return 0;
}

void render() {	
	//Not needed on Main thread
	//set.waitBeforeRender->wait();

	PRINT_DEBUG("Render Main " + std::to_string(indx));

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInf = {};
	submitInf.pNext = nullptr;
	submitInf.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInf.waitSemaphoreCount = 1;
	submitInf.pWaitSemaphores = syncs->operator[](indx).waitFor;
	submitInf.pWaitDstStageMask = waitStages;
	submitInf.signalSemaphoreCount = 1;
	submitInf.pSignalSemaphores = syncs->operator[](indx).signal;
	submitInf.commandBufferCount = 1;
	submitInf.pCommandBuffers = &buffer;

	vkQueueSubmit(queues->graphics, 1, &submitInf, *syncs->operator[](indx).signalFence);
	
	//Not needed on Main thread
	//set.signalAfterSubmit->signal();
}