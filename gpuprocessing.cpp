
#include <Metal/Metal.hpp>
#include <iostream>

using namespace MTL;

class MetalContext {

    public:

        MetalContext()  {
            device = CreateSystemDefaultDevice();
        }

        ~MetalContext()  {
            device->release();
        }

        void setup(const char * library_location) {

            NS::Error* error;

            // create a device

            if (device == nullptr) {
                printf("No device found");
                return;
            }

            auto name = device->name()->utf8String();
            printf("gpu name: %\n", name);

            NS::String* filePath = NS::String::string(library_location, NS::UTF8StringEncoding);
            auto defaultLibrary = device->newLibrary(filePath, &error);
            if (!defaultLibrary) {
                std::cerr << "Failed to find the default library: " << error->localizedDescription()->utf8String() << std::endl;
                exit(-1);
            }
            
            printf("library loaded\n");

            auto functionName = NS::String::string("work_on_arrays", NS::ASCIIStringEncoding);
            auto computeFunction = defaultLibrary->newFunction(functionName);
            
            if(!computeFunction){
                std::cerr << "Failed to find the compute function.\n";
            }
            
            printf("function loaded\n");

            computePipelineState = device->newComputePipelineState(computeFunction, &error);
            printf("pipeline state created\n");
            // create a command queue
            commandQueue = device->newCommandQueue();
            if (commandQueue == nullptr) {
                printf("Failed to create a command queue");
                return;
            }

            printf("command queue created\n");
        }
        
        void doWork() {


            auto mBufferA = device->newBuffer(100, MTL::ResourceStorageModeShared);
            auto mBufferB = device->newBuffer(100, MTL::ResourceStorageModeShared);
            auto mBufferResult = device->newBuffer(100, MTL::ResourceStorageModeShared);
            
            printf("buffers created\n");

            MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
            assert(commandBuffer != nullptr);

            printf("command buffer created\n");

            MTL::ComputeCommandEncoder* computeEncoder = commandBuffer->computeCommandEncoder();
            computeEncoder->setComputePipelineState(computePipelineState);

            printf("compute encoder created\n");
            // Encode the pipeline state object and its parameters.
            computeEncoder->setBuffer(mBufferA, 0, 0);
            computeEncoder->setBuffer(mBufferB, 0, 1);
            computeEncoder->setBuffer(mBufferResult, 0, 2);
            
            printf("encodeComputeCommand \n");
            MTL::Size gridSize = MTL::Size(100, 1, 1);

            printf("encodeComputeCommand 2 \n");
            // Encode the compute command.
            computeEncoder->dispatchThreads(gridSize, MTL::Size(100, 1, 1));

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


            float* a = (float*) mBufferA->contents();
            float* b = (float*) mBufferB->contents();
            float* result = (float*) mBufferResult->contents();
                
            for(unsigned long int index = 0; index < 100; index++)
            {
                std::cout << a[index] << " + " << b[index] << " = " << result[index] << std::endl;
            }
            


        }




    private:
        MTL::Device * device;
        MTL::CommandQueue * commandQueue;
        MTL::CommandBuffer * commandBuffer;
        MTL::ComputeCommandEncoder * computeEncoder;
        MTL::ComputePipelineState * computePipelineState;
        MTL::Buffer *mBufferA;
        MTL::Buffer *mBufferB;
        MTL::Buffer *mBufferResult;

};




