#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <map>
#include <functional>

#include "inttypes.h"
#include "ptr.h"
#include "StringUtils.h"

#include "inc_setting.h"

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
	struct DeviceQueueProfile {
		uint64 totalQueueCount = 0;
		/// @brief Pairs of the amount of queues and their priority
		std::map<vk_queue_type, std::pair<uint32, float>> queueFamilies;

		DeviceQueueProfile(uint16 g = 0, uint16 p = 0, uint16 t = 0, uint16 c = 0);
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
		DeviceQueueProfile queueProfile = DeviceQueueProfile(1, 1, 1, 0);
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

		/// @brief Requested Layers & Extensions, if they are not availabe, they are ignored.
		/// To check which extensions/layers are enabled (and therefore available) check the layers/extensions catalog of the Vulkaninstance
		/// Required Extensions for Debugging/GLFW are automatically added
		std::vector<std::string> requestedLayers;

		/// @brief Requested Layers & Extensions, if they are not availabe, they are ignored.
		/// To check which extensions/layers are enabled (and therefore available) check the layers/extensions catalog of the Vulkaninstance
		/// Required Extensions for Debugging/GLFW are automatically added
		std::vector<std::string> requestedExtensions;

		/// @brief Window for Surface Creation
		GLFWwindow* window = nullptr;

	};

	struct VulkanInterface {
	private:
		static wrap_ptr<VulkanInstance> inst;
	public:
		static void vkInit(const VulkanInstanceInfo& inf);

		static VulkanInstance& instance();

		static void destroyInstance();
	};

	struct PhysicalDevice;
	struct Device;

	/// @brief Wrapper class for the VkInstance, follows RAII
	struct VulkanInstance {
		/// @brief class used to keep track of layers/extensions that are enabled
		
		VkInstance hndl = nullptr;
		VkSurfaceKHR surface = nullptr;
		VulkanInstanceInfo instanceInfo;

		/// @brief PhysicalDevice handle. Only one device is supported (No DeviceGroup)
		wrap_ptr<PhysicalDevice> phyDev; 

		/// @brief LogicalDevice handle, Only one device is supported
		wrap_ptr<Device> dev;

		wrap_ptr<Catalog> layers;
		wrap_ptr<Catalog> extensions;
		VkDebugUtilsMessengerEXT debugMessenger;

		VulkanInstance(const VulkanInstanceInfo&);
		
		VulkanInstance(const VulkanInstance&) = delete;
		VulkanInstance(VulkanInstance&&) = delete;

		~VulkanInstance();

		VulkanInstance& operator=(const VulkanInstance&) = delete;
		VulkanInstance& operator=(VulkanInstance&&) = delete;

	private:

		/// @brief Vulkan requires you to find a Physical device with which to define a logical device with which most of the interaction with Vulkan is then handled
		void _findPhysicalDevice();

		/// @brief Sets up the Device from the physical Device found previously
		void _setupDevice();

	};

	/// @brief POD style class representing a Physical Device. 
	struct PhysicalDevice {
		VulkanInstance* vkInst = nullptr;
		VkPhysicalDevice hndl = nullptr;

		SurfaceProperties surfProps;
		PhyDevQueueFamilies phyDev;

		VkPhysicalDeviceProperties props;
		VkPhysicalDeviceFeatures features;

		PhysicalDevice(VulkanInstance*, VkPhysicalDevice, const SurfaceProperties&, const PhyDevQueueFamilies&, const VkPhysicalDeviceProperties&, const VkPhysicalDeviceFeatures&);
	};

	/// @brief Wrapper class for the VkDevice, follows RAII
	struct Device {
		VulkanInstance* vkInst;
		VkDevice hndl;

		DeviceQueueProfile queueProf;

		Device(VulkanInstance* ptr);
		~Device();

	};




}