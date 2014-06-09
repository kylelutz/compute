//---------------------------------------------------------------------------//
// Copyright (c) 2013-2014 Mageswaran.D <mageswaran1989@gmail.com>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// See http://kylelutz.github.com/compute for more information.
//---------------------------------------------------------------------------//

// This sample can be used to visualize the kernel internal global/local IDs.
//              Global               Local
//3.03   -> "00"th row "03"rd col."00"th row "03"rd col
//804.00 -> "08"th row "04"th col."00"th row "00"th col
//1111.33-> "11"th row "11"th col."03"rd row "03"rd col

#include <iostream>
#include <boost/compute.hpp>

namespace compute = boost::compute;

const char kernel_with_local_threads[] =
        BOOST_COMPUTE_STRINGIZE_SOURCE(
            __kernel void print_id(__global float *host_ptr)
            {
                int gx = get_global_id(0);
                int gy = get_global_id(1);
                int bx = get_group_id (0);
                int by = get_group_id (1);
                int lx = get_local_id (0);
                int ly = get_local_id (1);
                int kernel_width = get_global_size(0);

                host_ptr[gy * kernel_width + gx] = gx * 100 + gy * 1 +
                                                   lx * 0.1 + ly * 0.01;
            }
            );

int main(int argc, char *argv[])
{
    if(argc < 5)
    {
        std::cout<<"Usage: "<< argv[0]
                <<" NumGlobalThreadsX "
                <<" NumGlobalThreadsY "
                <<" NumLocalThreadsX  "
                <<" NumLocalThreadsY  "
                <<std::endl;
        std::cout<<"Eg:"<<argv[0]<<" 8 8 4 4"<<std::endl;
        std::cout<<"Note: Make sure your output window is large enough!"
                 <<std::endl;
        exit(0);
    }


    compute::device dev = compute::system::default_device();
    compute::context dev_context(dev);
    compute::command_queue dev_queue(dev_context, dev);

    compute::program visualize_program =
            compute::program::create_with_source(
                kernel_with_local_threads, dev_context);
    try
    {
        visualize_program.build();
    }
    catch(boost::compute::opencl_error &e)
    {
         std::cout << visualize_program.build_log() << std::endl;

    }

    compute::kernel visualize_kernel(visualize_program, "print_id");

    compute::buffer* dev_ptr;
    float* host_ptr;
    int g_x = atoi(argv[1]), g_y = atoi(argv[2]);
    int l_x = atoi(argv[3]), l_y = atoi(argv[4]);

    std::cout<<"Number of groups created / grid size: "<<g_x/l_x<< " x "<<g_y/l_y
             <<std::endl
             <<"Number of local threads / local size: "<<l_x<<" x "<<l_y
             <<std::endl
             <<"Number of global threads: "<<g_x<<" x "<<g_y
             <<std::endl;

    host_ptr = new float[g_x * g_y];
    dev_ptr = new compute::buffer(dev_context, sizeof(float) * g_x * g_y,
                              compute::memory_object::write_only,
                              host_ptr);
    visualize_kernel.set_arg(0, *dev_ptr);

    const size_t global_thread_size[2] = {g_x, g_y};
    const size_t local_thread_size[2] = {l_x, l_y};

    dev_queue.enqueue_nd_range_kernel(visualize_kernel,
                                      2,
                                      0,
                                      global_thread_size,
                                      local_thread_size);

    dev_queue.enqueue_read_buffer(*dev_ptr,
                                  0,
                                  sizeof(float) * g_x * g_y,
                                  host_ptr);

    std::cout<<std::endl
             <<"GlobalIdXY.LocalIdXY"
             <<std::endl
             <<"    g_xg_y.lxl_y"
             <<std::endl
             <<"------------------------------------"
             <<std::endl
             <<std::endl;

    for(int gj = 0; gj < g_y; gj++) {
        for(int gi=0; gi < g_x; gi++) {
            printf("%8.2f", host_ptr[gj * g_x + gi]);
            if(((gi + 1) % l_x) == 0)
                printf(" | ");
        }
        if(((gj + 1) % l_y) == 0)
            printf("\n");
        printf("\n");
    }

    std::cout<<"!!! Zeros pre to the number are ignored while printing !!!"
             <<std::endl;
    return 0;
}
