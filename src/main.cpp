#include "h5.h"

#include <thread>
#include <future>
#include <mutex>

namespace {
std::mutex hdf5_mutex;
}

void singleThreadedWrite(){
    std::string directory = std::filesystem::temp_directory_path().string();
    std::string file_prefix = "test";
    auto h = H5FileWriter(directory, file_prefix);

    h.writeScalarToDataset("scalar", 3.14);
}

void multiThreadedWrite(int threads){
    
    if (threads < 1) {threads = 1;}

    std::vector<std::future<void>> futures;
    for(int i = 0; i < threads; i++){
        futures.push_back(std::async(std::launch::async, [](){
            // If your HDF5 build is not thread-safe, serialize HDF5 API calls.
            // This still lets workers do non-HDF5 work in parallel.
            std::lock_guard<std::mutex> lock(hdf5_mutex);

            std::string directory = std::filesystem::temp_directory_path().string();
            std::string file_prefix = "test";
            auto h = H5FileWriter(directory, file_prefix);
            h.writeScalarToDataset("scalar", 3.14);
        }));
    }

    for(auto& f : futures){
        f.get();
    }
}

int main(void){

    // HDF5 can be used from multiple threads, but only when either:
    // 1) your HDF5 library was built with thread-safety enabled, or
    // 2) you serialize all HDF5 API calls with a process-wide lock.

    singleThreadedWrite();
    std::cout << "Single threaded write complete" << std::endl;

    singleThreadedWrite();
    std::cout << "Second single threaded write complete" << std::endl;

    multiThreadedWrite(1);
    std::cout << "Single separate thread write complete" << std::endl;

    multiThreadedWrite(4);
    std::cout << "Multi threaded write complete" << std::endl;

    return 0;
}
