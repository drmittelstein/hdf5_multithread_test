#include "h5.h"

#include <thread>
#include <future>

void singleThreadedWrite(){
    std::string directory = "C:/debug";
    std::string file_prefix = "test";
    auto h = H5FileWriter(directory, file_prefix);

    h.writeScalarToDataset("scalar", 3.14);
}

void multiThreadedWrite(){
    std::vector<std::future<void>> futures;
    for(int i = 0; i < 4; i++){
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
    singleThreadedWrite();
    std::cout << "Single threaded write complete" << std::endl;
    multiThreadedWrite();
    std::cout << "Multi threaded write complete" << std::endl;
    return 0;
}