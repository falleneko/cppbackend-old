#include "http_server.h"

#include <boost/asio/dispatch.hpp>
#include <iostream>

namespace http_server {
    const std::unordered_map<std::string_view, HttpMethod> MethodToString = {
        {"GET"sv, HttpMethod::GET},
        {"POST"sv, HttpMethod::POST},
        {"HEAD"sv, HttpMethod::HEAD},
    };

    HttpMethod StringAsMethod(std::string_view method) {
        auto it = MethodToString.find(method);
        if (it != MethodToString.end()) {
            return it->second;
        }
        return HttpMethod::UNKNOWN;
    }

    void ReportError(beast::error_code ec, std::string_view what) {
        std::cerr << what << ": "sv << ec.message() << std::endl;
    }

    void SessionBase::Run() {
        // Вызываем метод Read, используя executor объекта stream_.
        // Таким образом вся работа со stream_ будет выполняться, используя его executor
        net::dispatch(stream_.get_executor(),
                    beast::bind_front_handler(&SessionBase::Read, GetSharedThis()));
    }

}  // namespace http_server
