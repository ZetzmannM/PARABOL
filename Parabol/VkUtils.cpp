#include "VkUtils.h"

#include "ChannelPrintStream.h"

#include <string>
#include <algorithm>
#include <set>

#include "glm/vec2.hpp"
#include "hdr_util.h"
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
void VulkanInterface::destroyInstance() {
	inst.destroy();
}
uint32 VulkanInterface::nextFrame() {
	return inst->swapChain->nextFrame();
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
	for (uint32 i = 0; i < glfwExtCount; i++){
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

	if ((res = glfwCreateWindowSurface(this->hndl, inf.window->getGLFWHandle(), nullptr, &surface)) != VK_SUCCESS) { 
		PRINT_ERR("VulkanSetupFailure! (glfwCreateWindowSurface) " + std::to_string(res), CH_SEVERITY_HALT, CHANNEL_VULKAN);
	}

	//Now find a Physical device, that meets our expectations
	_findPhysicalDevice();
	
	//Now set up the Device
	_setupDevice();

	//Setup Swapchain
	swapChain = new SwapChain(this, this->instanceInfo.swapChainProfile);

}
void VulkanInstance::_swapChainRecreateNotify() {
	//Pipeline recreation etc. [TODO]
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

VulkanInstance::~VulkanInstance() {

	//SwapChain before Device
	this->swapChain.destroy();

	/// Device before Instance
	this->dev.destroy();

	vkDestroySurfaceKHR(this->hndl, this->surface, nullptr);

	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)(vkGetInstanceProcAddr(this->hndl, "vkDestroyDebugUtilsMessengerEXT"));
	func(this->hndl, this->debugMessenger, nullptr);

	vkDestroyInstance(this->hndl, nullptr);
}

Device::Device(VulkanInstance* ptr) : queueProf( ptr->instanceInfo.devProf.queueProfile ), vkInst( ptr ) {
	DeviceQueueCount& profile = ptr->instanceInfo.devProf.queueProfile;

	//Create the desired Queues
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfo;
	std::set<uint32> queueIds;
	
	for (auto it = profile.queueFamilies.begin(); it != profile.queueFamilies.end(); it++) {
		if (it->second.first > 0) { //If any queues were requested at all
			uint32 indx = 0;
			switch (it->first) {
			case vk_queue_type::graphics:
				if (ptr->phyDev->queueFamilies.graphics.set) { 
					indx = ptr->phyDev->queueFamilies.graphics.index;
				}
				break;
			case vk_queue_type::transfer:
				if (ptr->phyDev->queueFamilies.transfer.set) {
					indx = ptr->phyDev->queueFamilies.transfer.index;
				}
				break;
			case vk_queue_type::present:
				if (ptr->phyDev->queueFamilies.present.set) {
					indx = ptr->phyDev->queueFamilies.present.index;
				}
				break;
			case vk_queue_type::compute:
				if (ptr->phyDev->queueFamilies.compute.set) {
					indx = ptr->phyDev->queueFamilies.compute.index;
				}
				break;
			}
			if (!queueIds.count(indx)) { //Only unique queues, 
				VkDeviceQueueCreateInfo prf = {};
				prf.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

				prf.queueFamilyIndex = indx;
				prf.queueCount = it->second.first;
				prf.pQueuePriorities = &(it->second.second);

				queueCreateInfo.push_back(prf);
				queueIds.insert(indx);
			}
		}
	}
	// Comment: It is currently not possible to generate queues with different priorities... Maybe add that later?

	uint32 cnt = 0;
	VK_CHECK_ERR(vkEnumerateDeviceExtensionProperties(this->vkInst->phyDev->hndl, NULL, &cnt, NULL), "DeviceSetupFailure (vkEnumerateDeviceExtensionProperties query count) ", CHANNEL_VULKAN);
	std::vector<VkExtensionProperties> prop(cnt);
	VK_CHECK_ERR(vkEnumerateDeviceExtensionProperties(this->vkInst->phyDev->hndl, NULL, &cnt, prop.data()), "DeviceSetupFailure (vkEnumerateDeviceExtensionProperties) ", CHANNEL_VULKAN);

	this->extensions = new Catalog(prop.data(), cnt);
	
	VK_CHECK_ERR(vkEnumerateDeviceLayerProperties(this->vkInst->phyDev->hndl, &cnt, NULL), "DeviceSetupFailure (vkEnumerateDeviceLayerProperties query count) ", CHANNEL_VULKAN);
	std::vector<VkLayerProperties> lprop(cnt);
	VK_CHECK_ERR(vkEnumerateDeviceLayerProperties(this->vkInst->phyDev->hndl, &cnt, lprop.data()), "DeviceSetupFailure (vkEnumerateDeviceLayerProperties) ", CHANNEL_VULKAN);

	this->layers = new Catalog(lprop.data(), cnt);

	for (const std::string& ref : vkInst->instanceInfo.devProf.requestedExtensions) {
		this->extensions->addIfAvailable(ref);
	}
	for (const std::string& ref : vkInst->instanceInfo.devProf.requestedLayers) {
		this->layers->addIfAvailable(ref);
	}

	if (!this->extensions->addIfAvailable(VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
		PRINT_ERR("DeviceSetupFailure (No Swapchain extension available!)", CH_SEVERITY_HALT, CHANNEL_VULKAN);
	}

#ifdef _DEBUG
	//Request Validation Layers again (as with the instance)
	if (!this->layers->addIfAvailable("VK_LAYER_KHRONOS_validation")) {
		PRINT_ERR("Validation Layers not available", CH_SEVERITY_WARNING, CHANNEL_VULKAN);
	}
#endif

	Util::NonCpyStringContainer exts = this->extensions->toContainer(), lyrs = this->layers->toContainer();

	VkDeviceCreateInfo inf = {};
	inf.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	inf.pNext = nullptr;
	inf.queueCreateInfoCount = queueCreateInfo.size();
	inf.pQueueCreateInfos = queueCreateInfo.data();

	inf.pEnabledFeatures = &vkInst->instanceInfo.devProf.features;
	
	inf.enabledExtensionCount = exts.size();
	inf.ppEnabledExtensionNames = exts.data();
	inf.enabledLayerCount = lyrs.size();
	inf.ppEnabledLayerNames = lyrs.data();

	inf.flags = 0x0u; //Reserved for later use
	
	
	VK_CHECK_ERR(vkCreateDevice(vkInst->phyDev->hndl, &inf, nullptr, &hndl), "DeviceSetupFailure (vkCreateDevice) ", CHANNEL_VULKAN);
	DEBUG_PRINT("Logical Device successfully created", CHANNEL_VULKAN);
}
Device::~Device() {
	vkDestroyDevice(this->hndl, nullptr);
}

PhysicalDevice::PhysicalDevice(VulkanInstance* a, VkPhysicalDevice b, const SurfaceProperties& c, const PhyDevQueueFamilies& d, const VkPhysicalDeviceProperties& e, const VkPhysicalDeviceFeatures& f) : vkInst{ a }, hndl{ b }, surfProps{ c }, queueFamilies{ d }, props{ e }, features{ f } {}

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

DeviceQueueProfile::DeviceQueueProfile(const DeviceQueueCount& ref) {
	queueCount = ref;
}
int DeviceQueueProfile::reserveQueue(vk_queue_type type) {
	int res = -1;
	switch (type) {
	case vk_queue_type::graphics:
		res = this->graphicsQueueCounter.fetch_add(1);
		break;
	case vk_queue_type::present:
		res = this->presentQueueCounter.fetch_add(1);
		break;
	case vk_queue_type::transfer:
		res = this->transferQueueCounter.fetch_add(1);
		break;
	case vk_queue_type::compute:
		res = this->computeQueueCounter.fetch_add(1);
		break;
	}
	if (res >= static_cast<int>(this->queueCount.queueFamilies[type].first)) {
		return -1;
	}
	else {
		return res;
	}
}

QueueSet::QueueSet(VulkanInstance* dev, int g, int p, int t, int c) {
	_fetch(dev, g, p, t, c);
}
QueueSet::QueueSet(VulkanInstance* inst, QueueProfile prof) {
	int g = -1, p = -1, t = -1, c = -1;
	if (prof.graphics) {
		g= inst->dev->queueProf.reserveQueue(vk_queue_type::graphics);
		ASSERT(g> -1, "Failed to reserve Queue", CHANNEL_VULKAN);
	}
	if (prof.compute) {
		c = inst->dev->queueProf.reserveQueue(vk_queue_type::compute);
		ASSERT(c> -1, "Failed to reserve Queue", CHANNEL_VULKAN);
	}
	if (prof.present) {
		p = inst->dev->queueProf.reserveQueue(vk_queue_type::present);
		ASSERT(p> -1, "Failed to reserve Queue", CHANNEL_VULKAN);
	}
	if (prof.transfer) {
		t= inst->dev->queueProf.reserveQueue(vk_queue_type::transfer);
		ASSERT(t > -1, "Failed to reserve Queue", CHANNEL_VULKAN);
	}

	_fetch(inst, g,p,t,c);
}
void QueueSet::_fetch(VulkanInstance* ptr, int g, int p, int t, int c) {
	if (g> -1) {
		vkGetDeviceQueue(ptr->dev->hndl, ptr->phyDev->queueFamilies.graphics.index, g, &graphics);
	}
	if (p> -1) {
		vkGetDeviceQueue(ptr->dev->hndl, ptr->phyDev->queueFamilies.present.index, p, &present);
	}
	if (t> -1) {
		vkGetDeviceQueue(ptr->dev->hndl, ptr->phyDev->queueFamilies.transfer.index, t, &transfer);
	}
	if (c> -1) {
		vkGetDeviceQueue(ptr->dev->hndl, ptr->phyDev->queueFamilies.compute.index, c, &compute);
	}
}

DeviceQueueCount::DeviceQueueCount(uint16 g, uint16 p, uint16 t, uint16 c) {
	this->totalQueueCount += g;
	this->queueFamilies[VK::vk_queue_type::graphics] = { g, 1.0f };

	this->totalQueueCount += p;
	this->queueFamilies[VK::vk_queue_type::present] = { p, 1.0f };

	this->totalQueueCount += t;
	this->queueFamilies[VK::vk_queue_type::transfer] = { t, 1.0f };

	this->totalQueueCount += c;
	this->queueFamilies[VK::vk_queue_type::compute] = { c, 1.0f };
}

CommandPoolSet::CommandPoolSet(VulkanInstance* i, QueueProfile prf, std::map<vk_queue_type, VkCommandPoolCreateFlags>& flags) : vkInst{ i } {
	if (prf.graphics) {
		this->graphics = _create(this->vkInst->phyDev->queueFamilies.graphics.index, flags[vk_queue_type::graphics]);
	}
	if (prf.present) {
		this->present = _create(this->vkInst->phyDev->queueFamilies.present.index, flags[vk_queue_type::present]);
	}
	if (prf.transfer) {
		this->transfer = _create(this->vkInst->phyDev->queueFamilies.transfer.index, flags[vk_queue_type::transfer]);
	}
	if (prf.compute) {
		this->compute = _create(this->vkInst->phyDev->queueFamilies.compute.index, flags[vk_queue_type::compute]);
	}
}
CommandPoolSet::~CommandPoolSet() {
	if (graphics) {
		vkDestroyCommandPool(vkInst->dev->hndl, this->graphics, NULL);
	}
	if (present) {
		vkDestroyCommandPool(vkInst->dev->hndl, this->present, NULL);
	}
	if (transfer) {
		vkDestroyCommandPool(vkInst->dev->hndl, this->transfer, NULL);
	}
	if (compute) {
		vkDestroyCommandPool(vkInst->dev->hndl, this->compute, NULL);
	}
}
VkCommandPool CommandPoolSet::_create(uint32 ind, VkCommandPoolCreateFlags flags) {
	VkCommandPoolCreateInfo inf = {};
	inf.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	inf.pNext = nullptr;
	inf.flags = flags;
	inf.queueFamilyIndex = ind;

	VkCommandPool temp = nullptr;
	VK_CHECK_ERR(vkCreateCommandPool(this->vkInst->dev->hndl, &inf, NULL, &temp), "CommandPoolSetFailure (vkCreateCommandPool) ", CHANNEL_VULKAN);
	return temp;
}

SwapChain::SwapChain(VulkanInstance* ptr, SwapchainProfile scprf) : vkInst( ptr ), queues(ptr, {false, true,false,false} ) {
	_setup(scprf);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	this->imageAvailableSemaphore.resize(this->imageViews.size());
	this->renderFinishedSemaphore.resize(this->imageViews.size());
	this->renderFinishedFence.resize(this->imageViews.size());

	VkResult res = VK_SUCCESS;

	for (uint32 s = 0; s < this->imageViews.size(); s++) {
		if ((res = vkCreateSemaphore(vkInst->dev->hndl, &semaphoreInfo, nullptr, &this->imageAvailableSemaphore[s])) != VK_SUCCESS ||
			(res = vkCreateSemaphore(vkInst->dev->hndl, &semaphoreInfo, nullptr, &this->renderFinishedSemaphore[s])) != VK_SUCCESS) {
			PRINT_ERR("Render Semaphores creation failed! " + std::to_string(res), CH_SEVERITY_HALT, CHANNEL_VULKAN);
		}
		if ((res = vkCreateFence(vkInst->dev->hndl, &fenceInfo, NULL, &this->renderFinishedFence[s])) != VK_SUCCESS) {
			PRINT_ERR("Fence Creation failed " + std::to_string(res), CH_SEVERITY_HALT, CHANNEL_VULKAN);
		}
		renderBeginBarrier.push_back(new Sync::Barrier(true));
	}
}
void SwapChain::_setup(SwapchainProfile scprf) {
	SurfaceProperties& surfaceProp = vkInst->phyDev->surfProps;
	__SCProps props = queryFrom(surfaceProp);

	uint32 imageCount = std::clamp<uint32>(scprf.swapchainImageSize, surfaceProp.capabilities.minImageCount, surfaceProp.capabilities.maxImageCount);

	scprf.swapchainImageSize = imageCount;

	// ++++++++++++ Actual creation +++++++++++++

	VkSwapchainCreateInfoKHR inf = {};
	inf.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	inf.pNext = nullptr;

	inf.surface = this->vkInst->surface;
	inf.minImageCount = scprf.swapchainImageSize;
	inf.imageFormat = props.surfaceFormat.format;
	inf.imageColorSpace = props.surfaceFormat.colorSpace;
	inf.imageExtent = props.extend;
	inf.preTransform = surfaceProp.capabilities.currentTransform;
	inf.presentMode = props.presentMode;

	//[TODO] Maybe add as options later
	inf.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	inf.imageArrayLayers = 1; // 2 if Stereoscopic
	inf.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	inf.clipped = TRUE;
	//

	inf.oldSwapchain = VK_NULL_HANDLE; // [TODO] Useful for recreation?

	uint32 queueFamilyIndices[2] = { vkInst->phyDev->queueFamilies.graphics.index, vkInst->phyDev->queueFamilies.present.index };

	if (queueFamilyIndices[0] != queueFamilyIndices[1]) {
		inf.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		inf.queueFamilyIndexCount = 2;
		inf.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		inf.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		inf.queueFamilyIndexCount = 0; // Optional
		inf.pQueueFamilyIndices = nullptr; // Optional
	}

	VK_CHECK_ERR(vkCreateSwapchainKHR(this->vkInst->dev->hndl, &inf, NULL, &this->hndl), "SwapChainSetupFailure (vkCreateSwapchainKHR) ", CHANNEL_VULKAN);

	this->swapChainImages.clear();
	uint32 imgCount = 0;
	VK_CHECK_ERR(vkGetSwapchainImagesKHR(vkInst->dev->hndl, this->hndl, &imgCount, NULL), "SwapChainSetupFailure (vkGetSwapchainImagesKHR query count)", CHANNEL_VULKAN);
	this->swapChainImages.resize(imgCount);
	VK_CHECK_ERR(vkGetSwapchainImagesKHR(vkInst->dev->hndl, this->hndl, &imgCount, this->swapChainImages.data()), "vkGetSwapchainImagesKHR (vkGetSwapchainImagesKHR)", CHANNEL_VULKAN);
	
	this->surfaceFormat = props.surfaceFormat;
	this->extend = props.extend;
	this->presentMode = props.presentMode;

	///*++++++++++++++++++++++++++++++++++++
	/// Image View Creation:
	///*++++++++++++++++++++++++++++++++++++	

	imageViews.clear();
	imageViews.resize(swapChainImages.size());
	for (size_t i = 0; i < imageViews.size(); i++) {
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = this->surfaceFormat.format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		//[TODO] Make this an Option! 
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		// 

		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;
		VK_CHECK_ERR(vkCreateImageView(vkInst->dev->hndl, &createInfo, nullptr, &imageViews[i]), "Failed to create Imageview ", CHANNEL_VULKAN);
	}
}
void SwapChain::_destroy() {
	vkDestroySwapchainKHR(this->vkInst->dev->hndl, this->hndl, NULL);
}
SwapChain::~SwapChain() {
	_destroy();
	
	//Cleanup Semaphores & 
	for (VkSemaphore s : this->imageAvailableSemaphore) {
		vkDestroySemaphore(this->vkInst->dev->hndl, s, NULL);
	}
	for (VkSemaphore s : this->renderFinishedSemaphore) {
		vkDestroySemaphore(this->vkInst->dev->hndl, s, NULL);
	}
	for (VkFence f : this->renderFinishedFence) {
		vkDestroyFence(this->vkInst->dev->hndl, f, NULL);
	}
}
SwapChain::__SCProps SwapChain::queryFrom(const SurfaceProperties& prop) {
	__SCProps str = {};
	
	//PresentMode
	str.presentMode = VK_PRESENT_MODE_FIFO_KHR; //wow that was easy

	// Extend
	if (prop.capabilities.currentExtent.width != (uint32)0xFFFFFFFF) {
		str.extend = prop.capabilities.currentExtent;
	}
	else {
		glm::u32vec2 sz = vkInst->instanceInfo.window->getWindowSize();
		VkExtent2D screenSize = VkExtent2D();
		screenSize.width = MAX(prop.capabilities.minImageExtent.width, MIN(prop.capabilities.maxImageExtent.width, sz.x));
		screenSize.height = MAX(prop.capabilities.minImageExtent.height, MIN(prop.capabilities.maxImageExtent.height, sz.y));

		str.extend = screenSize;
	}

	// Format
	if (prop.formats.size() == 1 && prop.formats[0].format == VK_FORMAT_UNDEFINED) {
		str.surfaceFormat = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}
	for (const VkSurfaceFormatKHR& availableFormat : prop.formats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			str.surfaceFormat = availableFormat;
		}
	}

	str.surfaceFormat = prop.formats[0];

	return str;
}
uint32 SwapChain::nextFrame() {

	if(!first){
		VkSemaphore waitSems[] = { renderFinishedSemaphore[currentFrame] };

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = waitSems;

		VkSwapchainKHR swapChains[] = { this->hndl };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr; // Optional

		//This waits for the Client to have finished Rendering

		VK_CHECK_ERR(vkQueuePresentKHR(this->queues.present, &presentInfo), "Failed QueuePresent ", CHANNEL_VULKAN);

		//Advance to next Frame :)
		currentFrame = (currentFrame + 1u) % this->size();

	}
	else {
		first = false;
	}

	imageIndex = 0;
	vkWaitForFences(vkInst->dev->hndl, 1, &renderFinishedFence[currentFrame], VK_TRUE, UINT64_MAX);
	
	//Client can begin Rendering
	this->renderBeginBarrier[currentFrame]->signal();

	VkResult res = vkAcquireNextImageKHR(vkInst->dev->hndl, this->hndl, UINT64_MAX, imageAvailableSemaphore[currentFrame], VK_NULL_HANDLE, &imageIndex);
	if (res == VK_ERROR_OUT_OF_DATE_KHR) {
		_recreate();
		return currentFrame;
	}
	else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
		PRINT_ERR("vkAcquireNextImageKHR rt'd " + std::to_string(res), CH_SEVERITY_HALT, CHANNEL_VULKAN);
	}

	vkResetFences(vkInst->dev->hndl, 1, &renderFinishedFence[currentFrame]);

	return currentFrame;
}
void SwapChain::_recreate() {
	DEBUG_PRINT("SwapChainRecreate!", CHANNEL_VULKAN);
	this->_destroy();
	this->_setup(this->vkInst->instanceInfo.swapChainProfile);

	this->vkInst->_swapChainRecreateNotify();
}
uint32 SwapChain::size() {
	return this->imageViews.size();
}
RenderSubmitSynchronizationSet SwapChain::syncPrimitiveForFrame(uint32 indx) {
	return {&imageAvailableSemaphore[indx], &renderFinishedSemaphore[indx], &renderFinishedFence[indx], renderBeginBarrier[indx].get()};
}
