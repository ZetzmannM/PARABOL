#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <map>
#include <functional>
#include <atomic>

#include "Sync.h"
#include "inttypes.h"
#include "ptr.h"
#include "StringUtils.h"

#include "inc_setting.h"
#include "Surface.h"
#include "meta_tags.h"

#include "GLFW/glfw3.h"

namespace VK {
	enum struct vk_queue_type {
		graphics = 0,
		transfer = 2,
		compute = 1,
		present = 3
	};

	struct Catalog {
	private:
		std::map<std::string, bool> values;

	public:
		Catalog(){}
		Catalog(VkExtensionProperties* prop, int size);
		Catalog(VkLayerProperties* prop, int size);

		bool isAvailable(const std::string& ref);

		bool isEnabled(const std::string& ref);

		bool addIfAvailable(const std::string& ref);
		Util::NonCpyStringContainer toContainer();
	};
	struct PhyDevQueueFamilies {
		struct VKQueueIndex {
			bool needed = true;
			bool set = false;
			uint32 index = 0xFFFFFFFF;
		};

		VKQueueIndex graphics;
		VKQueueIndex transfer;
		VKQueueIndex compute;
		VKQueueIndex present;

		bool isComplete();
	};
	struct SurfaceProperties {
		VkSurfaceCapabilitiesKHR capabilities = { 0u, 0u, {0u,0u}, {0u,0u}, {0u,0u}, 0u, 0u, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, 0u, 0u };
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct DeviceQueueCount {
		uint64 totalQueueCount = 0;
		/// @brief Pairs of the amount of queues and their priority
		std::map<vk_queue_type, std::pair<uint32, float>> queueFamilies;

		DeviceQueueCount(uint16 g = 0, uint16 p = 0, uint16 t = 0, uint16 c = 0);
	};

	/// @brief Saves how many queues of which type and priority are available/requested from a Device
	struct DeviceQueueProfile {
		DeviceQueueCount queueCount;

	private:
		std::atomic_int graphicsQueueCounter = 0;
		std::atomic_int presentQueueCounter = 0;
		std::atomic_int transferQueueCounter = 0;
		std::atomic_int computeQueueCounter = 0;		
	public:
		
		DeviceQueueProfile(const DeviceQueueCount& ref);
		
		/// @brief Tries to reserve a queue for a queueSet from the set of queues given to a device upon creation (The maximum amount is specified in the DevProfile of the VulkanInstanceInfo)
		/// @param type type of desired queue
		/// @return index of queue, or -1 if no more queues are available
		thread_safe int reserveQueue(vk_queue_type type);
	};

	/// @brief Saves whi vch Queues are required/requested for a QueueSet
	struct QueueProfile {
		bool graphics = true;
		bool present = false;
		bool transfer = true;
		bool compute = false;
	};

	/// @brief The threads responsible for rendering HAVE TO use these synchronization primitives to ensure synchronization with the SwapChain
	/// These primitives should be cited for the last vkQueueSubmit as waitforSemaphore, signalSemaphore and the signalFence as an argument
	/// the waitBeforeRender Barrier should be waited for before even beginning rendering and the signalAfterSubmit should be called right after the vkQueueSubmit that signals the renderFinishedSemaphore
	struct RenderSubmitSynchronizationPtrSet {
		VkSemaphore* waitFor = nullptr;
		VkSemaphore* signal = nullptr;
		VkFence* signalFence = nullptr;

		//@brief Ignored on MainThread
		///Auto reset enabled, no need to call reset() after wait()
		Sync::Barrier* waitBeforeRender = nullptr;
		
		//@brief Ignored on MainThread.
		Sync::Barrier* signalAfterSubmit = nullptr;
	};

	typedef std::function<void()> render_call_func;

	struct RenderThreadData {
		/// @brief Pointer
		volatile uint32* currImg = nullptr;

		/// @brief Pointer to an array that will be filled by calling VkInit
		/// It contains the synchronization primitives that synchronize a Thread's rendering (queue submission)
		/// to the Swapchains nextFrame command.
		/// The pointer must point to an arbitrary but VALID arr_ptr
		/// It will be overwritten by a suitable array of the correct size
		wrap_ptr<arr_ptr<RenderSubmitSynchronizationPtrSet>>* syncPtrs = nullptr;
	};

	struct RenderThreadingProfile {
		/// @brief 
		bool includeMainThread = true;

		/// @brief A table of which threads are responsible for rendering which frames, the first vector corresponds to the main thread, IF includeMainThread is enabled
		std::vector<std::vector<uint32>> threadGroups = { {0,1} };
		
		/// @brief This method is called for rendering on the MainThread
		render_call_func mainThreadRender;

		/// @brief Data for the Render Threads
		std::vector<RenderThreadData> renderThreads = {};
	};
	struct PhyDevProfile {
		/// @brief list of required PhysicalDeviceExtensions
		std::vector<std::string> requestedExtensions;
		
		/// @brief See VkPhysicalDeviceType for possible values.
		std::vector<VkPhysicalDeviceType> acceptedTypes = { VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU };

		/// @brief This function can be used to manually check for compatabilities of physical devices
		std::function<bool(const VkPhysicalDeviceFeatures&, const VkPhysicalDeviceProperties&)> phyDevPredicate = [](const VkPhysicalDeviceFeatures&, const VkPhysicalDeviceProperties&) ->bool {return true; };
	};
	struct DevProfile {
		/// @brief The amount and types of the queues requested
		DeviceQueueCount queueProfile = DeviceQueueCount(1, 1, 1, 0);

		/// @brief Features to be enabled in Device Creation
		VkPhysicalDeviceFeatures features = {};

		/// @brief Requested Layers & Extensions, if they are not availabe, they are ignored.
		/// To check which extensions/layers are enabled (and therefore available) check the layers/extensions catalog of the Device
		/// Required Extensions for Debugging/GLFW are automatically added
		std::vector<std::string> requestedLayers;

		/// @brief Requested Layers & Extensions, if they are not availabe, they are ignored.
		/// To check which extensions/layers are enabled (and therefore available) check the layers/extensions catalog of the Device
		/// Required Extensions for Debugging/GLFW are automatically added
		std::vector<std::string> requestedExtensions;
	};
	struct SwapchainProfile {
		uint32 swapchainImageSize = 0; //Minimum usually

	};
	struct AppInfo {
		std::string name = "Application";
		uint32 version = 0;
	};
	struct VulkanInstanceInfo {
		/// @brief Information about the Application in general
		AppInfo appInfo;

		/// @brief Information about the PhysicalDevice that is to be chosen
		PhyDevProfile phyProf;
		
		/// @brief Information about the Device to be created
		DevProfile devProf;

		/// @brief Information about the SwapChain to be created
		SwapchainProfile swapChainProfile;

		/// @brief A description of the threading behaviour with respect to rendering
		/// The swapchain frames CAN be rendered asynchronously by assigning every thread (potentially including the main thread) 
		/// a set of frames. f.e. for a single thread application with 2 Images in the swapchain and rendering on the main thread,
		/// the RenderingProfile is the default profile : {true, {{0,1}}}. With states, that the first entry of the second vector describes which frames are rendered on the mainThread
		/// and the second vector specifies, that both frames should be rendered on the main thread.
		/// An Application with 2 Threads CAN specify : {true, {{0}, {1}}} to render frame 0 on the first thread (main thread) and frame 1 on the second thread.
		RenderThreadingProfile renderProfile;
		
		/// @brief Requested Layers & Extensions, if they are not availabe, they are ignored.
		/// To check which extensions/layers are enabled (and therefore available) check the layers/extensions catalog of the Vulkaninstance
		/// Required Extensions for Debugging/GLFW are automatically added
		std::vector<std::string> requestedLayers;

		/// @brief Requested Layers & Extensions, if they are not availabe, they are ignored.
		/// To check which extensions/layers are enabled (and therefore available) check the layers/extensions catalog of the Vulkaninstance
		/// Required Extensions for Debugging/GLFW are automatically added
		std::vector<std::string> requestedExtensions;

		/// @brief Window for Surface Creation
		Surface::Window* window = nullptr;

	};

	struct VulkanInterface;
	struct VulkanInstance;
	struct PhysicalDevice;
	struct Device;
	struct SwapChain;
	struct Pipeline;
	struct PipelineSet;
	struct CommandPoolSet;
	struct QueueSet;

#define FRIEND_CLUB friend Pipeline; friend PipelineSet; friend VulkanInterface; friend SwapChain; friend VulkanInstance; friend SwapChain; friend Device; friend QueueSet; friend CommandPoolSet;

	struct VulkanInterface {
	private:
		static wrap_ptr<VulkanInstance> inst;
	public:
		static void vkInit(const VulkanInstanceInfo& inf);

		static VulkanInstance& instance();

		static uint32 nextFrame();

		static void destroyInstance();
	};

	/// @brief Wrapper class for the VkInstance, follows RAII
	struct VulkanInstance {
		FRIEND_CLUB
	
		VkInstance hndl = nullptr;

	private:
		
		VkSurfaceKHR surface = nullptr;
		VulkanInstanceInfo instanceInfo;

		wrap_ptr<Catalog> layers;
		wrap_ptr<Catalog> extensions;
		VkDebugUtilsMessengerEXT debugMessenger;

	public:

		/// @brief PhysicalDevice handle. Only one device is supported (No DeviceGroup)
		wrap_ptr<PhysicalDevice> phyDev;

		/// @brief LogicalDevice handle, Only one device is supported
		wrap_ptr<Device> dev;

		/// @brief Create SwapChain
		wrap_ptr<SwapChain> swapChain;


		VulkanInstance(const VulkanInstanceInfo&);
		
		VulkanInstance(const VulkanInstance&) = delete;
		VulkanInstance(VulkanInstance&&) = delete;

		~VulkanInstance();

		VulkanInstance& operator=(const VulkanInstance&) = delete;
		VulkanInstance& operator=(VulkanInstance&&) = delete;

	private:
		
		/// @brief Called when the Swap chain is recreated (after the swapchain itself has been already been rebuilt)
		void _swapChainRecreateNotify();
		
		/// @brief Vulkan requires you to find a Physical device with which to define a logical device with which most of the interaction with Vulkan is then handled
		void _findPhysicalDevice();

		/// @brief Sets up the Device from the physical Device found previously
		void _setupDevice();

	};

	/// @brief POD style class representing ua Physical Device. 
	struct PhysicalDevice {
		FRIEND_CLUB

		VulkanInstance* vkInst = nullptr;
		VkPhysicalDevice hndl = nullptr;

	private:
		
		SurfaceProperties surfProps;
		PhyDevQueueFamilies queueFamilies;

		VkPhysicalDeviceProperties props;
		VkPhysicalDeviceFeatures features;
	public:
		PhysicalDevice(VulkanInstance*, VkPhysicalDevice, const SurfaceProperties&, const PhyDevQueueFamilies&, const VkPhysicalDeviceProperties&, const VkPhysicalDeviceFeatures&);
	};

	/// @brief Wrapper class for the VkDevice, follows RAII
	struct Device {
		FRIEND_CLUB

		VulkanInstance* vkInst = nullptr;
		VkDevice hndl = nullptr;

	private:
		wrap_ptr<Catalog> extensions;
		wrap_ptr<Catalog> layers;

		DeviceQueueProfile queueProf;
	public:
		
		Device(VulkanInstance* ptr);
		~Device();

	};

	// +++++++++++++++++++++ "Actual" Rendering components ++++++++++

	/// @brief A set of queues that can be used by a specific Thread/Component
	struct QueueSet {
		VkQueue graphics = nullptr;
		VkQueue present = nullptr;
		VkQueue transfer = nullptr;
		VkQueue compute = nullptr;

		/// @brief Fetches (already existing) queues, WITHOUT RESERVING. An index value for -1 means, that no Queue of that type is fetched
		/// @param g graphics queue Index 
		/// @param p present queue Index
		/// @param t transfer queue Index
		/// @param c compute queue Index
		QueueSet(VulkanInstance*, int g, int p, int t, int c = -1);

		/// @brief Contructs a new QueueSet, that is, it actually reserves queues 
		QueueSet(VulkanInstance*, QueueProfile);

	private:
		void _fetch(VulkanInstance*, int g, int p, int t, int c);
	};

	/// @brief A set of commandpools that can be used by a specific Thread/Component
	struct CommandPoolSet {
		VkCommandPool graphics = nullptr;
		VkCommandPool present = nullptr;
		VkCommandPool transfer = nullptr;
		VkCommandPool compute = nullptr;
		VulkanInstance* vkInst;

		CommandPoolSet(VulkanInstance*, QueueProfile, std::map<vk_queue_type, VkCommandPoolCreateFlags>& flags = std::map<vk_queue_type, VkCommandPoolCreateFlags>());
		~CommandPoolSet();
		
	private:
		VkCommandPool _create(uint32, VkCommandPoolCreateFlags);

	};

	/// @brief This is a wrapper class for the Sync Primitives used by the swapChain
	struct RenderSubmitSynchronizationSet {
		VulkanInstance* vkInst = nullptr;

		/// @brief ImageAvailableSemaphore
		VkSemaphore waitFor = nullptr;
		/// @brief RenderFinishedSemaphore
		VkSemaphore signal = nullptr;
		/// @brief RenderFinishedFence
		VkFence signalFence = nullptr;

		/// @brief Auto reset enabled, no need to call reset() after wait()
		Sync::Barrier signalAfterSubmit; 

		/// @brief RAII, takes the instance from VulkanInterface
		RenderSubmitSynchronizationSet();
		~RenderSubmitSynchronizationSet();
	};

	struct SwapChain {
		FRIEND_CLUB

		VulkanInstance* vkInst = nullptr;
		VkSwapchainKHR hndl = nullptr;

	private:
		
		uint32 frameCount = 0;

		arr_ptr<Sync::Barrier> begBarriers;
		arr_ptr<RenderSubmitSynchronizationSet> syncSet;

		/// For faster access
		arr_ptr<RenderSubmitSynchronizationPtrSet> ptrSets;
		arr_ptr<bool> mainThr;
		RenderThreadingProfile& prof;
		arr_ptr<uint32> thrdIndx;
		arr_ptr<volatile uint32*> currImgInd;
		/// 

		/// Images in the Swapchain
		arr_ptr<VkImage> swapChainImages;
		arr_ptr<VkImageView> imageViews;
		//arr_ptr<FrameBuffer> frameBuffers;
		/////////////////////////////////////////

		QueueSet queues;
		
		/// Swapchain properties
		VkSurfaceFormatKHR surfaceFormat;
		VkPresentModeKHR presentMode;
		VkExtent2D extend;
		///

	public:

		SwapChain(VulkanInstance*, SwapchainProfile);
		~SwapChain();

		/// @brief See RenderSubmitSynchronizationSet
		RenderSubmitSynchronizationPtrSet syncPrimitiveForFrame(uint32 indx);

		/// @brief This method is 90% of the magic of the SwapChain, it basically presents the next frame and 
		/// Causes the next to be rendered
		uint32 nextFrame();

		/// @return  SwapChain size, that is, how many frames can be rendered concurrently
		uint32 size();

	private:
		uint32 currentFrame = 0;

		void _nextFrame(uint32 imgIndx);
		
		/// @brief Called when the Swapchain is out of date and nextFrame is called,
		/// Propagates changes to VulkanInstances
		void _recreate();

		struct __SCProps {
			VkSurfaceFormatKHR surfaceFormat;
			VkPresentModeKHR presentMode;
			VkExtent2D extend;
		};

		void _destroy();
		void _setup(SwapchainProfile scprf);
		__SCProps queryFrom(const SurfaceProperties&);
	};

}