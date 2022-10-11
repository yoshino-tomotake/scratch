#include <iostream>
#include <vector>
#include <stdlib.h>
#include <mutex>
#include <fcntl.h>
#include <unistd.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <chrono>
#include <thread>

class spi_manager
{

private:
    int fd;

    std::mutex _mu;

public:

    spi_manager():fd(-1)
    {

    }
    
    /**
     * @brief 打开spi设备
     * 
     * @param device_name 设备名 
     * @return int 成功返回0,失败返回负数
     */
    int open_device(const std::string& device_name)
    {

        std::lock_guard<std::mutex> lock(_mu);

        if(is_opened())
        {
            std::cout << "device is opened.\n";
            return -1;
        }

        fd = open(device_name.c_str(),O_RDWR);
        
        if(fd < 0)
        {
            std::cout << "open spi device failed\n";
            return -1;
        }

        std::cout << "the spi dev fd is " << fd << std::endl;;


        return 0;
    }
    
    /**
     * @brief 关闭设备，成功返回0,失败返回负数
     * 
     */
    void close_device()
    {
        std::lock_guard<std::mutex> lock(_mu);
        
        if(fd >= 0)
        {
            close(fd);
        }

        fd = -1;
    }

    /**
     * @brief 设备是否打开
     * 
     * @return true 设备已经打开
     * @return false 设备未打开
     */
    bool is_opened()
    {
        return fd < 0 ? false : true;
    }

    /**
     * @brief 配置spi
     * 
     * @param bits 
     * @param max_speed 
     * @param mode 
     * @return int 
     */
    int config_spi(int bits,int max_speed,int mode)
    {
        std::lock_guard<std::mutex> lock(_mu);
        if(fd < 0)
        {
            std::cout << "device is not opened,cannot config spi.\n";
            return -1;
        }
        
        //设置极性和相位
        if (ioctl (fd, SPI_IOC_WR_MODE, &mode) < 0)
        {
            std::cout << "config mode failed.\n";
            return -2;
        }

        //设置数据位
        if (ioctl (fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0)
        {
            std::cout << "config bits failed.\n";
            return -3;
        }

        //设置最大传输速度
        if (ioctl (fd, SPI_IOC_WR_MAX_SPEED_HZ, &max_speed)  < 0)
        {
            std::cout << "config max_speed failed.\n";
            return -4;
        }

        return 0;
    }

    /**
     * @brief 从spi读取数据
     * 
     * @param buffer 缓冲区起始地址
     * @param nbytes 字节数量
     * @return int -2表示设备没有打开，其他返回值的含义等同于read函数
     */
    int read_data(void* buffer,size_t nbytes)
    {

        std::lock_guard<std::mutex> lock(_mu);

        if(fd < 0)
        {
            std::cout << "device is not opened,can not read. \n";
            return -2;
        }
        int ret =  read(fd,buffer,nbytes);

        if(ret <= 0)
        {
            std::cout  << "read spi device failed.\n";
            return -1;
        }
        
        return 0;
    }

    /**
     * @brief 写数据到spi
     * 
     * @param buffer 数据缓冲区的起始地质
     * @param nbytes 字节数量
     * @return int -2表示设备没有打开，其他返回值的含义等同于write函数
     */
    int write_data(void* buffer,size_t nbytes)
    {
        std::lock_guard<std::mutex> lock(_mu);

        if(fd < 0)
        {
            std::cout << "device is not opened,can not write.\n"; 
            return -2;
        }

        int ret =  write(fd,buffer,nbytes);

        if(ret < 0)
        {
            std::cout <<"write spi device failed.\n";
        }

        return 0;
    }

    /**
     * @brief 删除拷贝构造期
     * 
     * @param s 
     */
    spi_manager(const spi_manager& s)=delete;

    /**
    * @brief 删除拷贝赋值运算符
    * 
    * @param s 
    * @return spi_manager& 
    */
    spi_manager& operator=(const spi_manager& s) =delete;
    
    /**
     * @brief 删除移动构造器
     * 
     * @param s 
     */
    spi_manager(spi_manager&& s)  = delete;

    /**
     * @brief 删除移动赋值运算符
     * 
     * @param s 
     * @return spi_manager& 
     */
    spi_manager& operator=(spi_manager&& s)  =delete;

    /**
     * @brief Destroy the spi manager object
     * 
     */
    virtual ~spi_manager()
    {
        close_device();
    }

};

int main(int argc,char* argv[])
{

    spi_manager spi;
    spi.open_device("/dev/spidev0.0");
    spi.config_spi(8,16*1000000,3);
    
    constexpr long footLen = 8;
    unsigned char cmdData[footLen]{0XFF,0xFF,0x00,0x08,0x00,0x08,0x00,0x00};

    int crc = 0;

    for(int i =0;i < footLen - 2;++i)
    {
        crc ^= cmdData[i];
    }

    cmdData[footLen - 2] = (crc >> 8 ) & 0xFF;
    cmdData[footLen - 1] = crc & 0xFF;

    spi.write_data(cmdData,footLen);

    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    constexpr long inlen = 32;
    unsigned char inData[inlen];

    spi.read_data(inData,inlen);

    for(int i = 0;i<inlen ;++i)
    {
        std::cout << inData[i];
    }
    return 0;
}
