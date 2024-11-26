
#include "h5.h"

// Define the static mutex
std::mutex H5FileWriter::mtx;

H5FileWriter::H5FileWriter(std::string& directory, std::string& file_prefix){
    std::lock_guard<std::mutex> lock(mtx);

    // Create property lists
    fcpl = H5Pcreate(H5P_FILE_CREATE);
    fapl = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_libver_bounds(fapl, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);

    // Create dataset property lists
    dcpl = H5Pcreate(H5P_DATASET_CREATE);
    dapl = H5Pcreate(H5P_DATASET_ACCESS);
    dxpl = H5Pcreate(H5P_DATASET_XFER);
    lcpl = H5Pcreate(H5P_LINK_CREATE);
    
    // Ensure the directory exists
    if (!std::filesystem::exists(directory)) {
        throw std::runtime_error("Directory does not exist: " + directory);
    }

    // Check if user has write permissions in directory
    std::error_code ec;
    std::filesystem::permissions(directory, std::filesystem::perms::owner_all, ec);
    if (ec) {
        throw std::runtime_error("No write permissions in directory: " + directory);
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1000000);
    std::string file_path = directory + "/" + file_prefix + "___" + std::to_string(dis(gen)) + ".h5";
    std::cout << "Generated file name: " << file_path << std::endl;

    this->file_path = file_path;
    file = H5Fcreate(file_path.c_str(), H5F_ACC_TRUNC, fcpl, fapl);
    if (file < 0) {
        throw std::runtime_error("Failed to create HDF5 file: " + file_path);
    }
}

H5FileWriter::~H5FileWriter() {
    std::lock_guard<std::mutex> lock(mtx);

    // Close datasets
    for (auto dataset : open_datasets) {
        H5Dclose(dataset);
    }
    H5Fclose(file);
    H5Pclose(fcpl);
    H5Pclose(fapl);
    H5Pclose(dcpl);
    H5Pclose(dxpl);
    H5Pclose(lcpl);
    H5Pclose(dapl);
}

void H5FileWriter::writeScalarToDataset(const std::string& name, double value) {
    std::lock_guard<std::mutex> lock(mtx);

    hsize_t dims[1] = {1};
    hid_t dataspace = H5Screate_simple(1, dims, nullptr);
    if (dataspace < 0) throw std::runtime_error("Failed to create dataspace for " + name);

    hid_t dataset = H5Dcreate(file, name.c_str(), H5T_NATIVE_DOUBLE, dataspace, lcpl, dcpl, dapl);
    if (dataset < 0) throw std::runtime_error("Failed to create dataset: " + name);

    H5Dwrite(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, dxpl, &value);
    H5Dclose(dataset);
    H5Sclose(dataspace);
};

void H5FileWriter::writeDictionaryOfScalarsToDataset(const std::string& name, const std::map<std::string, double>& values) {
    std::lock_guard<std::mutex> lock(mtx);

    for (auto& value : values) {
        writeScalarToDataset(name + "_" + value.first, value.second);
    }
};

void H5FileWriter::writeMatrixAxisToDataset(const std::string& name, const std::vector<double>& axis) {
    std::lock_guard<std::mutex> lock(mtx);

    hsize_t dims[1] = {axis.size()};
    hid_t dataspace = H5Screate_simple(1, dims, nullptr);
    if (dataspace < 0) throw std::runtime_error("Failed to create dataspace for " + name);

    hid_t dataset = H5Dcreate(file, name.c_str(), H5T_NATIVE_DOUBLE, dataspace, lcpl, dcpl, dapl);
    if (dataset < 0) throw std::runtime_error("Failed to create dataset: " + name);

    H5Dwrite(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, dxpl, axis.data());
    H5Dclose(dataset);
    H5Sclose(dataspace);
};

hid_t H5FileWriter::generate4DMatrix(const std::string& name, size_t N, size_t M, size_t O, size_t P) {
    std::lock_guard<std::mutex> lock(mtx);

    // Define the data space for the dataset
    hsize_t dims[4] = {N, M, O, P};
    hid_t dataspace = H5Screate_simple(4, dims, nullptr);
    if (dataspace < 0) {
        throw std::runtime_error("Failed to create dataspace for " + name);
    }

    // Create the dataset
    hid_t dataset = H5Dcreate(file, name.c_str(), H5T_NATIVE_DOUBLE, dataspace, lcpl, dcpl, dapl);
    if (dataset < 0) {
        H5Sclose(dataspace);
        throw std::runtime_error("Failed to create dataset: " + name);
    }

    // Fill the dataset with NaN values
    std::vector<double> nanBuffer(N * M * O * P, std::numeric_limits<double>::quiet_NaN());
    herr_t status = H5Dwrite(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, dxpl, nanBuffer.data());
    if (status < 0) {
        H5Dclose(dataset);
        H5Sclose(dataspace);
        throw std::runtime_error("Failed to initialize dataset with NaN values for " + name);
    }

    H5Sclose(dataspace);
    open_datasets.push_back(dataset);
    return dataset;
}

hid_t H5FileWriter::generate5DMatrix(const std::string& name, size_t N, size_t M, size_t O, size_t P, size_t Q) {
    std::lock_guard<std::mutex> lock(mtx);

    // Define the data space for the dataset
    hsize_t dims[5] = {N, M, O, P, Q};
    hid_t dataspace = H5Screate_simple(5, dims, nullptr);
    if (dataspace < 0) {
        throw std::runtime_error("Failed to create dataspace for " + name);
    }

    // Create the dataset
    hid_t dataset = H5Dcreate(file, name.c_str(), H5T_NATIVE_DOUBLE, dataspace, lcpl, dcpl, dapl);
    if (dataset < 0) {
        H5Sclose(dataspace);
        throw std::runtime_error("Failed to create dataset: " + name);
    }

    // Fill the dataset with NaN values
    std::vector<double> nanBuffer(N * M * O * P * Q, std::numeric_limits<double>::quiet_NaN());
    herr_t status = H5Dwrite(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, dxpl, nanBuffer.data());
    if (status < 0) {
        H5Dclose(dataset);
        H5Sclose(dataspace);
        throw std::runtime_error("Failed to initialize dataset with NaN values for " + name);
    }

    H5Sclose(dataspace);
    open_datasets.push_back(dataset);
    return dataset;
}

void H5FileWriter::writeTo4DMatrix(hid_t dataset, double value, int i, int j, int k, int l) {
    std::lock_guard<std::mutex> lock(mtx);

    hsize_t offset[4] = {static_cast<hsize_t>(i), static_cast<hsize_t>(j), static_cast<hsize_t>(k), static_cast<hsize_t>(l)};
    hsize_t count[4] = {1, 1, 1, 1};

    // Create a memory space
    hid_t memspace = H5Screate_simple(4, count, nullptr);
    if (memspace < 0) {
        throw std::runtime_error("Failed to create memory dataspace for writing value.");
    }

    // Select a hyperslab in the file dataspace
    hid_t filespace = H5Dget_space(dataset);
    if (filespace < 0) {
        H5Sclose(memspace);
        throw std::runtime_error("Failed to get filespace for dataset.");
    }

    herr_t status = H5Sselect_hyperslab(filespace, H5S_SELECT_SET, offset, nullptr, count, nullptr);
    if (status < 0) {
        H5Sclose(memspace);
        H5Sclose(filespace);
        throw std::runtime_error("Failed to select hyperslab for dataset.");
    }

    // Write the value to the selected hyperslab
    status = H5Dwrite(dataset, H5T_NATIVE_DOUBLE, memspace, filespace, dxpl, &value);
    if (status < 0) {
        H5Sclose(memspace);
        H5Sclose(filespace);
        throw std::runtime_error("Failed to write value to dataset.");
    }

    // Close resources
    H5Sclose(memspace);
    H5Sclose(filespace);
}

void H5FileWriter::writeTo5DMatrix(hid_t dataset, double value, int i, int j, int k, int l, int m) {
    std::lock_guard<std::mutex> lock(mtx);

    hsize_t offset[5] = {static_cast<hsize_t>(i), static_cast<hsize_t>(j), static_cast<hsize_t>(k), static_cast<hsize_t>(l), static_cast<hsize_t>(m)};
    hsize_t count[5] = {1, 1, 1, 1, 1};

    // Create a memory space
    hid_t memspace = H5Screate_simple(5, count, nullptr);
    if (memspace < 0) {
        throw std::runtime_error("Failed to create memory dataspace for writing value.");
    }

    // Select a hyperslab in the file dataspace
    hid_t filespace = H5Dget_space(dataset);
    if (filespace < 0) {
        H5Sclose(memspace);
        throw std::runtime_error("Failed to get filespace for dataset.");
    }

    herr_t status = H5Sselect_hyperslab(filespace, H5S_SELECT_SET, offset, nullptr, count, nullptr);
    if (status < 0) {
        H5Sclose(memspace);
        H5Sclose(filespace);
        throw std::runtime_error("Failed to select hyperslab for dataset.");
    }

    // Write the value to the selected hyperslab
    status = H5Dwrite(dataset, H5T_NATIVE_DOUBLE, memspace, filespace, dxpl, &value);
    if (status < 0) {
        H5Sclose(memspace);
        H5Sclose(filespace);
        throw std::runtime_error("Failed to write value to dataset.");
    }

    // Close resources
    H5Sclose(memspace);
    H5Sclose(filespace);
}


H5FileReader::H5FileReader(const std::string& file_path) {
    
    this->file_path = file_path;
    file = H5Fopen(file_path.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
    if (file < 0) {
        throw std::runtime_error("Failed to open HDF5 file: " + file_path);
    }
}

H5FileReader::~H5FileReader() {
    H5Fclose(file);
}

std::vector<double> H5FileReader::readMatrixAxisFromDataset(const std::string& name) {
    hid_t dataset = H5Dopen(file, name.c_str(), H5P_DEFAULT);
    if (dataset < 0) throw std::runtime_error("Failed to open dataset: " + name);

    hid_t dataspace = H5Dget_space(dataset);
    if (dataspace < 0) throw std::runtime_error("Failed to get dataspace for " + name);

    hsize_t dims[1];
    H5Sget_simple_extent_dims(dataspace, dims, nullptr);

    std::vector<double> axis(dims[0]);
    H5Dread(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, axis.data());

    H5Sclose(dataspace);
    H5Dclose(dataset);
    return axis;
}

double H5FileReader::readScalarFromDataset(const std::string& name) {
    hid_t dataset = H5Dopen(file, name.c_str(), H5P_DEFAULT);
    if (dataset < 0) throw std::runtime_error("Failed to open dataset: " + name);

    double value;
    H5Dread(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &value);
    H5Dclose(dataset);
    return value;
}

std::vector<std::vector<double>> H5FileReader::read2DSliceFromMatrix(const std::string& name, int i, int j) {
    std::vector<std::vector<double>> slice;

    // Open the dataset
    hid_t dataset = H5Dopen(file, name.c_str(), H5P_DEFAULT);
    if (dataset < 0) {
        throw std::runtime_error("Failed to open dataset: " + name);
    }

    // Get the dataspace
    hid_t dataspace = H5Dget_space(dataset);
    if (dataspace < 0) {
        H5Dclose(dataset);
        throw std::runtime_error("Failed to get dataspace for dataset: " + name);
    }

    // Get the dimensions of the dataset
    hsize_t dims_out[4];
    H5Sget_simple_extent_dims(dataspace, dims_out, nullptr);

    // Ensure i and j are within bounds
    if (i < 0 || i >= dims_out[2] || j < 0 || j >= dims_out[3]) {
        H5Sclose(dataspace);
        H5Dclose(dataset);
        throw std::out_of_range("Indices i or j are out of bounds");
    }

    // Prepare the output slice
    hsize_t slice_dims[2] = {dims_out[0], dims_out[1]};
    std::vector<double> buffer(slice_dims[0] * slice_dims[1]);

    // Define hyperslab
    hsize_t offset[4] = {0, 0, static_cast<hsize_t>(i), static_cast<hsize_t>(j)};
    hsize_t count[4] = {slice_dims[0], slice_dims[1], 1, 1};
    H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset, nullptr, count, nullptr);

    // Define memory space for the slice
    hid_t memspace = H5Screate_simple(2, slice_dims, nullptr);
    if (memspace < 0) {
        H5Sclose(dataspace);
        H5Dclose(dataset);
        throw std::runtime_error("Failed to create memory dataspace for reading");
    }

    // Read the hyperslab into the buffer
    H5Dread(dataset, H5T_NATIVE_DOUBLE, memspace, dataspace, H5P_DEFAULT, buffer.data());

    // Convert the 1D buffer into a 2D vector
    slice.resize(slice_dims[0], std::vector<double>(slice_dims[1]));
    for (size_t row = 0; row < slice_dims[0]; ++row) {
        std::copy(buffer.begin() + row * slice_dims[1],
                buffer.begin() + (row + 1) * slice_dims[1],
                slice[row].begin());
    }

    // Close resources
    H5Sclose(memspace);
    H5Sclose(dataspace);
    H5Dclose(dataset);

    return slice;
}

double H5FileReader::readPointFromMatrix(const std::string& name, int i, int j, int k, int l) {
    // Open the dataset
    hid_t dataset = H5Dopen(file, name.c_str(), H5P_DEFAULT);
    if (dataset < 0) {
        throw std::runtime_error("Failed to open dataset: " + name);
    }

    // Get the dataspace
    hid_t dataspace = H5Dget_space(dataset);
    if (dataspace < 0) {
        H5Dclose(dataset);
        throw std::runtime_error("Failed to get dataspace for dataset: " + name);
    }

    // Get the dimensions of the dataset
    hsize_t dims_out[4];
    H5Sget_simple_extent_dims(dataspace, dims_out, nullptr);

    // Ensure i, j, k, and l are within bounds
    if (i < 0 || i >= dims_out[0] || j < 0 || j >= dims_out[1] || k < 0 || k >= dims_out[2] || l < 0 || l >= dims_out[3]) {
        H5Sclose(dataspace);
        H5Dclose(dataset);
        throw std::out_of_range("Indices i, j, k, or l are out of bounds");
    }

    // Define hyperslab
    hsize_t offset[4] = {static_cast<hsize_t>(i), static_cast<hsize_t>(j), static_cast<hsize_t>(k), static_cast<hsize_t>(l)};
    hsize_t count[4] = {1, 1, 1, 1};
    H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset, nullptr, count, nullptr);

    // Define memory space for the point
    hid_t memspace = H5Screate_simple(1, count, nullptr);
    if (memspace < 0) {
        H5Sclose(dataspace);
        H5Dclose(dataset);
        throw std::runtime_error("Failed to create memory dataspace for reading");
    }

    // Read the hyperslab into the buffer
    double point;
    H5Dread(dataset, H5T_NATIVE_DOUBLE, memspace, dataspace, H5P_DEFAULT, &point);

    // Close resources
    H5Sclose(memspace);
    H5Sclose(dataspace);
    H5Dclose(dataset);

    return point;
}

double H5FileReader::readPointFromVector(const std::string& name, int i) {
    // Open the dataset
    hid_t dataset = H5Dopen(file, name.c_str(), H5P_DEFAULT);
    if (dataset < 0) {
        throw std::runtime_error("Failed to open dataset: " + name);
    }

    // Get the dataspace
    hid_t dataspace = H5Dget_space(dataset);
    if (dataspace < 0) {
        H5Dclose(dataset);
        throw std::runtime_error("Failed to get dataspace for dataset: " + name);
    }

    // Get the dimensions of the dataset
    hsize_t dims_out[1];
    H5Sget_simple_extent_dims(dataspace, dims_out, nullptr);

    // Ensure the index is within bounds
    if (i < 0 || i >= dims_out[0]) {
        H5Sclose(dataspace);
        H5Dclose(dataset);
        throw std::out_of_range("Index i is out of bounds");
    }

    // Define hyperslab
    hsize_t offset[1] = {static_cast<hsize_t>(i)};
    hsize_t count[1] = {1};  // Only one point at index i
    H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset, nullptr, count, nullptr);

    // Define memory space for the point
    hid_t memspace = H5Screate_simple(1, count, nullptr);
    if (memspace < 0) {
        H5Sclose(dataspace);
        H5Dclose(dataset);
        throw std::runtime_error("Failed to create memory dataspace");
    }

    // Read the hyperslab into the buffer
    double point;
    H5Dread(dataset, H5T_NATIVE_DOUBLE, memspace, dataspace, H5P_DEFAULT, &point);

    // Close resources
    H5Sclose(memspace);
    H5Sclose(dataspace);
    H5Dclose(dataset);

    return point;
}


double H5FileReader::getMinimumFromMatrix(const std::string& name) {
    double min_value = std::numeric_limits<double>::max();

    // Open the dataset
    hid_t dataset = H5Dopen(file, name.c_str(), H5P_DEFAULT);
    if (dataset < 0) {
        throw std::runtime_error("Failed to open dataset: " + name);
    }

    // Get the dataspace
    hid_t dataspace = H5Dget_space(dataset);
    if (dataspace < 0) {
        H5Dclose(dataset);
        throw std::runtime_error("Failed to get dataspace for dataset: " + name);
    }

    // Get the dimensions of the dataset
    hsize_t dims_out[4];
    H5Sget_simple_extent_dims(dataspace, dims_out, nullptr);

    // Prepare the output buffer
    size_t total_elements = dims_out[0] * dims_out[1] * dims_out[2] * dims_out[3];
    std::vector<double> buffer(total_elements);

    // Read the data into the buffer
    H5Dread(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer.data());

    // Find the minimum value in the buffer
    for (const auto& value : buffer) {
        if (value < min_value) {
            min_value = value;
        }
    }

    // Close resources
    H5Sclose(dataspace);
    H5Dclose(dataset);

    return min_value;
}