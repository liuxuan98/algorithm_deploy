/**
 * @file main.cpp
 * @brief Pack Models Tool main entry point
 * @copyright (c) .
 */

#include "cli_parser.h"
#include "model_serializer.h"
#include "model_packager.h"
#include "../include/utils/file_utils.h"
#include "utils/platform_utils.h"
#include "utils/codec/auto_crypto.h"
#include "utils/codec/model_parse.h"

// Include model headers for type checking
#ifdef ENABLE_MNN_MODEL
#include "model/mnn/mnn_model.h"
#endif

#ifdef ENABLE_OPENVINO_MODEL
#include "model/openvino/openvino_model.h"
#endif

#ifdef ENABLE_ONNX_MODEL
#include "model/onnx/onnx_model.h"
#endif

#ifdef ENABLE_TENSORRT_MODEL
#include "model/tensorrt/tensorrt_model.h"
#endif

#include <iostream>
#include <chrono>

using namespace rayshape::tools;
using namespace rayshape::tools::utils;

/**
 * @brief Print verbose message if verbose mode is enabled
 */
void PrintVerbose(bool verbose, const std::string &message) {
    if (verbose) {
        std::cout << "[INFO] " << message << std::endl;
    }
}

/**
 * @brief Print warning message
 */
void PrintWarning(const std::string &message) {
    std::cout << "[WARNING] " << message << std::endl;
}


/**
 * @brief Print error message
 */
void PrintError(const std::string &message) {
    std::cerr << "[ERROR] " << message << std::endl;
}

/**
 * @brief Print success message
 */
void PrintSuccess(const std::string &message) {
    std::cout << "[SUCCESS] " << message << std::endl;
}

/**
 * @brief Check if auto-encryption is available and enabled
 */
bool IsAutoEncryptionEnabled(const CommandLineArgs &args) {
    if (!args.enable_encryption) {
        return false;
    }

    if (!rayshape::utils::codec::AutoCrypto::IsAvailable()) {
        PrintError("Auto-encryption is not available (missing CryptoPP library)");
        return false;
    }

    return true;
}

/**
 * @brief Execute serialize command
 */
int ExecuteSerialize(const CommandLineArgs &args) {
    PrintVerbose(args.verbose, "Starting model serialization...");

    // Check if auto-encryption is enabled
    bool auto_encrypt = IsAutoEncryptionEnabled(args);
    if (args.enable_encryption && !auto_encrypt) {
        return 1; // Error already printed by IsAutoEncryptionEnabled
    }

    if (auto_encrypt) {
        PrintVerbose(args.verbose, "Auto-encryption enabled for output file");
    }

    ModelSerializer serializer;
    bool success = false;

    auto start_time = std::chrono::high_resolution_clock::now();

    switch (args.model_type) {
    case rayshape::ModelType::MNN:
        PrintVerbose(args.verbose, "Serializing MNN model: " + args.input_path);
        PrintVerbose(args.verbose, "Using config file: " + args.config_path);
        success = serializer.SerializeMnnModel(args.input_path, args.config_path, args.output_path, auto_encrypt);
        break;

    case rayshape::ModelType::OPENVINO:
        PrintVerbose(args.verbose,
                     "Serializing OpenVINO model: " + args.xml_path + ", " + args.bin_path);
        PrintVerbose(args.verbose, "Using config file: " + args.config_path);
        success = serializer.SerializeOpenVINOModel(args.xml_path, args.bin_path, args.config_path,
                                                    args.output_path, auto_encrypt);
        break;

    case rayshape::ModelType::ONNX:
        PrintVerbose(args.verbose, "Serializing ONNX model: " + args.input_path);
        PrintVerbose(args.verbose, "Using config file: " + args.config_path);
        success = serializer.SerializeONNXModel(args.input_path, args.config_path, args.output_path, auto_encrypt);
        break;

    case rayshape::ModelType::TENSORRT:
        PrintVerbose(args.verbose, "Serializing TensorRT model: " + args.input_path);
        PrintVerbose(args.verbose, "Using config file: " + args.config_path);
        success = serializer.SerializeTensorRTModel(args.input_path, args.config_path, args.output_path, auto_encrypt);
        break;

    default:
        PrintError("Unsupported model type for serialization");
        return 1;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    if (success) {
        PrintSuccess("Model serialized successfully to: " + args.output_path);
        PrintVerbose(args.verbose,
                     "Serialization completed in " + std::to_string(duration.count()) + "ms");

        // Show file size information
        if (args.verbose) {
            long long output_size =
                rayshape::tools::utils::FileUtils::GetFileSize(args.output_path);
            if (output_size > 0) {
                std::cout << "[INFO] Output file size: " << output_size << " bytes" << std::endl;
            }
        }

        return 0;
    } else {
        PrintError("Failed to serialize model: " + serializer.GetLastError());
        return 1;
    }
}

/**
 * @brief Execute package command
 */
int ExecutePackage(const CommandLineArgs &args) {
    PrintVerbose(args.verbose, "Starting model packaging...");

    // Check if auto-encryption is enabled
    bool auto_encrypt = IsAutoEncryptionEnabled(args);
    if (args.enable_encryption && !auto_encrypt) {
        return 1; // Error already printed by IsAutoEncryptionEnabled
    }

    if (auto_encrypt) {
        PrintVerbose(args.verbose, "Auto-encryption enabled for package");
    }

    // Prepare input files list
    std::vector<std::string> input_files = args.input_files;
    if (!args.input_path.empty()) {
        input_files.push_back(args.input_path);
    }

    if (input_files.empty()) {
        PrintError("No input files specified for packaging");
        return 1;
    }

    // Prepare package info
    ModelPackageInfo package_info;
    package_info.package_name = args.package_name;
    package_info.version = args.version.empty() ? "1.0.0" : args.version;
    package_info.description = args.description;
    package_info.model_type = args.model_type;
    package_info.model_files = input_files;
    package_info.metadata = args.metadata;

    PrintVerbose(args.verbose, "Package name: " + package_info.package_name);
    PrintVerbose(args.verbose, "Package version: " + package_info.version);
    PrintVerbose(args.verbose, "Input files count: " + std::to_string(input_files.size()));

    ModelPackager packager;
    packager.SetCompressionLevel(args.compression_level);

    auto start_time = std::chrono::high_resolution_clock::now();

    bool success = packager.CreatePackage(package_info, input_files, args.output_path, auto_encrypt);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    if (success) {
        PrintSuccess("Package created successfully: " + args.output_path);
        PrintVerbose(args.verbose,
                     "Packaging completed in " + std::to_string(duration.count()) + "ms");

        // Show package size information
        if (args.verbose) {
            long long package_size =
                rayshape::tools::utils::FileUtils::GetFileSize(args.output_path);
            if (package_size > 0) {
                std::cout << "[INFO] Package size: " << package_size << " bytes" << std::endl;
            }
        }

        return 0;
    } else {
        PrintError("Failed to create package: " + packager.GetLastError());
        return 1;
    }
}

/**
 * @brief Execute extract command
 */
int ExecuteExtract(const CommandLineArgs &args) {
    PrintVerbose(args.verbose, "Starting package extraction...");
    PrintVerbose(args.verbose, "Input package: " + args.input_path);
    PrintVerbose(args.verbose, "Output directory: " + args.output_path);

    // Auto-decryption handles encrypted packages automatically

    ModelPackager packager;

    auto start_time = std::chrono::high_resolution_clock::now();

    bool success = packager.ExtractPackage(args.input_path, args.output_path);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    if (success) {
        PrintSuccess("Package extracted successfully to: " + args.output_path);
        PrintVerbose(args.verbose,
                     "Extraction completed in " + std::to_string(duration.count()) + "ms");
        return 0;
    } else {
        PrintError("Failed to extract package: " + packager.GetLastError());
        return 1;
    }
}

/**
 * @brief Execute list command
 */
int ExecuteList(const CommandLineArgs &args) {
    PrintVerbose(args.verbose, "Listing package contents: " + args.input_path);

    // Get password if provided (may be needed for encrypted packages)
    // Auto-decryption handles passwords automatically

    ModelPackager packager;
    std::vector<std::string> file_list;

    bool success = packager.ListPackageContents(args.input_path, file_list);

    if (success) {
        std::cout << "Package contents (" << file_list.size() << " files):" << std::endl;
        for (const auto &file : file_list) {
            std::cout << "  " << file << std::endl;
        }
        return 0;
    } else {
        PrintError("Failed to list package contents: " + packager.GetLastError());
        return 1;
    }
}

/**
 * @brief Execute info command
 */
int ExecuteInfo(const CommandLineArgs &args) {
    PrintVerbose(args.verbose, "Getting package information: " + args.input_path);

    // Get password if provided (may be needed for encrypted packages)
    // Auto-decryption handles passwords automatically

    ModelPackager packager;
    ModelPackageInfo package_info;

    bool success = packager.GetPackageInfo(args.input_path, package_info);

    if (success) {
        std::cout << "Package Information:" << std::endl;
        std::cout << "  Name: " << package_info.package_name << std::endl;
        std::cout << "  Version: " << package_info.version << std::endl;
        std::cout << "  Description: " << package_info.description << std::endl;
        std::cout << "  Model Type: " << static_cast<int>(package_info.model_type) << std::endl;
        std::cout << "  Files: " << package_info.model_files.size() << std::endl;

        if (!package_info.metadata.empty()) {
            std::cout << "  Metadata:" << std::endl;
            for (const auto &kv : package_info.metadata) {
                std::cout << "    " << kv.first << ": " << kv.second << std::endl;
            }
        }

        // Show file size
        long long package_size = rayshape::tools::utils::FileUtils::GetFileSize(args.input_path);
        if (package_size > 0) {
            std::cout << "  Package Size: " << package_size << " bytes" << std::endl;
        }

        return 0;
    } else {
        PrintError("Failed to get package information: " + packager.GetLastError());
        return 1;
    }
}

/**
 * @brief Execute validate command
 */
int ExecuteValidate(const CommandLineArgs &args) {
    PrintVerbose(args.verbose, "Validating package: " + args.input_path);

    // Get password if provided (may be needed for encrypted packages)
    // Auto-decryption handles passwords automatically

    ModelPackager packager;

    bool success = packager.ValidatePackage(args.input_path);

    if (success) {
        PrintSuccess("Package validation passed");
        return 0;
    } else {
        PrintError("Package validation failed: " + packager.GetLastError());
        return 1;
    }
}

/**
 * @brief Execute deserialize command
 */
int ExecuteDeserialize(const CommandLineArgs &args) {
    PrintVerbose(args.verbose, "Starting model deserialization...");
    PrintVerbose(args.verbose, "Input RSM file: " + args.input_path);
    PrintVerbose(args.verbose, "Output directory: " + args.output_path);

    try {
        // Create output directory if it doesn't exist
        if (!rayshape::tools::utils::FileUtils::DirectoryExists(args.output_path)) {
            if (!rayshape::tools::utils::FileUtils::CreateDirectory(args.output_path)) {
                PrintError("Failed to create output directory: " + args.output_path);
                return 1;
            }
            PrintVerbose(args.verbose, "Created output directory: " + args.output_path);
        } else {
            PrintWarning("Output directory already exists: " + args.output_path);
        }

        // Load model from RSM file using existing parsing infrastructure
        auto model = rayshape::ParseModel(args.input_path);
        if (!model) {
            PrintError("Failed to parse model from RSM file: " + args.input_path);
            return 1;
        }

        PrintVerbose(args.verbose, "Successfully loaded model from RSM file");

        // Determine model type and extract content
        rayshape::ModelType model_type = model->GetModelType();
        PrintVerbose(args.verbose, "Detected model type: " + std::to_string(static_cast<int>(model_type)));

        switch (model_type) {
        case rayshape::ModelType::MNN: {
#ifdef ENABLE_MNN_MODEL
            auto* mnn_model = dynamic_cast<rayshape::MNNModel*>(model.get());
            if (!mnn_model) {
                PrintError("Failed to cast to MNNModel");
                return 1;
            }

            // Write MNN model file
            std::string mnn_file_path = rayshape::tools::utils::FileUtils::JoinPath(args.output_path, "model.mnn");
            if (!rayshape::tools::utils::FileUtils::WriteFileContent(mnn_file_path, mnn_model->bin_buf_)) {
                PrintError("Failed to write MNN model file: " + mnn_file_path);
                return 1;
            }

            // Write config file if available
            if (!mnn_model->cfg_str_.empty()) {
                std::string config_file_path = rayshape::tools::utils::FileUtils::JoinPath(args.output_path, "config.json");
                if (!rayshape::tools::utils::FileUtils::WriteFileContent(config_file_path, mnn_model->cfg_str_)) {
                    PrintError("Failed to write config file: " + config_file_path);
                    return 1;
                }
                PrintVerbose(args.verbose, "Wrote config file: " + config_file_path);
            }

            PrintSuccess("MNN model deserialized successfully:");
            std::cout << "  Model file: " << mnn_file_path << std::endl;
            if (!mnn_model->cfg_str_.empty()) {
                std::cout << "  Config file: " << rayshape::tools::utils::FileUtils::JoinPath(args.output_path, "config.json") << std::endl;
            }
#else
            PrintError("MNN model support not enabled in this build");
            return 1;
#endif
            break;
        }

        case rayshape::ModelType::OPENVINO: {
#ifdef ENABLE_OPENVINO_MODEL
            auto* ov_model = dynamic_cast<rayshape::OpenVINOModel*>(model.get());
            if (!ov_model) {
                PrintError("Failed to cast to OpenVINOModel");
                return 1;
            }

            // Write XML file
            std::string xml_file_path = rayshape::tools::utils::FileUtils::JoinPath(args.output_path, "model.xml");
            if (!rayshape::tools::utils::FileUtils::WriteFileContent(xml_file_path, ov_model->xml_content_)) {
                PrintError("Failed to write XML file: " + xml_file_path);
                return 1;
            }

            // Write BIN file
            std::string bin_file_path = rayshape::tools::utils::FileUtils::JoinPath(args.output_path, "model.bin");
            if (!rayshape::tools::utils::FileUtils::WriteFileContent(bin_file_path, ov_model->bin_content_)) {
                PrintError("Failed to write BIN file: " + bin_file_path);
                return 1;
            }

            // Write config file if available
            if (!ov_model->cfg_str_.empty()) {
                std::string config_file_path = rayshape::tools::utils::FileUtils::JoinPath(args.output_path, "config.json");
                if (!rayshape::tools::utils::FileUtils::WriteFileContent(config_file_path, ov_model->cfg_str_)) {
                    PrintError("Failed to write config file: " + config_file_path);
                    return 1;
                }
                PrintVerbose(args.verbose, "Wrote config file: " + config_file_path);
            }

            PrintSuccess("OpenVINO model deserialized successfully:");
            std::cout << "  XML file: " << xml_file_path << std::endl;
            std::cout << "  BIN file: " << bin_file_path << std::endl;
            if (!ov_model->cfg_str_.empty()) {
                std::cout << "  Config file: " << rayshape::tools::utils::FileUtils::JoinPath(args.output_path, "config.json") << std::endl;
            }
#else
            PrintError("OpenVINO model support not enabled in this build");
            return 1;
#endif
            break;
        }

        case rayshape::ModelType::ONNX: {
#ifdef ENABLE_ONNX_MODEL
            auto* onnx_model = dynamic_cast<rayshape::ONNXModel*>(model.get());
            if (!onnx_model) {
                PrintError("Failed to cast to ONNXModel");
                return 1;
            }

            // Write ONNX model file
            std::string onnx_file_path = rayshape::tools::utils::FileUtils::JoinPath(args.output_path, "model.onnx");
            if (!rayshape::tools::utils::FileUtils::WriteFileContent(onnx_file_path, onnx_model->bin_buf_)) {
                PrintError("Failed to write ONNX model file: " + onnx_file_path);
                return 1;
            }

            // Write config file if available
            if (!onnx_model->cfg_str_.empty()) {
                std::string config_file_path = rayshape::tools::utils::FileUtils::JoinPath(args.output_path, "config.json");
                if (!rayshape::tools::utils::FileUtils::WriteFileContent(config_file_path, onnx_model->cfg_str_)) {
                    PrintError("Failed to write config file: " + config_file_path);
                    return 1;
                }
                PrintVerbose(args.verbose, "Wrote config file: " + config_file_path);
            }

            PrintSuccess("ONNX model deserialized successfully:");
            std::cout << "  Model file: " << onnx_file_path << std::endl;
            if (!onnx_model->cfg_str_.empty()) {
                std::cout << "  Config file: " << rayshape::tools::utils::FileUtils::JoinPath(args.output_path, "config.json") << std::endl;
            }
#else
            PrintError("ONNX model support not enabled in this build");
            return 1;
#endif
            break;
        }
        case rayshape::ModelType::TENSORRT: {
#ifdef ENABLE_TENSORRT_MODEL
            auto* tensorrt_model = dynamic_cast<rayshape::TensorRTModel*>(model.get());
            if (!tensorrt_model) {
                PrintError("Failed to cast to TensorRTModel");
                return 1;
            }

            // 写入TensorRT引擎文件
            std::string engine_file_path = rayshape::tools::utils::FileUtils::JoinPath(args.output_path, "model.trt");
            if (!rayshape::tools::utils::FileUtils::WriteFileContent(engine_file_path, tensorrt_model->engine_data_)) {
                PrintError("Failed to write TensorRT engine file: " + engine_file_path);
                return 1;
            }

            // 写入配置文件
            if (!tensorrt_model->cfg_str_.empty()) {
                std::string config_file_path = rayshape::tools::utils::FileUtils::JoinPath(args.output_path, "config.json");
                if (!rayshape::tools::utils::FileUtils::WriteFileContent(config_file_path, tensorrt_model->cfg_str_)) {
                    PrintError("Failed to write config file: " + config_file_path);
                    return 1;
                }
                PrintVerbose(args.verbose, "Wrote config file: " + config_file_path);
            }

            PrintSuccess("TensorRT model deserialized successfully:");
            std::cout << "  Engine file: " << engine_file_path << std::endl;
            if (!tensorrt_model->cfg_str_.empty()) {
                std::cout << "  Config file: " << rayshape::tools::utils::FileUtils::JoinPath(args.output_path, "config.json") << std::endl;
            }
#else
            PrintError("TensorRT model support not enabled in this build");
            return 1;
#endif
            break;
        }
        default:
            PrintError("Unknown or unsupported model type: " + std::to_string(static_cast<int>(model_type)));
            return 1;
        }

        PrintVerbose(args.verbose, "Deserialization completed successfully");
        return 0;

    } catch (const std::exception &e) {
        PrintError("Exception during deserialization: " + std::string(e.what()));
        return 1;
    }
}

/**
 * @brief Execute decode command
 */
int ExecuteDecode(const CommandLineArgs &args) {
    PrintVerbose(args.verbose, "Starting model decoding...");
    PrintVerbose(args.verbose, "Input file: " + args.input_path);
    PrintVerbose(args.verbose, "Output file: " + args.output_path);

    try {
        // Read encrypted file
        std::vector<uint8_t> file_data;
        if (!rayshape::tools::utils::FileUtils::ReadBinaryFile(args.input_path, file_data)) {
            PrintError("Cannot read input file: " + args.input_path);
            return 1;
        }

        // Check if file is encrypted and decrypt if necessary
        std::vector<uint8_t> decrypted_data;
        if (file_data.size() >= 4) {
            std::string magic_str(file_data.begin(), file_data.begin() + 4);
            if (magic_str == "RAEC" || magic_str == "RENC") {
                PrintVerbose(args.verbose, "Detected encrypted model (magic: " + magic_str + "), decrypting...");

                if (!rayshape::utils::codec::AutoCrypto::AutoDecrypt(file_data, decrypted_data)) {
                    PrintError("Failed to decrypt model file: " + rayshape::utils::codec::AutoCrypto::GetLastError());
                    return 1;
                }

                PrintVerbose(args.verbose, "Successfully decrypted model");
            } else {
                PrintVerbose(args.verbose, "File is not encrypted (magic: " + magic_str + "), copying as-is");
                decrypted_data = file_data;
            }
        } else {
            PrintError("Invalid file format - file too small");
            return 1;
        }

        // Write decrypted data to output file
        if (!rayshape::tools::utils::FileUtils::WriteBinaryFile(args.output_path, decrypted_data)) {
            PrintError("Failed to write output file: " + args.output_path);
            return 1;
        }

        PrintVerbose(args.verbose, "Decoding completed successfully");
        PrintSuccess("Model decoded successfully: " + args.output_path);
        return 0;

    } catch (const std::exception &e) {
        PrintError("Exception during decoding: " + std::string(e.what()));
        return 1;
    }
}

/**
 * @brief Main entry point
 */
int main(int argc, char *argv[]) {
    CLIParser parser;
    CommandLineArgs args;

    // Parse command line arguments
    if (!parser.Parse(argc, argv, args)) {
        PrintError(parser.GetLastError());
        std::cout << "\nUse '" << argv[0] << " help' for usage information." << std::endl;
        return 1;
    }

    // Handle help command
    if (args.command == CommandType::HELP) {
        parser.ShowHelp();
        return 0;
    }

    // Check for file overwrite protection
    if (!args.force_overwrite && !args.output_path.empty()) {
        if (rayshape::tools::utils::FileUtils::FileExists(args.output_path)) {
            PrintError("Output file already exists: " + args.output_path);
            std::cout << "Use -f/--force to overwrite existing files." << std::endl;
            return 1;
        }
    }

    // Execute command
    try {
        switch (args.command) {
        case CommandType::SERIALIZE:
            return ExecuteSerialize(args);

        case CommandType::DESERIALIZE:
            return ExecuteDeserialize(args);

        case CommandType::PACKAGE:
            return ExecutePackage(args);

        case CommandType::EXTRACT:
            return ExecuteExtract(args);

        case CommandType::LIST:
            return ExecuteList(args);

        case CommandType::INFO:
            return ExecuteInfo(args);

        case CommandType::VALIDATE:
            return ExecuteValidate(args);

        case CommandType::DECODE:
            return ExecuteDecode(args);

        default:
            PrintError("Unknown command");
            return 1;
        }
    } catch (const std::exception &e) {
        PrintError("Unexpected error: " + std::string(e.what()));
        return 1;
    }
}
