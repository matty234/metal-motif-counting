
#include <Metal/Metal.hpp>
#include <iostream>
#include "readsource.cpp"

using namespace MTL;

class MetalContext
{

public:
    MetalContext()
    {

        device = CreateSystemDefaultDevice();

        captureManager = CaptureManager::sharedCaptureManager();

        const NS::String *output_url = NS::String::string("file://./capture", NS::UTF8StringEncoding);
        CaptureDescriptor *captureDescriptor = CaptureDescriptor::alloc()->init();
        captureDescriptor->setCaptureObject(device);
        captureDescriptor->setDestination(CaptureDestination::CaptureDestinationGPUTraceDocument);
        captureDescriptor->setOutputURL(NS::URL::alloc()->fileURLWithPath(output_url));
        NS::Error *error;
        captureManager->startCapture(captureDescriptor, &error);
        if (error != nullptr)
        {
            std::cout << "Error starting capture: " << error->localizedDescription()->utf8String() << std::endl;
        }
    }

    ~MetalContext()
    {

        captureManager->stopCapture();

        device->release();
    }

    void setup(const char *library_location)
    {

        NS::Error *error;

        // create a device

        if (device == nullptr)
        {
            printf("No device found");
            return;
        }

        auto name = device->name()->utf8String();
        printf("gpu name: %s\n", name);

        NS::String *filePath = NS::String::string(library_location, NS::UTF8StringEncoding);
        auto defaultLibrary = device->newLibrary(filePath, &error);
        if (!defaultLibrary)
        {
            std::cerr << "Failed to find the default library: " << error->localizedDescription()->utf8String() << std::endl;
            exit(-1);
        }

        printf("library loaded\n");

        auto functionName = NS::String::string("find_motifs_in_encoded_sequnce", NS::ASCIIStringEncoding);
        auto computeFunction = defaultLibrary->newFunction(functionName);

        if (!computeFunction)
        {
            std::cerr << "Failed to find the compute function.\n";
        }

        printf("function loaded\n");

        computePipelineState = device->newComputePipelineState(computeFunction, &error);
        printf("pipeline state created\n");
        // create a command queue
        commandQueue = device->newCommandQueue();
        if (commandQueue == nullptr)
        {
            printf("Failed to create a command queue");
            return;
        }

        printf("command queue created\n");
    }

    void doWork(ReadSource * reads)
    {

        const int max_read_length = 1000;

        const char *motifs_to_find = "ATCTCACTAATTATGGGCTAATTATTTAGG";
        int local_motif_length = 5;
        int num_motifs = strlen(motifs_to_find) / local_motif_length;

        // we create one thread per read and group reads per motif size

        // max read length is 1000
        int buffer_encoded_sequence_length = max_read_length * sizeof(char) * reads->num_reads;
        int buffer_sequence_length_length = sizeof(int) * reads->num_reads;
        int buffer_encoded_motifs_length = sizeof(char) * strlen(motifs_to_find);
        int buffer_motif_length_length = sizeof(int);
        int buffer_motif_count_length = sizeof(int);

        int buffer_result_length = sizeof(bool) * num_motifs * max_read_length * reads->num_reads;

        auto mBuffer_encoded_sequence = device->newBuffer(buffer_encoded_sequence_length, MTL::ResourceStorageModeShared);
        auto mBuffer_sequence_length = device->newBuffer(buffer_sequence_length_length, MTL::ResourceStorageModeShared);
        auto mBuffer_encoded_motifs = device->newBuffer(buffer_encoded_motifs_length, MTL::ResourceStorageModeShared);
        auto mBuffer_motif_length = device->newBuffer(buffer_motif_length_length, MTL::ResourceStorageModeShared);
        auto mBuffer_motif_count = device->newBuffer(buffer_motif_count_length, MTL::ResourceStorageModeShared);

        auto mBuffer_result = device->newBuffer(buffer_result_length, MTL::ResourceStorageModeShared);

        // copy the reads into the buffer
        char *encoded_sequence = (char *)mBuffer_encoded_sequence->contents();
        int *sequence_length = (int *)mBuffer_sequence_length->contents();
        char *encoded_motifs = (char *)mBuffer_encoded_motifs->contents();
        int *motif_length = (int *)mBuffer_motif_length->contents();
        int *motif_count = (int *)mBuffer_motif_count->contents();

        for (int i = 0; i < reads->num_reads; i++)
        {

            Read read = reads->getReads()[i];

            sequence_length[i] = read.length;
            memcpy(encoded_sequence + i * 1000, read.bases, read.length);
        }

        memcpy(encoded_motifs, motifs_to_find, strlen(motifs_to_find));
        *motif_length = local_motif_length;
        *motif_count = num_motifs;

        printf("buffers created\n");

        MTL::CommandBuffer *commandBuffer = commandQueue->commandBuffer();
        assert(commandBuffer != nullptr);

        printf("command buffer created\n");

        MTL::ComputeCommandEncoder *computeEncoder = commandBuffer->computeCommandEncoder();
        computeEncoder->setComputePipelineState(computePipelineState);

        printf("compute encoder created\n");
        // Encode the pipeline state object and its parameters.
        computeEncoder->setBuffer(mBuffer_encoded_sequence, 0, 0);
        computeEncoder->setBuffer(mBuffer_sequence_length, 0, 1);
        computeEncoder->setBuffer(mBuffer_encoded_motifs, 0, 2);
        computeEncoder->setBuffer(mBuffer_motif_length, 0, 3);
        computeEncoder->setBuffer(mBuffer_motif_count, 0, 4);
        computeEncoder->setBuffer(mBuffer_result, 0, 5);

        printf("encodeComputeCommand \n");
        MTL::Size grid_size = MTL::Size(reads->num_reads, 1, 1);

        NS::UInteger _thread_group_size = computePipelineState->maxTotalThreadsPerThreadgroup();
        if (_thread_group_size > reads->num_reads)
        {
            _thread_group_size = reads->num_reads;
        }
    

        MTL::Size thread_group_size = MTL::Size(_thread_group_size, 1, 1);

        printf("encodeComputeCommand 2 \n");
        // Encode the compute command.
        computeEncoder->dispatchThreads(grid_size, thread_group_size);

        printf("encodeComputeCommand 3 \n");
        computeEncoder->endEncoding();

        printf("compute command encoder ended\n");
        // Execute the command.
        commandBuffer->commit();

        printf("command buffer committed\n");
        // Normally, you want to do other work in your app while the GPU is running,
        // but in this example, the code simply blocks until the calculation is complete.
        commandBuffer->waitUntilCompleted();

        printf("command buffer completed\n");

        bool *result = (bool *)mBuffer_result->contents();
        for (int i = 0; i < reads->num_reads; i++)
        {
            Read read = reads->getReads()[i];
            printf("result %s: ", read.bases);
            for (int j = 0; j < read.length; j++)
            {
                for (int k = 0; k < num_motifs; k++)
                {
                    if (result[i * max_read_length * num_motifs + j * num_motifs + k])
                    {
                        char *motif = (char *)malloc(local_motif_length + 1);
                        memcpy(motif, motifs_to_find + k * local_motif_length, local_motif_length);
                        printf("%s @ %d, ", motif, j);
                    }
                }
            }

            printf("\n");
        }
    }

private:
    MTL::Device *device;
    MTL::CommandQueue *commandQueue;
    MTL::CommandBuffer *commandBuffer;
    MTL::ComputeCommandEncoder *computeEncoder;
    MTL::ComputePipelineState *computePipelineState;
    MTL::Buffer *mBufferA;
    MTL::Buffer *mBufferB;
    MTL::Buffer *mBufferResult;
    CaptureManager *captureManager;
};
