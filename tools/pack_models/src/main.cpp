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

        default:
            PrintError("Unknown command");
            return 1;
        }
    } catch (const std::exception &e) {
        PrintError("Unexpected error: " + std::string(e.what()));
        return 1;
    }
}
