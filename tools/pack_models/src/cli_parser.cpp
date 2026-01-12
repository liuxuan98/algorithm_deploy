/**
 * @file cli_parser.cpp
 * @brief Command line interface parser implementation
 * @copyright (c) 2025 Shenzhen RayShape Medical Technology Co., Ltd.
 */

#include "cli_parser.h"
#include "utils/platform_utils.h"

#include <iostream>
#include <algorithm>
#include <sstream>

namespace rayshape
{
    namespace tools
    {

        bool CLIParser::Parse(int argc, char *argv[], CommandLineArgs &args) {
            if (argc < 2) {
                SetError("No command specified");
                return false;
            }

            // Parse command
            std::string command_str = argv[1];
            std::transform(command_str.begin(), command_str.end(), command_str.begin(), ::tolower);
            args.command = ParseCommand(command_str);

            if (args.command == CommandType::NONE) {
                SetError("Unknown command: " + command_str);
                return false;
            }

            if (args.command == CommandType::HELP) {
                return true; // No further parsing needed for help
            }

            // Parse arguments
            for (int i = 2; i < argc; ++i) {
                std::string arg = argv[i];

                if (arg == "-h" || arg == "--help") {
                    args.command = CommandType::HELP;
                    return true;
                } else if (arg == "-v" || arg == "--verbose") {
                    args.verbose = true;
                } else if (arg == "-f" || arg == "--force") {
                    args.force_overwrite = true;
                } else if (arg == "-t" || arg == "--type") {
                    if (i + 1 >= argc) {
                        SetError("Model type not specified after -t/--type");
                        return false;
                    }
                    args.model_type = ParseModelType(argv[++i]);
                    if (args.model_type == rayshape::ModelType::NONE) {
                        SetError("Invalid model type: " + std::string(argv[i]));
                        return false;
                    }
                } else if (arg == "-i" || arg == "--input") {
                    if (i + 1 >= argc) {
                        SetError("Input path not specified after -i/--input");
                        return false;
                    }
                    args.input_path = argv[++i];
                } else if (arg == "-o" || arg == "--output") {
                    if (i + 1 >= argc) {
                        SetError("Output path not specified after -o/--output");
                        return false;
                    }
                    args.output_path = argv[++i];
                } else if (arg == "--xml") {
                    if (i + 1 >= argc) {
                        SetError("XML path not specified after --xml");
                        return false;
                    }
                    args.xml_path = argv[++i];
                } else if (arg == "--bin") {
                    if (i + 1 >= argc) {
                        SetError("BIN path not specified after --bin");
                        return false;
                    }
                    args.bin_path = argv[++i];
                } else if (arg == "--config") {
                    if (i + 1 >= argc) {
                        SetError("Config path not specified after --config");
                        return false;
                    }
                    args.config_path = argv[++i];
                } else if (arg == "-n" || arg == "--name") {
                    if (i + 1 >= argc) {
                        SetError("Package name not specified after -n/--name");
                        return false;
                    }
                    args.package_name = argv[++i];
                } else if (arg == "--version") {
                    if (i + 1 >= argc) {
                        SetError("Version not specified after --version");
                        return false;
                    }
                    args.version = argv[++i];
                } else if (arg == "-d" || arg == "--description") {
                    if (i + 1 >= argc) {
                        SetError("Description not specified after -d/--description");
                        return false;
                    }
                    args.description = argv[++i];
                } else if (arg == "-c" || arg == "--compression") {
                    if (i + 1 >= argc) {
                        SetError("Compression level not specified after -c/--compression");
                        return false;
                    }
                    try {
                        args.compression_level = std::stoi(argv[++i]);
                        if (args.compression_level < 0 || args.compression_level > 9) {
                            SetError("Compression level must be between 0-9");
                            return false;
                        }
                    } catch (const std::exception &) {
                        SetError("Invalid compression level: " + std::string(argv[i]));
                        return false;
                    }
                } else if (arg == "--files") {
                    // Collect all remaining arguments as input files
                    for (int j = i + 1; j < argc; ++j) {
                        std::string file_arg = argv[j];
                        if (file_arg.empty() || file_arg[0] == '-') {
                            break; // Stop at next option
                        }
                        args.input_files.push_back(file_arg);
                        ++i;
                    }
                } else if (arg == "--metadata") {
                    if (i + 1 >= argc) {
                        SetError("Metadata not specified after --metadata");
                        return false;
                    }
                    std::string metadata_str = argv[++i];
                    // Parse key=value pairs separated by comma
                    std::istringstream iss(metadata_str);
                    std::string pair;
                    while (std::getline(iss, pair, ',')) {
                        size_t eq_pos = pair.find('=');
                        if (eq_pos != std::string::npos) {
                            std::string key = pair.substr(0, eq_pos);
                            std::string value = pair.substr(eq_pos + 1);
                            args.metadata[key] = value;
                        }
                    }
                } else if (arg == "--encrypt") {
                    args.enable_encryption = true;
                } else if (arg[0] != '-') {
                    // Positional argument - treat as input file if no input path set
                    if (args.input_path.empty()) {
                        args.input_path = arg;
                    } else {
                        args.input_files.push_back(arg);
                    }
                } else {
                    SetError("Unknown option: " + arg);
                    return false;
                }
            }

                        // No additional validation needed for encryption - it's auto-managed

            return ValidateArguments(args);
        }

        void CLIParser::ShowHelp() const {
            std::cout << TOOL_NAME << " v" << TOOL_VERSION
                      << " - Model serialization and packaging tool\n\n";

            std::cout << "USAGE:\n";
            std::cout << "  " << TOOL_NAME << " <command> [options]\n\n";

            std::cout << "COMMANDS:\n";
            std::cout << "  serialize    Serialize individual model files\n";
            std::cout << "  package      Create model package from multiple files\n";
            std::cout << "  extract      Extract model package\n";
            std::cout << "  list         List package contents\n";
            std::cout << "  info         Show package information\n";
            std::cout << "  validate     Validate package integrity\n";
            std::cout << "  help         Show this help message\n\n";

            ShowCommandUsage(CommandType::SERIALIZE);
            ShowCommandUsage(CommandType::PACKAGE);
            ShowCommandUsage(CommandType::EXTRACT);
            ShowCommandUsage(CommandType::LIST);
            ShowCommandUsage(CommandType::INFO);
            ShowCommandUsage(CommandType::VALIDATE);

            std::cout << "GLOBAL OPTIONS:\n";
            std::cout << "  -h, --help                Show help message\n";
            std::cout << "  -v, --verbose             Enable verbose output\n";
            std::cout << "  -f, --force               Force overwrite existing files\n";
            std::cout << "  -c, --compression <0-9>   Set compression level (default: 6)\n\n";

            std::cout << "ENCRYPTION OPTIONS:\n";
            std::cout << "  --encrypt                 Enable automatic encryption for output files\n";
            std::cout << "                            (Uses auto-generated secure keys, transparent to user)\n\n";

            std::cout << "MODEL TYPES:\n";
            std::cout << "  mnn          MNN model format\n";
            std::cout << "  openvino     OpenVINO model format\n\n";

            std::cout << "EXAMPLES:\n";
            std::cout << "  # Serialize MNN model\n";
            std::cout
                << "  " << TOOL_NAME
                << " serialize -t mnn -i model.mnn --config config.json -o model_serialized.rsm\n\n";

            std::cout << "  # Serialize OpenVINO model\n";
            std::cout
                << "  " << TOOL_NAME
                << " serialize -t openvino --xml model.xml --bin model.bin --config config.json -o model_serialized.rsm\n\n";

            std::cout << "  # Create package\n";
            std::cout
                << "  " << TOOL_NAME
                << " package -n \"My Model\" --version \"1.0\" -d \"Description\" --files model1.rsm model2.rsm -o package.rsmp\n\n";

            std::cout << "  # Extract package\n";
            std::cout << "  " << TOOL_NAME << " extract -i package.rsmp -o output_dir/\n";

            std::cout << "Platform: " << utils::PlatformUtils::GetPlatformName() << "\n";
        }

        void CLIParser::ShowVersion() const {
            std::cout << TOOL_NAME << " version " << TOOL_VERSION << std::endl;
            std::cout << "Platform: " << utils::PlatformUtils::GetPlatformName() << std::endl;
        }

        rayshape::ModelType CLIParser::ParseModelType(const std::string &type_str) const {
            std::string lower_type = type_str;
            std::transform(lower_type.begin(), lower_type.end(), lower_type.begin(), ::tolower);

            if (lower_type == "mnn") {
                return rayshape::ModelType::MNN;
            } else if (lower_type == "openvino" || lower_type == "ov") {
                return rayshape::ModelType::OPENVINO;
            } else if (lower_type == "tensorrt" || lower_type == "trt") {
                return rayshape::ModelType::TENSORRT;
            } else if (lower_type == "coreml") {
                return rayshape::ModelType::COREML;
            } else if (lower_type == "ncnn") {
                return rayshape::ModelType::NCNN;
            } else if (lower_type == "onnx") {
                return rayshape::ModelType::ONNX;
            }

            return rayshape::ModelType::NONE;
        }

        CommandType CLIParser::ParseCommand(const std::string &command_str) const {
            if (command_str == "serialize" || command_str == "s") {
                return CommandType::SERIALIZE;
            } else if (command_str == "package" || command_str == "pack" || command_str == "p") {
                return CommandType::PACKAGE;
            } else if (command_str == "extract" || command_str == "e") {
                return CommandType::EXTRACT;
            } else if (command_str == "list" || command_str == "ls" || command_str == "l") {
                return CommandType::LIST;
            } else if (command_str == "info" || command_str == "i") {
                return CommandType::INFO;
            } else if (command_str == "validate" || command_str == "check" || command_str == "v") {
                return CommandType::VALIDATE;
            } else if (command_str == "help" || command_str == "h") {
                return CommandType::HELP;
            }

            return CommandType::NONE;
        }

        bool CLIParser::ValidateArguments(const CommandLineArgs &args) {
            switch (args.command) {
            case CommandType::SERIALIZE:
                if (args.output_path.empty()) {
                    SetError("Output path required for serialize command");
                    return false;
                }
                if (args.model_type == rayshape::ModelType::NONE) {
                    SetError("Model type required for serialize command");
                    return false;
                }
                if (args.config_path.empty()) {
                    SetError("Config file path required for serialize command (use --config)");
                    return false;
                }
                if (args.model_type == rayshape::ModelType::OPENVINO) {
                    if (args.xml_path.empty() || args.bin_path.empty()) {
                        SetError("Both XML and BIN paths required for OpenVINO models");
                        return false;
                    }
                } else {
                    // For non-OpenVINO models (MNN, etc.), check input_path
                    if (args.input_path.empty()) {
                        SetError("Input path required for serialize command");
                        return false;
                    }
                }
                break;

            case CommandType::PACKAGE:
                if (args.input_files.empty() && args.input_path.empty()) {
                    SetError("Input files required for package command");
                    return false;
                }
                if (args.output_path.empty()) {
                    SetError("Output path required for package command");
                    return false;
                }
                if (args.package_name.empty()) {
                    SetError("Package name required for package command");
                    return false;
                }
                break;

            case CommandType::EXTRACT:
                if (args.input_path.empty()) {
                    SetError("Input package path required for extract command");
                    return false;
                }
                if (args.output_path.empty()) {
                    SetError("Output directory required for extract command");
                    return false;
                }
                break;

            case CommandType::LIST:
            case CommandType::INFO:
            case CommandType::VALIDATE:
                if (args.input_path.empty()) {
                    SetError("Input package path required");
                    return false;
                }
                break;

            case CommandType::HELP:
                // No validation needed
                break;

            default:
                SetError("Invalid command");
                return false;
            }

            return true;
        }

        void CLIParser::SetError(const std::string &error) {
            last_error_ = error;
        }

        void CLIParser::ShowCommandUsage(CommandType command) const {
            switch (command) {
            case CommandType::SERIALIZE:
                std::cout << "SERIALIZE:\n";
                std::cout << "  " << TOOL_NAME
                          << " serialize -t <type> -i <input> -o <output> [options]\n";
                std::cout << "  Options:\n";
                std::cout << "    -t, --type <type>         Model type (mnn, openvino)\n";
                std::cout << "    -i, --input <path>        Input model file\n";
                std::cout << "    -o, --output <path>       Output serialized file\n";
                std::cout << "                        --xml <path>              XML file (OpenVINO only)\n";
                std::cout << "    --bin <path>              BIN file (OpenVINO only)\n";
                std::cout << "    --config <path>           Configuration JSON file (required)\n";
                std::cout << "    --encrypt                 Enable automatic encryption\n\n";
                break;

            case CommandType::PACKAGE:
                std::cout << "PACKAGE:\n";
                std::cout << "  " << TOOL_NAME
                          << " package -n <name> --files <files...> -o <output> [options]\n";
                std::cout << "  Options:\n";
                std::cout << "    -n, --name <name>         Package name\n";
                std::cout << "    --version <version>       Package version\n";
                std::cout << "    -d, --description <desc>  Package description\n";
                std::cout << "    --files <files...>        Input files to package\n";
                std::cout << "    -o, --output <path>       Output package file\n";
                std::cout << "    --metadata <key=val,...>  Additional metadata\n";
                std::cout << "    --encrypt                 Enable automatic encryption\n\n";
                break;

            case CommandType::EXTRACT:
                std::cout << "EXTRACT:\n";
                std::cout << "  " << TOOL_NAME << " extract -i <package> -o <output_dir>\n";
                std::cout << "  Options:\n";
                std::cout << "    -i, --input <path>        Input package file\n";
                std::cout << "    -o, --output <path>       Output directory\n";
                std::cout << "                              (Encrypted packages are automatically decrypted)\n\n";
                break;

            case CommandType::LIST:
                std::cout << "LIST:\n";
                std::cout << "  " << TOOL_NAME << " list -i <package>\n";
                std::cout << "  Options:\n";
                std::cout << "    -i, --input <path>        Input package file\n\n";
                break;

            case CommandType::INFO:
                std::cout << "INFO:\n";
                std::cout << "  " << TOOL_NAME << " info -i <package>\n";
                std::cout << "  Options:\n";
                std::cout << "    -i, --input <path>        Input package file\n\n";
                break;

            case CommandType::VALIDATE:
                std::cout << "VALIDATE:\n";
                std::cout << "  " << TOOL_NAME << " validate -i <package>\n";
                std::cout << "  Options:\n";
                std::cout << "    -i, --input <path>        Input package file\n\n";
                break;

            default:
                break;
            }
        }

    } // namespace tools
} // namespace rayshape
