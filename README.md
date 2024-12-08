## 简介

本项目是基于SWD协议的一个离线烧录程序，使用者只需要两个GPIO，即可完成对不同单片机的固件烧录，目前已经适配以下芯片：
- RK3588
- 算能 SG2002
- i.mx9352
- 树莓派3b
- 全志H6
- 全志H618

软件所使用的烧录算法使用Keil所使用的elf文件，里面包含了烧录需要使用的读写擦除等函数，用户可以通过命令行传递烧录算法文件所在路径来指定烧录算法，除此之外，该程序还支持将烧录算法内嵌到可执行程序中，用户只需要提供烧录算法的名称即可，它首先会从内嵌的烧录算法中寻找是否已经存在该算法，如果不存在该算法，将查找系统目录下是否存在该文件，然后开始进行烧录。

通过IO模拟SWD协议时，最大传输速率与IO速度息息相关，例如i.mx9352可以轻松达到20M的速率，而全志的IO速度较慢，在测试时最大速度不超过3M，会使整个烧录过程很漫长。

## 使用方式
软件目前支持flash与read两条指令，需要提供所使用的swdio和swdio引脚，以及所使用的烧录算法。

### 烧录

```
Programmer flash swclk swdio algorithm image <addr>
```
- Programmer 可执行程序名称。
- flash 执行烧录操作。
- swclk 引脚编号（引脚编号规则与 sysfs gpio 保持一致）。
- swdio 引脚编号（引脚编号规则与 sysfs gpio 保持一致）。
- algorithm 使用的算法文件。
- image 被烧录的程序。
- addr 当image为hex格式时，该参数无用，当文件为bin文件时，必须提供该地址。

### 读取内存

除了基本的烧录功能之外，该程序也支持读取操作，被读取的区域可以是外设寄存器，ram，以及内置的flash。

```
Programmer read swclk swdio addr length
```
- Programmer 可执行程序名称。
- read 执行读取操作。
- swclk 引脚编号（引脚编号规则与 sysfs gpio 保持一致）。
- swdio 引脚编号（引脚编号规则与 sysfs gpio 保持一致）。
- addr 被读取的地址。
- 读取的数据长度，单位为字节。

## 适配其他芯片

1. 通过Linux系统的sysfs访问gpio的速度较慢，需要长时间的等待，为了加快烧录的速率，需要根据不同芯片的技术参考手册中gpio的描述，实现GPIO的5个操作函数：
- void init(volatile void *base, uint32_t pin, const bool output)
- void write(volatile void *base, uint32_t pin, uint8_t output)
- void set(volatile void *base, uint32_t pin)
- void clear(volatile void *base, uint32_t pin)
- void toggle(volatile void *base, uint32_t pin)
- uint32_t read(volatile void *base, uint32_t pin)

2. 在完成上述函数的功能之后，再基于SWDIface实现一个SWD的子类，其中大部分函数均已实现，剩余的函数与硬件相关，实现细节可参考 ah618_swd.cpp，在改函数中首先会使用 SWD_IO_OPERATIONS_DEFINE 定义一组SWD操作相关的内联函数，SWD的IO操作相对密集，使用内联函数可以减少跳转提高运行速率，在SWD_IO_OPERATIONS_DEFINE 之后再使用 SWD_TRANSFER_DEFINE和SWJ_SEQUENCE_DEFINE 实现 swd_transfer 和 swj_sequence 函数，完成以上操作之后便可以开始实现子类了，需要被实现的函数如下：

- 构造函数（打开mmap并建立相关io的内存映射）
- 析构函数 (取消内存映射并关闭mmap)
- void msleep(uint32_t ms)
- bool init(void)
- bool off(void)
- transfer_err_def transer(uint32_t request, uint32_t *data)
- void swj_sequence(uint32_t count, const uint8_t *data)
- void set_target_reset(uint8_t asserted)

3. 查看系统中 /proc/device-tree/compatible 文件下的名称，并将其中一个属性添加到main.cpp 中的 swd_create 函数中，用于识别芯片类型选择对应的初始化函数。

```c
std::shared_ptr<SWDIface> swd_create(uint32_t swclk, uint32_t swdio)
{
    std::string temp;
    std::shared_ptr<SWDIface> swd;
    std::vector<std::string> compatible;
    std::ifstream file("/proc/device-tree/compatible");

    if (file.is_open())
    {
        if (getline(file, temp))
        {
            LOG_INFO("device %s", temp.c_str());
            compatible = split_string(temp, '\0');

            for (auto name : compatible)
            {
                if (name.compare("allwinner,h616") == 0)
                {
                    swd = std::make_shared<AH618SWD>(swclk, swdio, 2000000, true);
                    break;
                }
            }
        }

        file.close();
        if (!swd)
        {
            throw std::runtime_error("Unsupported device types " + temp);
        }
    }
    else
    {
        throw std::runtime_error("Unable to open /proc/device-tree/compatible");
    }

    return swd;
}

```