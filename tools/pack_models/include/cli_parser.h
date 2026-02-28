/**
 * @file cli_parser.h
 * @brief Command line interface parser for pack_models tool
 * @copyright (c) .
 */

#ifndef PACK_MODELS_CLI_PARSER_H_
#define PACK_MODELS_CLI_PARSER_H_

#include <string>
#include <vector>
#include <map>

#include "base/common.h"
#include "model_packager.h"

namespace rayshape
{
    namespace tools
    {

        /**
         * @brief Command type enumeration
         */
        enum class CommandType {
            NONE,
            SERIALIZE,   // Serialize individual model
            DESERIALIZE, // Deserialize model file back to original format
            PACKAGE,     // Create model package
            EXTRACT,     // Extract model package
            LIST,        // List package contents
            INFO,        // Show package info
            VALIDATE,    // Validate package
            DECODE,      // Decode encrypted model file
            HELP         // Show help
        };

        /**
         * @brief Command line arguments structure
         */
        struct CommandLineArgs {
            CommandType command = CommandType::NONE;
            rayshape::ModelType model_type = rayshape::ModelType::NONE;

            // Input/Output paths
            std::string input_path;
            std::string output_path;
            std::string xml_path;    // For OpenVINO models
            std::string bin_path;    // For OpenVINO models
            std::string config_path; // For model configuration JSON file
            std::vector<std::string> input_files;

            // Package information
            std::string package_name;
            std::string version;
            std::string description;
            std::map<std::string, std::string> metadata;

            // Options
            int compression_level = 6;
            bool verbose = false;
            bool force_overwrite = false;

            // Encryption options
            bool enable_encryption = false;

            CommandLineArgs() = default;
        };

        /**
         * @brief Command line interface parser class
         */
        class CLIParser {
        public:
            CLIParser() = default;
            ~CLIParser() = default;

            /**
             * @brief Parse command line arguments
             * @param argc Argument count
             * @param argv Argument values
             * @param args Output parsed arguments
             * @return true if successful, false otherwise
             */
            bool Parse(int argc, char *argv[], CommandLineArgs &args);

            /**
             * @brief Show help information
             */
            void ShowHelp() const;

            /**
             * @brief Show version information
             */
            void ShowVersion() const;

            /**
             * @brief Get last error message
             * @return Error message string
             */
            const std::string &GetLastError() const {
                return last_error_;
            }

        private:
            /**
             * @brief Parse model type from string
             * @param type_str Model type string
             * @return ModelType value
             */
            rayshape::ModelType ParseModelType(const std::string &type_str) const;

            /**
             * @brief Parse command from string
             * @param command_str Command string
             * @return CommandType value
             */
            CommandType ParseCommand(const std::string &command_str) const;

            /**
             * @brief Validate parsed arguments
             * @param args Arguments to validate
             * @return true if valid, false otherwise
             */
            bool ValidateArguments(const CommandLineArgs &args);

            /**
             * @brief Set last error message
             * @param error Error message
             */
            void SetError(const std::string &error);

            /**
             * @brief Show usage for specific command
             * @param command Command type
             */
            void ShowCommandUsage(CommandType command) const;

        private:
            std::string last_error_;
            static constexpr const char *TOOL_VERSION = "1.0.0";
            static constexpr const char *TOOL_NAME = "pack_models";
        };

    } // namespace tools
} // namespace rayshape

#endif // PACK_MODELS_CLI_PARSER_H_
