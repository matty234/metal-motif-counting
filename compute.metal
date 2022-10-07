#include <metal_stdlib>
using namespace metal;


kernel void work_on_arrays(device const float* inA,
                           device const float* inB,
                           device float* result,
                           uint index [[thread_position_in_grid]])
{
    result[index] = 1;

    
}