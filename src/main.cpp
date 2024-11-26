#include "h5.h"

#include <thread>
#include <future>

void singleThreadedWrite(){
    std::string directory = "C:/debug";
    std::string file_prefix = "test";
    auto h = H5FileWriter(directory, file_prefix);

    h.writeScalarToDataset("scalar", 3.14);
}

void multiThreadedWrite(int threads){
    
    if (threads < 1) {threads = 1;}

    std::vector<std::future<void>> futures;
    for(int i = 0; i < threads; i++){
        futures.push_back(std::async(std::launch::async, [](){
            std::string directory = "C:/debug";
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

    // If I build with:
    // set(HDF5_BUILD_FROM_SOURCE ON CACHE BOOL "" FORCE)
    // then, I get the following error at initialization:
    // The program '[19848] hdf5_multithread_test.exe' has exited with code -1073741515 (0xc0000135).

    // If I build with:
    // set(HDF5_BUILD_FROM_SOURCE OFF CACHE BOOL "" FORCE)
    // then the following lines run without error:

    singleThreadedWrite();
    std::cout << "Single threaded write complete" << std::endl;

    singleThreadedWrite();
    std::cout << "Second single threaded write complete" << std::endl;

    multiThreadedWrite(1);
    std::cout << "Single separate thread write complete" << std::endl;

    multiThreadedWrite(4);
    // But, an error occurs here:
    // Exception thrown at 0x00007FFD70116A44 (hdf5.dll) in hdf5_multithread_test.exe: 0xC0000005: Access violation reading location 0xFFFFFFFFFFFFFFFF.
    // Exception thrown at 0x00007FFD7011F9B7 (hdf5.dll) in hdf5_multithread_test.exe: 0xC0000005: Access violation reading location 0x0000000000000000.
    // HDF5-DIAG: Error detected in HDF5 (1.14.5):
    //   #000: D:\a\hdf5\hdf5\hdf5-1.14.5\src\H5F.c line 653 in H5Fcreate(): unable to synchronously create file
    //     major: File accessibility
    //     minor: Unable to create file
    //   #001: D:\a\hdf5\hdf5\hdf5-1.14.5\src\H5F.c line 608 in H5F__create_api_common(): unable to create file
    //     major: File accessibility
    //     minor: Unable to open file
    //   #002: D:\a\hdf5\hdf5\hdf5-1.14.5\src\H5VLcallback.c line 3445 in H5VL_file_create(): file create failed
    //     major: Virtual Object Layer
    //     minor: Unable to create file
    //   #003: D:\a\hdf5\hdf5\hdf5-1.14.5\src\H5VLcallback.c line 3411 in H5VL__file_create(): file create failed
    //     major: Virtual Object Layer
    //     minor: Unable to create file
    //   #004: D:\a\hdf5\hdf5\hdf5-1.14.5\src\H5VLnative_file.c line 94 in H5VL__native_file_create(): unable to create file
    //     major: File accessibility
    //     minor: Unable to open file
    //   #005: D:\a\hdf5\hdf5\hdf5-1.14.5\src\H5Fint.c line 1974 in H5F_open(): unable to initialize file structure
    //     major: File accessibility
    //     minor: Unable to open file
    std::cout << "Multi threaded write complete" << std::endl;

    return 0;
}