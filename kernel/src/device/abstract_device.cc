#include "device/abstract_device.h"

namespace rayshape
{
    namespace device
    {
        // Factory registration
        AbstractDevice::AbstractDevice(DeviceType device_type) : device_type_(device_type) {}
        AbstractDevice::~AbstractDevice() = default;
        DeviceType AbstractDevice::GetDeviceType() {
            return device_type_;
        }

        AbstractDevice *GetDevice(DeviceType type) {
            return GetGlobalDeviceMap()[type].get();
        }
        /*
         * All devices are stored in this map.
         * The actual Device is registered as runtime.
         */
        std::map<DeviceType, std::shared_ptr<AbstractDevice>> &GetGlobalDeviceMap() {
            static std::once_flag once;
            static std::shared_ptr<std::map<DeviceType, std::shared_ptr<AbstractDevice>>>
                device_map;
            std::call_once(once, []() {
                device_map.reset(new std::map<DeviceType, std::shared_ptr<AbstractDevice>>);
            });
            return *device_map;
        }

        bool IsHostDeviceType(DeviceType device_type) {
            return (device_type == DeviceType::CPU || device_type == DeviceType::X86
                    || device_type == DeviceType::ARM);
        }
    } // namespace device
} // namespace rayshape
