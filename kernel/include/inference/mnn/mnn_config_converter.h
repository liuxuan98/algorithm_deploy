#ifndef _MNN_CONFIG_CONVERTER_H_
#define _MNN_CONFIG_CONVERTER_H_

#include "mnn_config.h"

namespace rayshape
{
    namespace mnn
    {
        class MnnConfigConverter {
        public: // Fp16 ,unint8 ,fp32
            static ErrorCode ConvertFromNetworkConfig(MNN::ScheduleConfig &schedule_config,
                                                      DeviceType device_type, int num_threads,
                                                      Precision precision);
        };

    } // namespace mnn
} // namespace rayshape

#endif // _MNN_CONFIG_CONVERTER_H_