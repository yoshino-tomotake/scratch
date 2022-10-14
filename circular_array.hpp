#pragma once

#ifndef __JHC_CHANNEL__

#define __JHC_CHANNEL__

#include <mutex>

template<typename T,unsigned int size>
class circular_array
{
private:

    unsigned long read_loops;

    unsigned long write_loops;

    unsigned int read_count;

    unsigned int write_count;

    T buffer[size];

    std::mutex _mutex;

public:

    circular_array():read_loops(0),write_loops(0),read_count(0),write_count(0)
    {

    }

    void write(const T& data)
    {
        std::lock_guard<std::mutex> lock(_mutex);

        buffer[write_count] = data;
        write_count++;
        if(write_count == size)
        {
            write_count = 0;
            write_loops++;
        }
      
    }

    T* read()
    {
        std::lock_guard<std::mutex> lock(_mutex);

        T* data = &buffer[read_count];
        read_count++;
        if(read_count == size)
        {
            read_count = 0;
            read_loops++;
        }
        return  data;
    }


    int can_read()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if(write_loops == read_loops)
        {
            //如果圈数相等
            if(read_count <= write_count)
            {
                return write_count - read_count;
            }
            else
            {
                exit(-1);
            }
        }
        else if( static_cast<unsigned char>(write_loops) ==  static_cast<unsigned char>(read_loops + 1) )
        {
            return size - read_count;
        }
        else
        {
            exit(-1);

        }
    }

    bool can_write()
    {
        std::lock_guard<std::mutex> lock(_mutex);

        if(write_loops == read_loops)
        {
            return true;
        }
        else if( static_cast<unsigned char>(write_loops) == static_cast<unsigned char>(read_loops+1))
        {
            if(write_count < read_count)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            exit(-1);

        }
    }
};


#endif //__JHC_CHANNEL__