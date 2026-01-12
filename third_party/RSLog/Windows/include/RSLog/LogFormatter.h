//=============================================================================
//
//  Copyright (c) 2019-2025 RayShape Medical Technology Co., Ltd.
//  All Rights Reserved.
//  Confidential and Proprietary - RayShape Medical Technology Co., Ltd.
//
//=============================================================================

#pragma once

#include "RSLogExport.h"
#include <string>
#include <sstream>
#include <vector>
#include <type_traits>

namespace RSLog {

/**
 * @brief Format helper class for {} placeholder formatting
 */
class RSLOG_API LogFormatter {
public:
    /**
     * @brief Format string with {} placeholders (no arguments version)
     * @param format Format string containing {} placeholders
     * @return Formatted string
     */
    static std::string format(const std::string& format) {
        return formatImpl(format, std::vector<std::string>());
    }

    /**
     * @brief Format string with {} placeholders
     * @param format Format string containing {} placeholders
     * @param args Arguments to substitute into placeholders
     * @return Formatted string
     */
    template<typename... Args>
    static std::string format(const std::string& format, Args&&... args) {
        std::vector<std::string> argStrings;
        argStrings.reserve(sizeof...(args));
        
        // Convert all arguments to strings
#if RSLOGKIT_HAS_FOLD_EXPRESSIONS
        (argStrings.push_back(toString(std::forward<Args>(args))), ...);
#else
        // C++14 fallback using recursive template expansion
        convertArgsToStrings(argStrings, std::forward<Args>(args)...);
#endif
        
        return formatImpl(format, argStrings);
    }

    /**
     * @brief Implementation of format function
     * @param format Format string
     * @param args Vector of string arguments
     * @return Formatted string
     */
    static std::string formatImpl(const std::string& format, const std::vector<std::string>& args);

private:
#if !RSLOGKIT_HAS_FOLD_EXPRESSIONS
    /**
     * @brief C++14 fallback for fold expressions - base case (no arguments)
     * @param argStrings Vector to store converted arguments
     */
    static void convertArgsToStrings(std::vector<std::string>& /* argStrings */) {
        // Base case - do nothing
    }
    
    /**
     * @brief C++14 fallback for fold expressions - recursive case
     * @param argStrings Vector to store converted arguments
     * @param first First argument
     * @param rest Remaining arguments
     */
    template<typename T, typename... Rest>
    static void convertArgsToStrings(std::vector<std::string>& argStrings, T&& first, Rest&&... rest) {
        argStrings.push_back(toString(std::forward<T>(first)));
        convertArgsToStrings(argStrings, std::forward<Rest>(rest)...);
    }
#endif
    /**
     * @brief Convert value to string
     * @tparam T Value type
     * @param value Value to convert
     * @return String representation
     */
    template<typename T>
    static std::string toString(T&& value) {
#if RSLOGKIT_HAS_IF_CONSTEXPR
        if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
            return value;
        } else if constexpr (std::is_same_v<std::decay_t<T>, const char*>) {
            return std::string(value);
        } else if constexpr (std::is_same_v<std::decay_t<T>, char*>) {
            return std::string(value);
        } else if constexpr (std::is_same_v<std::decay_t<T>, bool>) {
            return value ? "true" : "false";
        } else if constexpr (std::is_same_v<std::decay_t<T>, char>) {
            return std::string(1, value);
        } else if constexpr (std::is_arithmetic_v<std::decay_t<T>>) {
            return std::to_string(value);
        } else {
            // For other types, try to use stringstream
            std::ostringstream oss;
            oss << value;
            return oss.str();
        }
#else
        // C++14 fallback using SFINAE and function overloading
        return toStringImpl(std::forward<T>(value));
#endif
    }

#if !RSLOGKIT_HAS_IF_CONSTEXPR
private:
    /**
     * @brief C++14 SFINAE-based toString implementation for std::string
     */
    template<typename T>
    static typename std::enable_if<std::is_same<typename std::decay<T>::type, std::string>::value, std::string>::type
    toStringImpl(T&& value) {
        return value;
    }
    
    /**
     * @brief C++14 SFINAE-based toString implementation for const char*
     */
    template<typename T>
    static typename std::enable_if<std::is_same<typename std::decay<T>::type, const char*>::value, std::string>::type
    toStringImpl(T&& value) {
        return std::string(value);
    }
    
    /**
     * @brief C++14 SFINAE-based toString implementation for char*
     */
    template<typename T>
    static typename std::enable_if<std::is_same<typename std::decay<T>::type, char*>::value, std::string>::type
    toStringImpl(T&& value) {
        return std::string(value);
    }
    
    /**
     * @brief C++14 SFINAE-based toString implementation for bool
     */
    template<typename T>
    static typename std::enable_if<std::is_same<typename std::decay<T>::type, bool>::value, std::string>::type
    toStringImpl(T&& value) {
        return value ? "true" : "false";
    }
    
    /**
     * @brief C++14 SFINAE-based toString implementation for char
     */
    template<typename T>
    static typename std::enable_if<std::is_same<typename std::decay<T>::type, char>::value, std::string>::type
    toStringImpl(T&& value) {
        return std::string(1, value);
    }
    
    /**
     * @brief C++14 SFINAE-based toString implementation for arithmetic types (excluding bool and char)
     */
    template<typename T>
    static typename std::enable_if<
        std::is_arithmetic<typename std::decay<T>::type>::value && 
        !std::is_same<typename std::decay<T>::type, bool>::value &&
        !std::is_same<typename std::decay<T>::type, char>::value, 
        std::string
    >::type
    toStringImpl(T&& value) {
        return std::to_string(value);
    }
    
    /**
     * @brief C++14 SFINAE-based toString implementation for other types (fallback)
     */
    template<typename T>
    static typename std::enable_if<
        !std::is_same<typename std::decay<T>::type, std::string>::value &&
        !std::is_same<typename std::decay<T>::type, const char*>::value &&
        !std::is_same<typename std::decay<T>::type, char*>::value &&
        !std::is_arithmetic<typename std::decay<T>::type>::value,
        std::string
    >::type
    toStringImpl(T&& value) {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }
#endif
};

} // namespace RSLog 