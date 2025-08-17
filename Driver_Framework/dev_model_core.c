#include "dev_model_core.h"

#include "shell_style.h"

int dev_model_core_init(dev_model_t dev_model[]) {
    if (dev_model == NULL) {
        return -1;  // 参数错误
    }
    unsigned int i = 0;
    // 使用 NULL 指针作为数组结束标志不再适用，需要其他方式
    // 假设数组以 device_name 为空字符串结束
    while (dev_model[i].device_name[0] != '\0') {
        // 分配索引
        dev_model[i].index = i;
        // 初始化设备状态
        if (dev_model[i].init != NULL) {
            if (!dev_model[i].init(dev_model[i].arg)) {
                // 初始化成功
                dev_model[i].status = 1;
#ifdef __FRONT_H
                success("Device %s initialized successfully.\n",
                        dev_model[i].device_name);
#else
                printf("Device %s initialized successfully.\n",
                       dev_model[i].device_name);
#endif
            } else {
                // 初始化失败
                dev_model[i].status = -1;
#ifdef __FRONT_H
                error("Device %s initialization failed.\n",
                      dev_model[i].device_name);
#else
                printf("Device %s initialization failed.\n",
                       dev_model[i].device_name);
#endif
            }
        }
        i++;
    }
    return 0;
}
