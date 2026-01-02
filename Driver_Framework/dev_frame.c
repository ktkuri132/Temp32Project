#include "dev_frame.h"
#include "df_log.h"
#include "df_init.h"
#include <string.h>

// ============ 错误码转字符串 ============
const char *df_err_to_str(df_err_t err)
{
    switch (err)
    {
    case DF_OK:
        return "Success";
    case DF_ERR_PARAM:
        return "Parameter error";
    case DF_ERR_NOT_FOUND:
        return "Device not found";
    case DF_ERR_TIMEOUT:
        return "Timeout";
    case DF_ERR_BUSY:
        return "Device busy";
    case DF_ERR_NO_MEM:
        return "Out of memory";
    case DF_ERR_HW_FAIL:
        return "Hardware failure";
    case DF_ERR_NOT_INIT:
        return "Not initialized";
    case DF_ERR_ALREADY:
        return "Already executed";
    case DF_ERR_NOT_SUPPORT:
        return "Not supported";
    default:
        return "Unknown error";
    }
}

// ============ 设备注册 ============
int df_dev_register(df_dev_t dev_info[])
{
    if (dev_info == NULL)
    {
        LOG_E("DEV", "df_dev_register: NULL parameter\n");
        return DF_ERR_PARAM;
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
            if (ret == DF_OK)
            {
                dev_info[i].status = DF_STATE_INITIALIZED;
                LOG_I("DEV", "Device '%s' initialized successfully\n", dev_info[i].name);
                success_count++;
            }
            else
            {
                dev_info[i].status = DF_STATE_ERROR;
                LOG_E("DEV", "Device '%s' initialization failed: %s\n",
                      dev_info[i].name, df_err_to_str(ret));
            }
        }
        else
        {
            dev_info[i].status = DF_STATE_UNINITIALIZED;
            LOG_W("DEV", "Device '%s' has no init function\n", dev_info[i].name);
        }
        i++;
    }

    LOG_I("DEV", "Device registration complete: %d/%d devices initialized\n",
          success_count, i);

    return DF_OK;
}

// ============ 设备查找 ============
int df_dev_find(df_dev_t dev_info[], const char *name, df_dev_t **device)
{
    if (name == NULL || device == NULL)
    {
        LOG_E("DEV", "df_dev_find: NULL parameter\n");
        return DF_ERR_PARAM;
    }

    unsigned int i = 0;
    while (dev_info[i].name[0] != '\0')
    {
        if (strcmp(dev_info[i].name, name) == 0)
        {
            *device = &dev_info[i];
            LOG_D("DEV", "Device '%s' found at index %d\n", name, i);
            return DF_OK;
        }
        i++;
    }

    LOG_W("DEV", "Device '%s' not found\n", name);
    return DF_ERR_NOT_FOUND;
}

// ============ 设备打开 ============
int df_dev_open(df_dev_t *device)
{
    if (device == NULL)
    {
        LOG_E("DEV", "df_dev_open: NULL device\n");
        return DF_ERR_PARAM;
    }

    if (device->status != DF_STATE_INITIALIZED &&
        device->status != DF_STATE_DISABLED)
    {
        LOG_E("DEV", "Device '%s' cannot be opened (status=%d)\n",
              device->name, device->status);
        return DF_ERR_NOT_INIT;
    }

    if (device->open != NULL)
    {
        int ret = device->open(device->arg);
        if (ret == DF_OK)
        {
            device->ref_count++;
            device->status = DF_STATE_ENABLED;
            LOG_I("DEV", "Device '%s' opened (ref_count=%d)\n",
                  device->name, device->ref_count);
        }
        return ret;
    }

    // 如果没有open函数，直接增加引用计数
    device->ref_count++;
    LOG_D("DEV", "Device '%s' opened without open() (ref_count=%d)\n",
          device->name, device->ref_count);
    return DF_OK;
}

// ============ 设备关闭 ============
int df_dev_close(df_dev_t *device)
{
    if (device == NULL)
    {
        LOG_E("DEV", "df_dev_close: NULL device\n");
        return DF_ERR_PARAM;
    }

    if (device->ref_count == 0)
    {
        LOG_W("DEV", "Device '%s' already closed\n", device->name);
        return DF_ERR_ALREADY;
    }

    device->ref_count--;

    if (device->ref_count == 0 && device->close != NULL)
    {
        int ret = device->close(device->arg);
        if (ret == DF_OK)
        {
            device->status = DF_STATE_DISABLED;
            LOG_I("DEV", "Device '%s' closed\n", device->name);
        }
        return ret;
    }

    LOG_D("DEV", "Device '%s' ref_count decreased to %d\n",
          device->name, device->ref_count);
    return DF_OK;
}

// ============ 设备启用 ============
int df_dev_enable(df_dev_t *device)
{
    if (device == NULL)
    {
        LOG_E("DEV", "df_dev_enable: NULL device\n");
        return DF_ERR_PARAM;
    }

    if (device->status == DF_STATE_ENABLED)
    {
        LOG_W("DEV", "Device '%s' already enabled\n", device->name);
        return DF_ERR_ALREADY;
    }

    if (device->enable != NULL)
    {
        int ret = device->enable(device->arg);
        if (ret == DF_OK)
        {
            device->status = DF_STATE_ENABLED;
            LOG_I("DEV", "Device '%s' enabled\n", device->name);
        }
        return ret;
    }

    device->status = DF_STATE_ENABLED;
    return DF_OK;
}

// ============ 设备禁用 ============
int df_dev_disable(df_dev_t *device)
{
    if (device == NULL)
    {
        LOG_E("DEV", "df_dev_disable: NULL device\n");
        return DF_ERR_PARAM;
    }

    if (device->status == DF_STATE_DISABLED)
    {
        LOG_W("DEV", "Device '%s' already disabled\n", device->name);
        return DF_ERR_ALREADY;
    }

    if (device->disable != NULL)
    {
        int ret = device->disable(device->arg);
        if (ret == DF_OK)
        {
            device->status = DF_STATE_DISABLED;
            LOG_I("DEV", "Device '%s' disabled\n", device->name);
        }
        return ret;
    }

    device->status = DF_STATE_DISABLED;
    return DF_OK;
}

// ============ 设备控制 ============
int df_dev_ioctl(df_dev_t *device, int cmd, df_arg_t arg)
{
    if (device == NULL)
    {
        LOG_E("DEV", "df_dev_ioctl: NULL device\n");
        return DF_ERR_PARAM;
    }

    if (device->ioctl == NULL)
    {
        LOG_E("DEV", "Device '%s' does not support ioctl\n", device->name);
        return DF_ERR_NOT_SUPPORT;
    }

    LOG_D("DEV", "Device '%s' ioctl cmd=0x%02X\n", device->name, cmd);
    return device->ioctl(cmd, arg);
}


