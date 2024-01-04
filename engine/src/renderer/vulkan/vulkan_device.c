#include "vulkan_device.h"
#include "../../core/logger.h"
#include "../../core/vstring.h"
#include "../../core/vmemory.h"
#include "../../containers/darray.h"

typedef struct vulkan_physical_device_requirements {
    b8 graphics;
    b8 present;
    b8 compute;
    b8 transfer;
    // darray
    const char** device_extension_names;
    b8 sampler_anisotropy;
    b8 discrete_gpu;
} vulkan_physical_device_requirements;

typedef struct vulkan_physical_device_queue_family_info {
    u32 graphics_family_index;
    u32 present_family_index;
    u32 compute_family_index;
    u32 transfer_family_index;
} vulkan_physical_device_queue_family_info;

b8 select_physical_device(vulkan_context* context);
b8 physical_device_meets_requirements(
    VkPhysicalDevice device, 
    VkSurfaceKHR surface,
    const VkPhysicalDeviceProperties* properties,
    const VkPhysicalDeviceFeatures* features,
    const vulkan_physical_device_requirements* requirements,
    vulkan_physical_device_queue_family_info* out_queue_family_info,
    vulkan_swapchain_support_info* out_swapchain_support);

b8 vulkan_device_create(vulkan_context* context){
    if(!select_physical_device(context)){
        return FALSE;
    }
}

void vulkan_device_destroy(vulkan_context* context){

}

void vulkan_device_query_swapchain_support(VkPhysicalDevice physical_device, VkSurfaceKHR surface, vulkan_swapchain_support_info* out_support_info){
    // Surface capabilities
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &out_support_info->capabilities));

    // Surface formats
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &out_support_info->format_count, 0));
    if(out_support_info->format_count != 0){
        if(!out_support_info->formats){
            out_support_info->formats = vallocate(sizeof(VkSurfaceFormatKHR) * out_support_info->format_count, MEMORY_TAG_RENDERER);
        }
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &out_support_info->format_count, &out_support_info->formats));    
    }

    // Present modes
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &out_support_info->present_mode_count, 0));
    if(out_support_info->present_mode_count != 0){
        if(!out_support_info->present_modes){
            out_support_info->present_modes = vallocate(sizeof(VkPresentModeKHR) * out_support_info->present_mode_count, MEMORY_TAG_RENDERER);
        }
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &out_support_info->present_mode_count, &out_support_info->present_modes));    
    }

}

b8 select_physical_device(vulkan_context* context){
    u32 physical_device_count = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physical_device_count, 0));
    if(physical_device_count == 0){
        VFATAL("No devices which support Vulkan were found.");
        return FALSE;
    }

    VkPhysicalDevice physical_devices[physical_device_count];
    VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physical_device_count, physical_devices));
    for(u32 i = 0; i < physical_device_count; ++i){
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physical_devices[i], &properties);

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(physical_devices[i], &features);

        VkPhysicalDeviceMemoryProperties memory;
        vkGetPhysicalDeviceMemoryProperties(physical_devices[i], &memory);
    
        // TODO: These requirements should probably be driven by engine configuration
        vulkan_physical_device_requirements requirements = {};
        requirements.graphics = TRUE;
        requirements.present = TRUE;
        requirements.transfer = TRUE;
        // NOTE: Enable this if compute will be required.
        // requirements.compute = TRUE;
        requirements.sampler_anisotropy = TRUE;
        requirements.discrete_gpu = TRUE;
        requirements.device_extension_names = darray_create(const char*);
        darray_push(requirements.device_extension_names, &VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        vulkan_physical_device_queue_family_info queue_info = {};
        b8 result = physical_device_meets_requirements(
            physical_devices[i],
            context->surface,
            &properties,
            &features,
            &requirements,
            &queue_info,
            &context->device.swapchain_support);

        if(result) {
            VINFO("Selected device: '%s'.", properties.deviceName);
            // GPU type, etc.
            // Continue @ Kohi #015, 39:39 
        }
    }
}

b8 physical_device_meets_requirements(
    VkPhysicalDevice device, 
    VkSurfaceKHR surface,
    const VkPhysicalDeviceProperties* properties,
    const VkPhysicalDeviceFeatures* features,
    const vulkan_physical_device_requirements* requirements,
    vulkan_physical_device_queue_family_info* out_queue_info,
    vulkan_swapchain_support_info* out_swapchain_support) {

    // Evaluate device properties to determine if it meets the needs of outr application
    out_queue_info->graphics_family_index = -1;
    out_queue_info->present_family_index = -1;
    out_queue_info->compute_family_index = -1;
    out_queue_info->transfer_family_index = -1;

    // Discrete GPU?
    if(requirements->discrete_gpu) {
        if(properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            VINFO("Device is not a discrete GPU, and one is required. Skipping.");
            return FALSE;
        }
    }

    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, 0);
    VkQueueFamilyProperties queue_families[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

    // Look at each queue and see what queues it supports
    VINFO("Graphics | Present | Compute | Transfer | Name");
    u8 min_transfer_score = 255;
    for(u32 i = 0; i < queue_family_count; ++i){
        u8 current_transfer_score = 0;

        // Graphics Queue?
        if(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
            out_queue_info->graphics_family_index = i;
            ++current_transfer_score;
        }

        // Compute Queue?
        if(queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT){
            out_queue_info->compute_family_index = i;
            ++current_transfer_score;
        }

        // Transfer Queue?
        if(queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT){
            // Take the index if it is the current lowest. This increases
            // the liklihood that it is a dedicated transfer queue.
            if(current_transfer_score <= min_transfer_score){
                min_transfer_score = current_transfer_score;
                out_queue_info->transfer_family_index = i;
            }
        }

        // Present Queue?
        VkBool32 supports_present = VK_FALSE;
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supports_present));
        if(supports_present){
            out_queue_info->present_family_index = i;
        }
    }

    // Print out some info about the device
    VINFO("       %d |       %d |       %d |        %d | %s",
          out_queue_info->graphics_family_index != -1,
          out_queue_info->present_family_index != -1,
          out_queue_info->compute_family_index != -1,
          out_queue_info->transfer_family_index != -1,
          properties->deviceName);

    if(
        (!requirements->graphics || (requirements->graphics && out_queue_info->graphics_family_index != -1)) &&
        (!requirements->compute || (requirements->compute && out_queue_info->compute_family_index != -1)) &&
        (!requirements->present || (requirements->present && out_queue_info->present_family_index != -1)) &&
        (!requirements->transfer || (requirements->transfer && out_queue_info->transfer_family_index != -1))){
        VINFO("Device meets queue requirements.");
        VTRACE("Graphics Family Index: %i", out_queue_info->graphics_family_index);    
        VTRACE("Present Family Index: %i", out_queue_info->present_family_index);
        VTRACE("Compute Family Index: %i", out_queue_info->compute_family_index);
        VTRACE("Transfer Family Index: %i", out_queue_info->transfer_family_index);

        // Query swapchain support.
        vulkan_device_query_swapchain_support(device, surface, out_swapchain_support);

        if(out_swapchain_support->format_count < 1 || out_swapchain_support->present_mode_count < 1){
            if(out_swapchain_support->formats){
                vfree(out_swapchain_support->formats, sizeof(VkSurfaceFormatKHR) * out_swapchain_support->format_count, MEMORY_TAG_RENDERER);
            }
            if(out_swapchain_support->present_modes){
                vfree(out_swapchain_support->present_modes, sizeof(VkPresentModeKHR) * out_swapchain_support->present_mode_count, MEMORY_TAG_RENDERER);
            }
            VINFO("Required swapchain support not present, skipping device.");
            return FALSE;
        }

        // Device extensions
        if(requirements->device_extension_names) {
            u32 available_extension_count = 0;
            VkExtensionProperties* available_extensions = 0;
            VK_CHECK(vkEnumerateDeviceExtensionProperties(device, 0, &available_extension_count, 0));
            if(available_extension_count != 0){
                available_extensions = vallocate(sizeof(VkExtensionProperties) * available_extension_count, MEMORY_TAG_RENDERER);
                VK_CHECK(vkEnumerateDeviceExtensionProperties(device, 0, &available_extension_count, available_extensions));

                u32 required_extension_count = darray_length(requirements->device_extension_names);
                for(u32 i = 0; i < required_extension_count; ++i){
                    b8 found = FALSE;
                    for(u32 j = 0; j < available_extension_count; ++j){
                        if(strings_equal(requirements->device_extension_names[i], available_extensions[j].extensionName)){
                            found = TRUE;
                            break;
                        }
                    }

                    if(!found){
                        VINFO("Reuired extension not found: '%s', skipping device.", requirements->device_extension_names[i]);
                        vfree(available_extensions, sizeof(VkExtensionProperties) * available_extension_count, MEMORY_TAG_RENDERER);
                        return FALSE;
                    }
                }

            }
            vfree(available_extensions, sizeof(VkExtensionProperties) * available_extension_count, MEMORY_TAG_RENDERER);
        }

        // Sampler Anisotropy
        if(requirements->sampler_anisotropy && !features->samplerAnisotropy){
            VINFO("Device does not support samplerAnisotropy, skipping.");
            return FALSE;
        }

        // Device meets all requirements
        return TRUE;
    }

    return FALSE;
}