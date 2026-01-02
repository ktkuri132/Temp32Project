#include "dev_frame.h"
#include "df_log.h"
#include "df_init.h"
#include <string.h>

// ============ 错误码转字符串 ============
const char *Device_Get_Error_String(dev_err_t err)
{
    switch (err)
    {
    case DEV_OK:
        return "Success";
    case DEV_ERR_PARAM:
        return "Parameter error";
    case DEV_ERR_NOT_FOUND:
        return "Device not found";
    case DEV_ERR_TIMEOUT:
        return "Timeout";
    case DEV_ERR_BUSY:
        return "Device busy";
    case DEV_ERR_NO_MEM:
        return "Out of memory";
    case DEV_ERR_HW_FAIL:
        return "Hardware failure";
    case DEV_ERR_NOT_INIT:
        return "Not initialized";
    case DEV_ERR_ALREADY:
        return "Already executed";
    case DEV_ERR_NOT_SUPPORT:
        return "Not supported";
    default:
        return "Unknown error";
    }
}

// ============ 设备注册 ============
int Device_Registration(dev_info_t dev_info[])
{
    if (dev_info == NULL)
    {
        LOG_E("DEV", "Device_Registration: NULL parameter\n");
        return DEV_ERR_PARAM;
    }

    unsigned int i = 0;
    int success_count = 0;

    while (dev_info[i].name[0] != '\0')
    {
        // 分配索引
        dev_info[i].index = i;
        dev_info[i].ref_count = 0;

        // 初始化设备状态
        if (dev_info[i].init != NULL)
        {
            int ret = dev_info[i].init(dev_info[i].arg);
            if (ret == DEV_OK)
            {
                dev_info[i].status = DEV_STATE_INITIALIZED;
                LOG_I("DEV", "Device '%s' initialized successfully\n", dev_info[i].name);
                success_count++;
            }
            else
            {
                dev_info[i].status = DEV_STATE_ERROR;
                LOG_E("DEV", "Device '%s' initialization failed: %s\n",
                      dev_info[i].name, Device_Get_Error_String(ret));
            }
        }
        else
        {
            dev_info[i].status = DEV_STATE_UNINITIALIZED;
            LOG_W("DEV", "Device '%s' has no init function\n", dev_info[i].name);
        }
        i++;
    }

    LOG_I("DEV", "Device registration complete: %d/%d devices initialized\n",
          success_count, i);

    return DEV_OK;
}

// ============ 设备查找 ============
int Find_Device(dev_info_t dev_info[], const char *name, dev_info_t **device)
{
    if (name == NULL || device == NULL)
    {
        LOG_E("DEV", "Find_Device: NULL parameter\n");
        return DEV_ERR_PARAM;
    }

    unsigned int i = 0;
    while (dev_info[i].name[0] != '\0')
    {
        if (strcmp(dev_info[i].name, name) == 0)
        {
            *device = &dev_info[i];
            LOG_D("DEV", "Device '%s' found at index %d\n", name, i);
            return DEV_OK;
        }
        i++;
    }

    LOG_W("DEV", "Device '%s' not found\n", name);
    return DEV_ERR_NOT_FOUND;
}

// ============ 设备打开 ============
int Device_Open(dev_info_t *device)
{
    if (device == NULL)
    {
        LOG_E("DEV", "Device_Open: NULL device\n");
        return DEV_ERR_PARAM;
    }

    if (device->status != DEV_STATE_INITIALIZED &&
        device->status != DEV_STATE_DISABLED)
    {
        LOG_E("DEV", "Device '%s' cannot be opened (status=%d)\n",
              device->name, device->status);
        return DEV_ERR_NOT_INIT;
    }

    if (device->open != NULL)
    {
        int ret = device->open(device->arg);
        if (ret == DEV_OK)
        {
            device->ref_count++;
            device->status = DEV_STATE_ENABLED;
            LOG_I("DEV", "Device '%s' opened (ref_count=%d)\n",
                  device->name, device->ref_count);
        }
        return ret;
    }

    // 如果没有open函数，直接增加引用计数
    device->ref_count++;
    LOG_D("DEV", "Device '%s' opened without open() (ref_count=%d)\n",
          device->name, device->ref_count);
    return DEV_OK;
}

// ============ 设备关闭 ============
int Device_Close(dev_info_t *device)
{
    if (device == NULL)
    {
        LOG_E("DEV", "Device_Close: NULL device\n");
        return DEV_ERR_PARAM;
    }

    if (device->ref_count == 0)
    {
        LOG_W("DEV", "Device '%s' already closed\n", device->name);
        return DEV_ERR_ALREADY;
    }

    device->ref_count--;

    if (device->ref_count == 0 && device->close != NULL)
    {
        int ret = device->close(device->arg);
        if (ret == DEV_OK)
        {
            device->status = DEV_STATE_DISABLED;
            LOG_I("DEV", "Device '%s' closed\n", device->name);
        }
        return ret;
    }

    LOG_D("DEV", "Device '%s' ref_count decreased to %d\n",
          device->name, device->ref_count);
    return DEV_OK;
}

// ============ 设备启用 ============
int Device_Enable(dev_info_t *device)
{
    if (device == NULL)
    {
        LOG_E("DEV", "Device_Enable: NULL device\n");
        return DEV_ERR_PARAM;
    }

    if (device->status == DEV_STATE_ENABLED)
    {
        LOG_W("DEV", "Device '%s' already enabled\n", device->name);
        return DEV_ERR_ALREADY;
    }

    if (device->enable != NULL)
    {
        int ret = device->enable(device->arg);
        if (ret == DEV_OK)
        {
            device->status = DEV_STATE_ENABLED;
            LOG_I("DEV", "Device '%s' enabled\n", device->name);
        }
        return ret;
    }

    device->status = DEV_STATE_ENABLED;
    return DEV_OK;
}

// ============ 设备禁用 ============
int Device_Disable(dev_info_t *device)
{
    if (device == NULL)
    {
        LOG_E("DEV", "Device_Disable: NULL device\n");
        return DEV_ERR_PARAM;
    }

    if (device->status == DEV_STATE_DISABLED)
    {
        LOG_W("DEV", "Device '%s' already disabled\n", device->name);
        return DEV_ERR_ALREADY;
    }

    if (device->disable != NULL)
    {
        int ret = device->disable(device->arg);
        if (ret == DEV_OK)
        {
            device->status = DEV_STATE_DISABLED;
            LOG_I("DEV", "Device '%s' disabled\n", device->name);
        }
        return ret;
    }

    device->status = DEV_STATE_DISABLED;
    return DEV_OK;
}

// ============ 设备控制 ============
int Device_Ioctl(dev_info_t *device, int cmd, dev_arg_t arg)
{
    if (device == NULL)
    {
        LOG_E("DEV", "Device_Ioctl: NULL device\n");
        return DEV_ERR_PARAM;
    }

    if (device->ioctl == NULL)
    {
        LOG_E("DEV", "Device '%s' does not support ioctl\n", device->name);
        return DEV_ERR_NOT_SUPPORT;
    }

    LOG_D("DEV", "Device '%s' ioctl cmd=0x%02X\n", device->name, cmd);
    return device->ioctl(cmd, arg);
}

// ============ 自动初始化 ============
/**
 * @brief 设备框架自动初始化函数
 * @details 在框架初始化时自动调用，初始化设备管理框架
 * @return 0表示成功
 */
static int df_device_frame_auto_init(void)
{
    // LOG_I("DEV", "Device framework initialized\n");
    return 0;
}

// 将设备框架初始化注册到PREV级别（在BOARD之后）
DF_INIT_EXPORT(df_device_frame_auto_init, DF_INIT_EXPORT_PREV);
