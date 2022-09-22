#include "exceptions.h"

#include <fmt/format.h>

namespace wpwrapper {

api_error::api_error(const std::string& message) noexcept
        :std::runtime_error(message),
         error_number_(0)
{
}

api_error::api_error(const std::string& message, int error_number) noexcept
        :std::runtime_error(fmt::format("{} (error code: {})", message, error_number)),
         error_number_(error_number)
{
}

api_error::api_error(const std::string& message, int error_number, const std::shared_ptr<spdlog::logger>& logger) noexcept
        :api_error(message, error_number)
{
    logger->error("{} (error code: {})", message, error_number);
}

} // namespace wpwrapper
