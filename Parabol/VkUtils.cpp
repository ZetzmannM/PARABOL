#include "VkUtils.h"

#include "ChannelPrintStream.h"

#include <string>
#include <algorithm>

#include "inc_setting.h"

using namespace VK;

wrap_ptr<VulkanInstance> VulkanInterface::inst = nullptr;

void VulkanInterface::vkInit(const VulkanInstanceInfo& inf) {
	if (!VulkanInterface::inst) {
		VulkanInterface::inst = new VulkanInstance(inf);
	}
	else {
		PRINT_ERR("Vulkan instance already initialized!", CH_SEVERITY_HALT, CHANNEL_VULKAN);
	}
}
VulkanInstance& VulkanInterface::instance() {
	if (VulkanInterface::inst) {
		return *VulkanInterface::inst.get();
	}
	else {
		PRINT_ERR("vkInit has not been called!", CH_SEVERITY_HALT, CHANNEL_VULKAN);
	}
}
//VkDebugCallbackFunction

#ifdef _DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL _debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	std::string severity = "";
	std::string type = "";
	bool bl = false;
	bool print = true;

	switch (messageSeverity) {
	case VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT:
		severity = "MAX";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		severity = "VERBOSE";
		print = false;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		severity = "INFO";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		severity = "WARNING";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		severity = "ERROR";
		bl = true;
		break;
	}

	switch (messageType) {
	case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
		type = "PERFORMANCE";
		break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
		type = "VALIDATION";
		break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
		type = "GENERAL";
		break;
	}

#if __SKIP_VERBOSE
	if (print) {
#endif
		if (bl) {
			PRINT_ERR("{" + type + "} " + pCallbackData->pMessage, CH_SEVERITY_HALT, CHANNEL_VULKAN);
		}
		else {
			PRINT("{" + severity + "|" + type + "} " + pCallbackData->pMessage, CHANNEL_VULKAN);
		}
#if __SKIP_VERBOSE
	}
#endif
	return VK_FALSE;
}
#endif
//++++++++++++++++

VulkanInstance::VulkanInstance(const VulkanInstanceInfo& inf) : instanceInfo{inf} {
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.applicationVersion = inf.appInfo.version;
	appInfo.pApplicationName = inf.appInfo.name.c_str();

	appInfo.pEngineName = "Parabol";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.apiVersion = VK_API_VERSION_1_2;

	VkInstanceCreateInfo createInfo = {};
	createInfo.pNext = nullptr;
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// Fetch Available Extensions & Layers --------------
	uint32 cnt = 0;
	VkResult res = vkEnumerateInstanceExtensionProperties(nullptr, &cnt, nullptr);
	ASSERT(res == VK_SUCCESS, "VulkanInitFailure (vkEnumerateInstanceExtensionProperties Recieve ExtensionCount) " +std::to_string(res) , CHANNEL_VULKAN); 

	std::vector<VkExtensionProperties> prop(cnt);
	res = vkEnumerateInstanceExtensionProperties(nullptr, &cnt, prop.data());
	ASSERT(res == VK_SUCCESS, "VulkanInitFailure (vkEnumerateInstanceExtensionProperties Recieve Extensions) " + std::to_string(res), CHANNEL_VULKAN);

	this->extensions = new Catalog(prop.data(), cnt);

	res = vkEnumerateInstanceLayerProperties(&cnt, nullptr);
	ASSERT(res == VK_SUCCESS, "VulkanInitFailure (vkEnumerateInstanceLayerProperties Recieve LayerCount) " +std::to_string(res), CHANNEL_VULKAN);

	std::vector<VkLayerProperties> propLayer(cnt);
	res = vkEnumerateInstanceLayerProperties(&cnt, propLayer.data());
	ASSERT(res == VK_SUCCESS, "VulkanInitFailure (vkEnumerateInstanceLayerProperties Recieve Layers) " + std::to_string(res), CHANNEL_VULKAN);

	this->layers = new Catalog(propLayer.data(), cnt);

	// --------------------------------------------------
	//Necessary extensions for GLFW, VK_KHR_surface f.e.
	uint32 glfwExtCount = 0;
	const char** exts = glfwGetRequiredInstanceExtensions(&glfwExtCount);
	ASSERT(exts != NULL, "VulkanInitFailure (glfwGetRequiredInstanceExtensions Recieve Extension)", CHANNEL_VULKAN);
	
	const char** curr = exts;
	bool flag = true;
	for (int i = 0; i < glfwExtCount; i++){
		flag &= this->extensions->addIfAvailable(std::string(*curr));
		curr++;
	}
	ASSERT(flag, "VulkanInitFailure (Necessary Extensions for GLFW not available)", CHANNEL_VULKAN);

	//Add requested stuff
	for (const std::string& ref : inf.requestedExtensions) {
		this->extensions->addIfAvailable(ref);
	}
	for (const std::string& ref : inf.requestedLayers) {
		this->layers->addIfAvailable(ref);
	}

#ifdef _DEBUG
	//Add debug layers/extensions: (Validation Layers)
	bool vl;
	if (!(vl = this->layers->addIfAvailable("VK_LAYER_KHRONOS_validation"))) {
		PRINT_ERR("Validation Layers not available", CH_SEVERITY_WARNING, CHANNEL_VULKAN);
	}

	bool dbgExt;
	if (!(dbgExt = this->extensions->addIfAvailable(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))) {
		PRINT_ERR("Debug Callback not available", CH_SEVERITY_WARNING, CHANNEL_VULKAN);
	}

	//Create Callback function

	VkDebugUtilsMessengerCreateInfoEXT dcreateInfo = {};
	dcreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	dcreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	dcreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	dcreateInfo.pfnUserCallback = _debugCallback;
	dcreateInfo.pUserData = NULL;

	VkValidationFeatureEnableEXT enables[] = { VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT };
	VkValidationFeaturesEXT features = {};
	features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
	features.enabledValidationFeatureCount = 1;
	features.pEnabledValidationFeatures = enables;

	createInfo.pNext = &features;
#endif

	Util::NonCpyStringContainer extes = this->extensions->toContainer();
	Util::NonCpyStringContainer layers = this->layers->toContainer();

	createInfo.enabledExtensionCount = extes.size();
	createInfo.ppEnabledExtensionNames = extes.data();

	createInfo.enabledLayerCount = layers.size();
	createInfo.ppEnabledLayerNames = layers.data();
	
	//Finally Create the Instance:
	res = vkCreateInstance(&createInfo, nullptr, &this->hndl);
	ASSERT(res == VK_SUCCESS, "VulkanInitFailure (vkCreateInstance) ", CHANNEL_VULKAN);

#ifdef _DEBUG
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)(vkGetInstanceProcAddr(this->hndl, "vkCreateDebugUtilsMessengerEXT"));
	res = func(this->hndl, &dcreateInfo, nullptr, &debugMessenger);
	ASSERT(res == VK_SUCCESS, "VulkanPostInitFailure (PFN_vkCreateDebugUtilsMessengerEXT)" + std::to_string(res), CHANNEL_VULKAN);
#endif
	
	// ++++++++++++++++++++++++++++++++++++++++++++
	// SURFACE
	// ++++++++++++++++++++++++++++++++++++++++++++

	if ((res = glfwCreateWindowSurface(this->hndl, inf.window, nullptr, &surface)) != VK_SUCCESS) { 
		PRINT_ERR("VulkanSetupFailure! (glfwCreateWindowSurface) " + std::to_string(res), CH_SEVERITY_HALT, CHANNEL_VULKAN);
	}

	//Now find a Physical device, that meets our expectations
	_findPhysicalDevice();
	
	//Now set up the Device
	_setupDevice();
}

void VulkanInstance::_setupDevice() {
	this->dev = new Device(this);
}
void VulkanInstance::_findPhysicalDevice() {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(this->hndl, &deviceCount, nullptr);

	if (!deviceCount) {
		PRINT_ERR("No physical device found!", CH_SEVERITY_HALT, CHANNEL_VULKAN);
	} else {
		DEBUG_PRINT(std::to_string(deviceCount) + " Devices Found" , CHANNEL_VULKAN);
	}
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(this->hndl, &deviceCount, devices.data());

	std::vector<std::string> requiredExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME //Very gowd :>
	};
	requiredExtensions.insert(requiredExtensions.begin(), this->instanceInfo.phyProf.requestedExtensions.begin(), this->instanceInfo.phyProf.requestedExtensions.end());

	//Make required Extensions unique
	requiredExtensions.erase(std::unique(requiredExtensions.begin(), requiredExtensions.end()), requiredExtensions.end());

	//Search for fitting device
	VkPhysicalDeviceProperties prop;
	VkPhysicalDeviceFeatures features;
	std::vector<bool> checklist (requiredExtensions.size());
	for (VkPhysicalDevice& dev : devices) {
		vkGetPhysicalDeviceProperties(dev, &prop);
		vkGetPhysicalDeviceFeatures(dev, &features);

		if (!this->instanceInfo.phyProf.phyDevPredicate(features, prop)) {
			continue;
		}
		
		DEBUG_PRINT("Found device " + std::string(prop.deviceName) + " With API v" + 
			std::to_string(VK_VERSION_MAJOR(prop.apiVersion)) + "." +
			std::to_string(VK_VERSION_MINOR(prop.apiVersion)) + "." +
			std::to_string(VK_VERSION_PATCH(prop.apiVersion)), CHANNEL_VULKAN);

		uint32 cnt = 0;
		VkResult res;
		if ((res = vkEnumerateDeviceExtensionProperties(dev, NULL, &cnt, NULL)) != VK_SUCCESS) {
			PRINT_ERR("Instantiation FAILED! (vkEnumerateDeviceExtensionProperties) " + std::to_string(res), CH_SEVERITY_HINT, CHANNEL_VULKAN);
			continue;
		}
		
		std::vector<VkExtensionProperties> extProp(cnt);
		if ((res = vkEnumerateDeviceExtensionProperties(dev, NULL, &cnt, extProp.data())) != VK_SUCCESS) {
			PRINT_ERR("Instantiation FAILED! (vkEnumerateDeviceExtensionProperties) "+std::to_string(res), CH_SEVERITY_HINT, CHANNEL_VULKAN);
			continue;
		}

		int matches = 0;
		for (auto reqIt = requiredExtensions.begin(); reqIt != requiredExtensions.end(); reqIt++) {
			for (auto it = extProp.begin(); it != extProp.end(); it++) {
				if (*reqIt == std::string(it->extensionName)) {
					matches++;
					break;
				}
			}
		}

		if (matches == requiredExtensions.size()) {
			//Has all required Extensions
			//Now check SwapChain Properties
			SurfaceProperties props;

			//Fetch Surface Properties:
			VkResult res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(dev, surface, &props.capabilities);
			if (res != VK_SUCCESS) {
				PRINT_ERR("PhyDevSetupFail (vkGetPhysicalDeviceSurfaceCapabilitiesKHR)" + std::to_string(res), CH_SEVERITY_HINT, CHANNEL_VULKAN);
			}

			uint32 cnt = 0;
			vkGetPhysicalDeviceSurfaceFormatsKHR(dev, surface, &cnt, NULL);
			if (res != VK_SUCCESS) {
				PRINT_ERR("PhyDevSetupFail (vkGetPhysicalDeviceSurfaceFormatsKHR)" + std::to_string(res), CH_SEVERITY_HINT, CHANNEL_VULKAN);
				continue;
			}
			if (cnt == 0) { 
				PRINT_ERR("PhyDevSetupFail (No Format)", CH_SEVERITY_HINT, CHANNEL_VULKAN); 
				continue;
			}

			props.formats.resize(cnt);

			res = vkGetPhysicalDeviceSurfaceFormatsKHR(dev, surface, &cnt, props.formats.data());
			if (res != VK_SUCCESS) {
				PRINT_ERR("PhyDevSetupFail (vkGetPhysicalDeviceSurfaceFormatsKHR) " + std::to_string(res), CH_SEVERITY_HINT, CHANNEL_VULKAN);
				continue;
			}
			
			vkGetPhysicalDeviceSurfacePresentModesKHR(dev, surface, &cnt, NULL);
			if (res != VK_SUCCESS) {
				PRINT_ERR("PhyDevSetupFail (vkGetPhysicalDeviceSurfacePresentModesKHR) " + std::to_string(res), CH_SEVERITY_HINT, CHANNEL_VULKAN);
				continue;
			}

			if (cnt == 0) {
				PRINT_ERR("PhyDevSetupFail (No Present Modes)", CH_SEVERITY_HINT, CHANNEL_VULKAN);
				continue;
			}

			props.presentModes.resize(cnt);
			if ((res = vkGetPhysicalDeviceSurfacePresentModesKHR(dev, surface, &cnt, props.presentModes.data())) != VK_SUCCESS) {
				PRINT_ERR("PhyDevSetupFail (vkGetPhysicalDeviceSurfacePresentModesKHR) "+std::to_string(res), CH_SEVERITY_HINT, CHANNEL_VULKAN);
				continue;
			}
				
			/// Finding Queue indices ():
			PhyDevQueueFamilies phyDevFamilies;

			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(dev, &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(dev, &queueFamilyCount, queueFamilies.data());

			for (uint32 i = 0; i < queueFamilyCount; i++) {
				VkBool32 supported = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, surface, &supported);
				if (queueFamilies.at(i).queueCount > 0 && (queueFamilies.at(i).queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
					phyDevFamilies.graphics.index = i;
					phyDevFamilies.graphics.set = true;
				}
				if (queueFamilies.at(i).queueCount > 0 && supported) {
					phyDevFamilies.present.index = i;
					phyDevFamilies.present.set = true;
				}
				if (queueFamilies.at(i).queueCount > 0 && (queueFamilies.at(i).queueFlags & VK_QUEUE_COMPUTE_BIT)) {
					phyDevFamilies.compute.index = i;
					phyDevFamilies.compute.set = true;
				}
				if (queueFamilies.at(i).queueCount > 0 && (queueFamilies.at(i).queueFlags & VK_QUEUE_TRANSFER_BIT)) {
					phyDevFamilies.transfer.index = i;
					phyDevFamilies.transfer.set = true;
				}
				if (phyDevFamilies.isComplete()) {
					break;
				}
			}

			//Finally actually check the Devices:
			if ((std::find(this->instanceInfo.phyProf.acceptedTypes.begin(), this->instanceInfo.phyProf.acceptedTypes.end(), prop.deviceType) != this->instanceInfo.phyProf.acceptedTypes.end())
				&&	phyDevFamilies.isComplete()) {
				DEBUG_PRINT("Found device: " + std::string(prop.deviceName), CHANNEL_VULKAN);

				//Populate the PhysicalDevice struct
				this->phyDev = new PhysicalDevice(this, dev, props, phyDevFamilies, prop, features);
				break;
			}
		}
	}

	ASSERT(this->phyDev, "PhyDevSetupFailure (No Device Found!)", CHANNEL_VULKAN);
}

Device::Device(VulkanInstance* ptr) : vkInst{ ptr }, queueProf{ ptr->instanceInfo.devProf.queueProfile }{
	
}
Device::~Device() {
	vkDestroyDevice(this->hndl, nullptr);
}


PhysicalDevice::PhysicalDevice(VulkanInstance* a, VkPhysicalDevice b, const SurfaceProperties& c, const PhyDevQueueFamilies& d, const VkPhysicalDeviceProperties& e, const VkPhysicalDeviceFeatures& f)
	: vkInst{ a }, hndl{ b }, surfProps{ c }, phyDev{ d }, props{ e }, features{ f } {}

void VulkanInterface::destroyInstance() {
	inst.destroy();
}
VulkanInstance::~VulkanInstance() {

	/// Device before Instance
	this->dev.destroy();

	vkDestroySurfaceKHR(this->hndl, this->surface, nullptr);

	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)(vkGetInstanceProcAddr(this->hndl, "vkDestroyDebugUtilsMessengerEXT"));
	func(this->hndl, this->debugMessenger, nullptr);

	vkDestroyInstance(this->hndl, nullptr);
}

Catalog::Catalog(VkExtensionProperties* prop, int size) {
	for (int i = 0; i < size; i++) {
		this->values[std::string(prop[i].extensionName)] = false;
	}
}
Catalog::Catalog(VkLayerProperties* prop, int size) {
	for (int i = 0; i < size; i++) {
		this->values[std::string(prop[i].layerName)] = false;
	}
}
bool Catalog::isAvailable(const std::string& ref) {
	return this->values.count(ref);
}
bool Catalog::addIfAvailable(const std::string& ref) {
	if (isAvailable(ref)) {
		this->values[ref] = true;
		return true;
	}
	return false;
}
Util::NonCpyStringContainer Catalog::toContainer() {
	Util::NonCpyStringContainer cont;
	for (auto it = this->values.begin(); it != this->values.end(); it++) {
		if(it->second){
			cont.add(it->first);
		}
	}
	return cont;
}
bool Catalog::isEnabled(const std::string& ref) {
	if (isAvailable(ref)) {
		return this->values[ref];
	}
	return false;
}
bool PhyDevQueueFamilies::isComplete() {
	return graphics.set && present.set;
}

DeviceQueueProfile::DeviceQueueProfile(uint16 g, uint16 p, uint16 t, uint16 c) {
	this->totalQueueCount += g;
	this->queueFamilies[VK::vk_queue_type::graphics] = { g, 1.0f };

	this->totalQueueCount += p;
	this->queueFamilies[VK::vk_queue_type::present] = { p, 1.0f };
	
	this->totalQueueCount += t;
	this->queueFamilies[VK::vk_queue_type::transfer] = { t, 1.0f };
	
	this->totalQueueCount += c;
	this->queueFamilies[VK::vk_queue_type::compute] = {c, 1.0f};
}