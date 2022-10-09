
#include <iostream>
#include "gpuprocessing.cpp"
#include <format>

int main(int argc, char *argv[]) {

    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <count of generated reads> <read length>" << std::endl;
        return 1;
    }

    // get int from command line
    int n = std::stoi(argv[1]);
    if (n < 1) {
        printf("Invalid number of reads: %d\n", n);
        exit(1);
    }



    // get int from command line
    int read_length = std::stoi(argv[2]);
    if (n < 1) {
        printf("Invalid number of read length: %d\n", n);
        exit(1);
    }

    MetalContext * context = new MetalContext();
    context->setup("./shaders/compute.metallib");

    ReadSource * readSource = new ReadSource();
    readSource->generate(n, read_length);
    context->doWork(readSource);
    return 0;
}
