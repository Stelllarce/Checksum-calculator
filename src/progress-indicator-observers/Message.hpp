#pragma once
#include <cstdint>
#include <string>

/**
 * @brief Base message type for Observer notifications.
 */
struct Message {
    enum class Type { NewFile, BytesRead };
    explicit Message(Type t) : type(t) {}
    virtual ~Message() = default;
    Type type;
};

/**
 * @brief Notifies that a new file is about to be processed.
 */
struct NewFileMessage : public Message {
    explicit NewFileMessage(std::string p)
        : Message(Type::NewFile), path(std::move(p)) {}
    std::string path;
};

/**
 * @brief Notifies cumulative bytes processed for current file.
 */
struct BytesReadMessage : public Message {
    explicit BytesReadMessage(std::uint64_t n)
        : Message(Type::BytesRead), bytesRead(n) {}
    std::uint64_t bytesRead;
};