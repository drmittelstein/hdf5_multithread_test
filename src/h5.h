#ifndef H5_H
#define H5_H

#include "hdf5.h"
#include <map>
#include <string>
#include <chrono>
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <filesystem>
#include <queue>


class H5FileWriter {

    public:

        H5FileWriter(std::string& directory, std::string& file_prefix);
        ~H5FileWriter();

        void writeScalarToDataset(const std::string& name, double value);
        void writeDictionaryOfScalarsToDataset(const std::string& name, const std::map<std::string, double>& values);
        void writeMatrixAxisToDataset(const std::string& name, const std::vector<double>& axis);

        hid_t generate4DMatrix(const std::string& name, size_t N, size_t M, size_t O, size_t P);
        void writeTo4DMatrix(hid_t dataset, double value, int i, int j, int k, int l);

        hid_t generate5DMatrix(const std::string& name, size_t N, size_t M, size_t O, size_t P, size_t Q);
        void writeTo5DMatrix(hid_t dataset, double value, int i, int j, int k, int l, int m);

    protected:
        hid_t file;
        std::string file_path;

        hid_t fcpl; // File creation property list
        hid_t fapl; // File access property list
        hid_t dcpl; // Dataset creation property list
        hid_t dapl; // Dataset access property list
        hid_t dxpl; // Dataset transfer property list
        hid_t lcpl; // Link creation property list

        std::vector<hid_t> open_datasets;
        
};

class H5FileReader {
    public:
        H5FileReader(const std::string& file_path);
        ~H5FileReader();

        std::vector<double> readMatrixAxisFromDataset(const std::string& name);
        double readScalarFromDataset(const std::string& name);
        std::vector<std::vector<double>> read2DSliceFromMatrix(const std::string& name, int i, int j);
        double readPointFromMatrix(const std::string& name, int i, int j, int k, int l);
        double readPointFromVector(const std::string& name, int i);
        double getMinimumFromMatrix(const std::string& name);

    protected:
        hid_t file;
        std::string file_path;
};

#endif // H5_H