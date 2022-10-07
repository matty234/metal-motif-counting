
#include <iostream>
#include "gpuprocessing.cpp"

int main() {

    MetalContext d = MetalContext();
    d.setup("/Users/matthew/metal-motif-count/shaders/compute.metallib");

    d.doWork();
    return 0;

    
    
}
