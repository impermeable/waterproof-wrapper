#ifndef WPWRAPPER_EXCEPTIONS_H
#define WPWRAPPER_EXCEPTIONS_H

#include <cerrno>
#include <stdexcept>

#include <spdlog/logger.h>

namespace wpwrapper {

/// \brief An object that can be thrown as exception. It reports errors that occur as a result of a call to a platform
/// native API.
class api_error : public std::runtime_error {

public:
    /// \brief Constructs the exception object with \c message as an explanatory string.
    /// \param message An explanatory string.
    explicit api_error(const std::string& message) noexcept;

    /// \brief Constructs the exception object with \c message as an explanatory string and with \c error_number
    /// providing a more detailed indication of the kind of error that occurred.
    /// \param message An explanatory string.
    /// \param error_number Indicates the kind of error, typically provided by the platform native API.
    api_error(const std::string& message, int error_number) noexcept;

    /// \brief Constructs the exception object with \c message as an explanatory string and with \c error_number
    /// providing a more detailed indication of the kind of error that occurred. Immediately logs the error using
    /// \c logger.
    /// \param message An explanatory string.
    /// \param error_number Indicates the kind of error, typically provided by the platform native API.
    /// \param logger The logger used to log the error.
    api_error(const std::string& message, int error_number, const std::shared_ptr<spdlog::logger>& logger) noexcept;

    /// \brief Indicates the kind of error that occurred. The value is typically provided by the platform native API.
    const int error_number_;

};

// Defines a platform-agnostic connection reset error code.
#ifdef WPWRAPPER_WIN
#define WPCONNRESET WSAECONNRESET
#elif WPWRAPPER_POSIX
#define WPCONNRESET ECONNRESET
#endif

} // namespace wpwrapper

#endif // WPWRAPPER_EXCEPTIONS_H
