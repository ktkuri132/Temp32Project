#include "dev_frame.h"
#include "shell/shell_style.h"
#include <string.h>
int Device_Registration(dev_info_t dev_info[])
{
    if (dev_info == NULL)
    {
        return -1; // 参数错误
    }
    unsigned int i = 0;
    // 使用 NULL 指针作为数组结束标志不再适用，需要其他方式
    // 假设数组以 name 为空字符串结束
    while (dev_info[i].name[0] != '\0')
    {
        // 分配索引
        dev_info[i].index = i;
        // 初始化设备状态
        if (dev_info[i].init != NULL)
        {
            if (!dev_info[i].init(dev_info[i].arg))
            {
                // 初始化成功
                dev_info[i].status = 1;
#ifdef __FRONT_H
                success("Device %s initialized successfully.\n",
                        dev_info[i].name);
#else
                printf("Device %s initialized successfully.\n",
                       dev_info[i].name);
#endif
            }
            else
            {
                // 初始化失败
                dev_info[i].status = -1;
#ifdef __FRONT_H
                error("Device %s initialization failed.\n",
                      dev_info[i].name);
#else
                printf("Device %s initialization failed.\n",
                       dev_info[i].name);
#endif
            }
        }
        i++;
    }
    return 0;
}

int Find_Device(dev_info_t dev_info[],const char *name, dev_info_t **device)
{
    if (name == NULL || device == NULL)
    {
        return -1; // 参数错误
    }
    unsigned int i = 0;
    while (dev_info[i].name[0] != '\0')
    {
        if (strcmp(dev_info[i].name, name) == 0)
        {
            *device = &dev_info[i];
            return 0; // 找到设备
        }
        i++;
    }
    return -1; // 未找到设备
}
