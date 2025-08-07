// The target platform: Windows and Linux
// The library dependencies: OpenSSL, WinINet (for Windows), standard C++ libraries
// Copyright: (C) 2025 LoseYouNow
// Github: github.com/LoseYouNow
// License: MIT License

#ifndef FASTHTTP_HPP
#define FASTHTTP_HPP

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <functional>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <thread>
#include <mutex>
#include <chrono>
#include <iomanip>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <wininet.h>
    #pragma comment(lib, "ws2_32.lib")
    #pragma comment(lib, "wininet.lib")
    // 防止Windows宏定义冲突
    #ifdef DELETE
    #undef DELETE
    #endif
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <openssl/ssl.h>
    #include <openssl/err.h>
#endif

namespace fasthttp {

// Forward declarations
class HttpResponse;
class HttpRequest;
class HttpClient;
class FormData;
class UrlEncoder;
class Cookie;
class RequestBuilder;

// HTTP Methods
enum class Method {
    GET,
    POST,
    PUT,
    DELETE_METHOD,  // 改名避免与Windows DELETE宏冲突
    HEAD,
    OPTIONS,
    PATCH,
    TRACE,
    CONNECT
};

// Request/Response body types
enum class ContentType {
    TextPlain,
    TextHtml,
    ApplicationJson,
    ApplicationXml,
    ApplicationFormUrlencoded,
    MultipartFormData,
    ApplicationOctetStream,
    ImagePng,
    ImageJpeg,
    ImageGif
};

// Exception classes
class HttpException : public std::runtime_error {
public:
    explicit HttpException(const std::string& message) : std::runtime_error(message) {}
};

class NetworkException : public HttpException {
public:
    explicit NetworkException(const std::string& message) : HttpException("Network Error: " + message) {}
};

class TimeoutException : public HttpException {
public:
    explicit TimeoutException() : HttpException("Request timeout") {}
};

// Utility functions
inline std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

inline std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

// Utility functions for content types
inline std::string getContentTypeString(ContentType type) {
    switch (type) {
        case ContentType::TextPlain: return "text/plain";
        case ContentType::TextHtml: return "text/html";
        case ContentType::ApplicationJson: return "application/json";
        case ContentType::ApplicationXml: return "application/xml";
        case ContentType::ApplicationFormUrlencoded: return "application/x-www-form-urlencoded";
        case ContentType::MultipartFormData: return "multipart/form-data";
        case ContentType::ApplicationOctetStream: return "application/octet-stream";
        case ContentType::ImagePng: return "image/png";
        case ContentType::ImageJpeg: return "image/jpeg";
        case ContentType::ImageGif: return "image/gif";
        default: return "application/octet-stream";
    }
}

// URL parsing utility
class URL {
public:
    std::string scheme;
    std::string host;
    int port;
    std::string path;
    std::string query;
    std::string fragment;

    URL() : port(80) {}

    static URL parse(const std::string& url) {
        URL result;
        std::string temp = url;

        // Parse scheme
        size_t schemePos = temp.find("://");
        if (schemePos != std::string::npos) {
            result.scheme = temp.substr(0, schemePos);
            temp = temp.substr(schemePos + 3);
            if (result.scheme == "https") {
                result.port = 443;
            }
        }

        // Parse host and port
        size_t pathPos = temp.find('/');
        std::string hostPort = (pathPos != std::string::npos) ? temp.substr(0, pathPos) : temp;
        
        size_t portPos = hostPort.find(':');
        if (portPos != std::string::npos) {
            result.host = hostPort.substr(0, portPos);
            result.port = std::stoi(hostPort.substr(portPos + 1));
        } else {
            result.host = hostPort;
        }

        // Parse path, query, and fragment
        if (pathPos != std::string::npos) {
            temp = temp.substr(pathPos);
            size_t queryPos = temp.find('?');
            size_t fragmentPos = temp.find('#');

            if (queryPos != std::string::npos) {
                result.path = temp.substr(0, queryPos);
                if (fragmentPos != std::string::npos) {
                    result.query = temp.substr(queryPos + 1, fragmentPos - queryPos - 1);
                    result.fragment = temp.substr(fragmentPos + 1);
                } else {
                    result.query = temp.substr(queryPos + 1);
                }
            } else if (fragmentPos != std::string::npos) {
                result.path = temp.substr(0, fragmentPos);
                result.fragment = temp.substr(fragmentPos + 1);
            } else {
                result.path = temp;
            }
        } else {
            result.path = "/";
        }

        return result;
    }
};

// URL encoding utility
class UrlEncoder {
public:
    static std::string encode(const std::string& value) {
        std::ostringstream encoded;
        encoded.fill('0');
        encoded << std::hex;

        for (char c : value) {
            if (isAlphaNumeric(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                encoded << c;
            } else {
                encoded << std::uppercase;
                encoded << '%' << std::setw(2) << int(static_cast<unsigned char>(c));
                encoded << std::nouppercase;
            }
        }

        return encoded.str();
    }

    static std::string decode(const std::string& value) {
        std::string decoded;
        for (size_t i = 0; i < value.length(); ++i) {
            if (value[i] == '%') {
                if (i + 2 < value.length()) {
                    int hexValue;
                    std::istringstream iss(value.substr(i + 1, 2));
                    if (iss >> std::hex >> hexValue) {
                        decoded += static_cast<char>(hexValue);
                        i += 2;
                    } else {
                        decoded += value[i];
                    }
                } else {
                    decoded += value[i];
                }
            } else if (value[i] == '+') {
                decoded += ' ';
            } else {
                decoded += value[i];
            }
        }
        return decoded;
    }

    static std::string buildQueryString(const std::map<std::string, std::string>& params) {
        std::ostringstream oss;
        bool first = true;
        for (const auto& param : params) {
            if (!first) oss << "&";
            oss << encode(param.first) << "=" << encode(param.second);
            first = false;
        }
        return oss.str();
    }

private:
    static bool isAlphaNumeric(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
    }
};

// Cookie handling
class Cookie {
public:
    std::string name;
    std::string value;
    std::string domain;
    std::string path;
    bool secure;
    bool httpOnly;
    std::string sameSite;

    Cookie() : secure(false), httpOnly(false) {}

    Cookie(const std::string& name, const std::string& value) 
        : name(name), value(value), secure(false), httpOnly(false) {}

    std::string toString() const {
        std::ostringstream oss;
        oss << name << "=" << value;
        if (!domain.empty()) oss << "; Domain=" << domain;
        if (!path.empty()) oss << "; Path=" << path;
        if (secure) oss << "; Secure";
        if (httpOnly) oss << "; HttpOnly";
        if (!sameSite.empty()) oss << "; SameSite=" << sameSite;
        return oss.str();
    }

    static Cookie parse(const std::string& cookieString) {
        Cookie cookie;
        std::istringstream iss(cookieString);
        std::string item;
        
        // Parse name=value first
        if (std::getline(iss, item, ';')) {
            size_t equalPos = item.find('=');
            if (equalPos != std::string::npos) {
                cookie.name = trim(item.substr(0, equalPos));
                cookie.value = trim(item.substr(equalPos + 1));
            }
        }

        // Parse attributes
        while (std::getline(iss, item, ';')) {
            item = trim(item);
            if (item.find("Domain=") == 0) {
                cookie.domain = item.substr(7);
            } else if (item.find("Path=") == 0) {
                cookie.path = item.substr(5);
            } else if (item == "Secure") {
                cookie.secure = true;
            } else if (item == "HttpOnly") {
                cookie.httpOnly = true;
            } else if (item.find("SameSite=") == 0) {
                cookie.sameSite = item.substr(9);
            }
        }

        return cookie;
    }
};

// Form data handling
class FormData {
private:
    std::map<std::string, std::string> fields_;
    std::string boundary_;

public:
    FormData() {
        // Generate a random boundary
        boundary_ = "----FastHTTPBoundary" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    }

    FormData& addField(const std::string& name, const std::string& value) {
        fields_[name] = value;
        return *this;
    }

    std::string getBoundary() const {
        return boundary_;
    }

    std::string getContentType() const {
        return "multipart/form-data; boundary=" + boundary_;
    }

    std::string encode() const {
        std::ostringstream oss;
        for (const auto& field : fields_) {
            oss << "--" << boundary_ << "\r\n";
            oss << "Content-Disposition: form-data; name=\"" << field.first << "\"\r\n\r\n";
            oss << field.second << "\r\n";
        }
        oss << "--" << boundary_ << "--\r\n";
        return oss.str();
    }
};

// HTTP Response class
class HttpResponse {
private:
    int statusCode_;
    std::string statusMessage_;
    std::map<std::string, std::string> headers_;
    std::string body_;
    std::vector<Cookie> cookies_;

public:
    HttpResponse() : statusCode_(0) {}

    HttpResponse(int statusCode, const std::string& statusMessage)
        : statusCode_(statusCode), statusMessage_(statusMessage) {}

    // Getters
    int getStatusCode() const { return statusCode_; }
    const std::string& getStatusMessage() const { return statusMessage_; }
    const std::map<std::string, std::string>& getHeaders() const { return headers_; }
    const std::string& getBody() const { return body_; }
    const std::vector<Cookie>& getCookies() const { return cookies_; }

    // Setters
    void setStatusCode(int code) { statusCode_ = code; }
    void setStatusMessage(const std::string& message) { statusMessage_ = message; }
    void setBody(const std::string& body) { body_ = body; }

    // Header operations
    void setHeader(const std::string& key, const std::string& value) {
        std::string lowerKey = toLower(key);
        headers_[lowerKey] = value;
        
        // Parse cookies from Set-Cookie headers
        if (lowerKey == "set-cookie") {
            cookies_.push_back(Cookie::parse(value));
        }
    }

    std::string getHeader(const std::string& key) const {
        auto it = headers_.find(toLower(key));
        return it != headers_.end() ? it->second : "";
    }

    bool hasHeader(const std::string& key) const {
        return headers_.find(toLower(key)) != headers_.end();
    }

    // Cookie operations
    std::vector<Cookie> getCookiesByName(const std::string& name) const {
        std::vector<Cookie> result;
        for (const auto& cookie : cookies_) {
            if (cookie.name == name) {
                result.push_back(cookie);
            }
        }
        return result;
    }

    Cookie getCookie(const std::string& name) const {
        for (const auto& cookie : cookies_) {
            if (cookie.name == name) {
                return cookie;
            }
        }
        return Cookie(); // Return empty cookie if not found
    }

    bool hasCookie(const std::string& name) const {
        for (const auto& cookie : cookies_) {
            if (cookie.name == name) {
                return true;
            }
        }
        return false;
    }

    // Utility methods
    bool isSuccess() const {
        return statusCode_ >= 200 && statusCode_ < 300;
    }

    bool isRedirect() const {
        return statusCode_ >= 300 && statusCode_ < 400;
    }

    bool isClientError() const {
        return statusCode_ >= 400 && statusCode_ < 500;
    }

    bool isServerError() const {
        return statusCode_ >= 500 && statusCode_ < 600;
    }

    bool isInformational() const {
        return statusCode_ >= 100 && statusCode_ < 200;
    }

    // Get status code category as string
    std::string getStatusCategory() const {
        if (isInformational()) return "Informational";
        if (isSuccess()) return "Success";
        if (isRedirect()) return "Redirect";
        if (isClientError()) return "Client Error";
        if (isServerError()) return "Server Error";
        return "Unknown";
    }

    // Content type helpers
    std::string getContentType() const {
        return getHeader("content-type");
    }

    size_t getContentLength() const {
        std::string lengthStr = getHeader("content-length");
        if (!lengthStr.empty()) {
            try {
                return std::stoull(lengthStr);
            } catch (...) {
                return 0;
            }
        }
        return body_.length();
    }

    std::string getContentEncoding() const {
        return getHeader("content-encoding");
    }

    // JSON helper (simple check)
    bool isJson() const {
        std::string contentType = getContentType();
        return contentType.find("application/json") != std::string::npos;
    }

    // XML helper (simple check)
    bool isXml() const {
        std::string contentType = getContentType();
        return contentType.find("application/xml") != std::string::npos || 
               contentType.find("text/xml") != std::string::npos;
    }

    // HTML helper (simple check)
    bool isHtml() const {
        std::string contentType = getContentType();
        return contentType.find("text/html") != std::string::npos;
    }

    // Get summary for debugging
    std::string getSummary() const {
        std::ostringstream oss;
        oss << "HTTP " << statusCode_ << " " << statusMessage_ << "\n";
        oss << "Content-Type: " << getContentType() << "\n";
        oss << "Content-Length: " << getContentLength() << "\n";
        return oss.str();
    }

    // Convert to string (for debugging)
    std::string toString() const {
        std::ostringstream oss;
        oss << "HTTP " << statusCode_ << " " << statusMessage_ << "\n";
        for (const auto& header : headers_) {
            oss << header.first << ": " << header.second << "\n";
        }
        oss << "\n" << body_;
        return oss.str();
    }
};

// HTTP request class
class HttpRequest {
private:
    Method method_;
    std::string url_;
    std::map<std::string, std::string> headers_;
    std::string body_;
    int timeout_;
    std::vector<Cookie> cookies_;

    std::string base64Encode(const std::string& input) const {
        const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string encoded;
        int val = 0, valb = -6;
        for (unsigned char c : input) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                encoded.push_back(chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) {
            encoded.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
        }
        while (encoded.size() % 4) {
            encoded.push_back('=');
        }
        return encoded;
    }

public:
    HttpRequest(Method method, const std::string& url)
        : method_(method), url_(url), timeout_(30000) {}

    // Getters
    Method getMethod() const { return method_; }
    const std::string& getUrl() const { return url_; }
    const std::map<std::string, std::string>& getHeaders() const { return headers_; }
    const std::string& getBody() const { return body_; }
    int getTimeout() const { return timeout_; }
    const std::vector<Cookie>& getCookies() const { return cookies_; }

    // Setters
    HttpRequest& setMethod(Method method) { method_ = method; return *this; }
    HttpRequest& setUrl(const std::string& url) { url_ = url; return *this; }
    HttpRequest& setBody(const std::string& body) { body_ = body; return *this; }
    HttpRequest& setTimeout(int timeoutMs) { timeout_ = timeoutMs; return *this; }

    // Header operations
    HttpRequest& setHeader(const std::string& key, const std::string& value) {
        headers_[key] = value;
        return *this;
    }

    HttpRequest& addHeader(const std::string& key, const std::string& value) {
        return setHeader(key, value);
    }

    std::string getHeader(const std::string& key) const {
        auto it = headers_.find(key);
        return it != headers_.end() ? it->second : "";
    }

    bool hasHeader(const std::string& key) const {
        return headers_.find(key) != headers_.end();
    }

    // Cookie operations
    HttpRequest& addCookie(const Cookie& cookie) {
        cookies_.push_back(cookie);
        return *this;
    }

    HttpRequest& addCookie(const std::string& name, const std::string& value) {
        cookies_.emplace_back(name, value);
        return *this;
    }

    // Content type setters
    HttpRequest& setContentType(const std::string& contentType) {
        return setHeader("Content-Type", contentType);
    }

    HttpRequest& setContentType(ContentType type) {
        return setContentType(getContentTypeString(type));
    }

    HttpRequest& setJsonContent() {
        return setContentType(ContentType::ApplicationJson);
    }

    HttpRequest& setFormContent() {
        return setContentType(ContentType::ApplicationFormUrlencoded);
    }

    // Authentication
    HttpRequest& setBasicAuth(const std::string& username, const std::string& password) {
        std::string credentials = username + ":" + password;
        std::string encoded = base64Encode(credentials);
        return setHeader("Authorization", "Basic " + encoded);
    }

    HttpRequest& setBearerToken(const std::string& token) {
        return setHeader("Authorization", "Bearer " + token);
    }

    // Convert to string (for debugging)
    std::string toString() const {
        std::ostringstream oss;
        oss << "HttpRequest:\n";
        oss << "Method: " << static_cast<int>(method_) << "\n";
        oss << "URL: " << url_ << "\n";
        oss << "Headers: " << headers_.size() << "\n";
        for (const auto& header : headers_) {
            oss << "  " << header.first << ": " << header.second << "\n";
        }
        oss << "Body: " << body_.size() << " bytes\n";
        oss << "Timeout: " << timeout_ << " ms\n";
        oss << "Cookies: " << cookies_.size() << "\n";
        for (const auto& cookie : cookies_) {
            oss << "  " << cookie.name << "=" << cookie.value << "\n";
        }
        return oss.str();
    }
};

// RequestBuilder class
class RequestBuilder {
private:
    Method method_;
    std::string url_;
    std::map<std::string, std::string> headers_;
    std::string body_;
    int timeout_;
    std::vector<Cookie> cookies_;

    std::string base64Encode(const std::string& input) const {
        const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string encoded;
        int val = 0, valb = -6;
        for (unsigned char c : input) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                encoded.push_back(chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) {
            encoded.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
        }
        while (encoded.size() % 4) {
            encoded.push_back('=');
        }
        return encoded;
    }

public:
    RequestBuilder(Method method, const std::string& url) 
        : method_(method), url_(url), timeout_(30000) {}

    RequestBuilder& addHeader(const std::string& key, const std::string& value) {
        headers_[key] = value;
        return *this;
    }

    RequestBuilder& setContentType(ContentType type) {
        return addHeader("Content-Type", getContentTypeString(type));
    }

    RequestBuilder& setBody(const std::string& body) {
        body_ = body;
        if (!body.empty()) {
            addHeader("Content-Length", std::to_string(body.size()));
        }
        return *this;
    }

    RequestBuilder& setJsonBody(const std::string& json) {
        setContentType(ContentType::ApplicationJson);
        return setBody(json);
    }

    RequestBuilder& setFormData(const FormData& formData) {
        addHeader("Content-Type", formData.getContentType());
        return setBody(formData.encode());
    }

    RequestBuilder& setFormUrlEncoded(const std::map<std::string, std::string>& params) {
        setContentType(ContentType::ApplicationFormUrlencoded);
        return setBody(UrlEncoder::buildQueryString(params));
    }

    RequestBuilder& addQueryParam(const std::string& key, const std::string& value) {
        std::string separator = (url_.find('?') != std::string::npos) ? "&" : "?";
        url_ += separator + UrlEncoder::encode(key) + "=" + UrlEncoder::encode(value);
        return *this;
    }

    RequestBuilder& addQueryParams(const std::map<std::string, std::string>& params) {
        for (const auto& param : params) {
            addQueryParam(param.first, param.second);
        }
        return *this;
    }

    RequestBuilder& setTimeout(int timeoutMs) {
        timeout_ = timeoutMs;
        return *this;
    }

    RequestBuilder& addCookie(const Cookie& cookie) {
        cookies_.push_back(cookie);
        return *this;
    }

    RequestBuilder& addCookie(const std::string& name, const std::string& value) {
        return addCookie(Cookie(name, value));
    }

    RequestBuilder& setBasicAuth(const std::string& username, const std::string& password) {
        std::string credentials = username + ":" + password;
        std::string encoded = base64Encode(credentials);
        return addHeader("Authorization", "Basic " + encoded);
    }

    RequestBuilder& setBearerToken(const std::string& token) {
        return addHeader("Authorization", "Bearer " + token);
    }

    HttpRequest build() {
        HttpRequest request(method_, url_);
        
        // Set headers
        for (const auto& header : headers_) {
            request.setHeader(header.first, header.second);
        }

        // Set cookies
        if (!cookies_.empty()) {
            std::ostringstream cookieHeader;
            for (size_t i = 0; i < cookies_.size(); ++i) {
                if (i > 0) cookieHeader << "; ";
                cookieHeader << cookies_[i].name << "=" << cookies_[i].value;
            }
            request.setHeader("Cookie", cookieHeader.str());
        }

        // Set body and timeout
        request.setBody(body_);
        request.setTimeout(timeout_);

        return request;
    }
};

// Forward declaration for HttpClient method implementations
class HttpClient {
private:
    int defaultTimeout_;
    std::map<std::string, std::string> defaultHeaders_;

#ifdef _WIN32
    HINTERNET hSession_;
#endif

public:
    HttpClient();
    ~HttpClient();

    // Configuration
    void setDefaultTimeout(int timeoutMs);
    void setDefaultHeader(const std::string& key, const std::string& value);

    // Builder pattern methods
    RequestBuilder GET(const std::string& url);
    RequestBuilder POST(const std::string& url);
    RequestBuilder PUT(const std::string& url);
    RequestBuilder DELETE(const std::string& url);
    RequestBuilder PATCH(const std::string& url);
    RequestBuilder HEAD(const std::string& url);
    RequestBuilder OPTIONS(const std::string& url);

    // Simple HTTP methods
    HttpResponse get(const std::string& url, const std::map<std::string, std::string>& headers = {});
    HttpResponse post(const std::string& url, const std::string& data = "", const std::map<std::string, std::string>& headers = {});
    HttpResponse put(const std::string& url, const std::string& data = "", const std::map<std::string, std::string>& headers = {});
    HttpResponse del(const std::string& url, const std::map<std::string, std::string>& headers = {});
    HttpResponse patch(const std::string& url, const std::string& data = "", const std::map<std::string, std::string>& headers = {});
    HttpResponse head(const std::string& url, const std::map<std::string, std::string>& headers = {});
    HttpResponse options(const std::string& url, const std::map<std::string, std::string>& headers = {});

    // JSON methods
    HttpResponse postJson(const std::string& url, const std::string& json, const std::map<std::string, std::string>& headers = {});
    HttpResponse putJson(const std::string& url, const std::string& json, const std::map<std::string, std::string>& headers = {});
    HttpResponse patchJson(const std::string& url, const std::string& json, const std::map<std::string, std::string>& headers = {});

    // Form methods
    HttpResponse postForm(const std::string& url, const std::map<std::string, std::string>& formData, const std::map<std::string, std::string>& headers = {});
    HttpResponse postMultipart(const std::string& url, const FormData& formData, const std::map<std::string, std::string>& headers = {});

    // Main execution method
    HttpResponse execute(const HttpRequest& request);

private:
#ifdef _WIN32
    HttpResponse executeWindows(const HttpRequest& request);
    HttpResponse readWindowsResponse(HINTERNET hRequest);
    std::string getMethodString(Method method);
    void parseHeaders(const std::string& headerText, HttpResponse& response);
#else
    HttpResponse executeLinux(const HttpRequest& request);
#endif
};

// HttpClient implementation
inline HttpClient::HttpClient() : defaultTimeout_(30000) {
#ifdef _WIN32
    hSession_ = InternetOpenA("FastHTTP/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hSession_) {
        throw NetworkException("Failed to initialize WinINet session");
    }
#endif
}

inline HttpClient::~HttpClient() {
#ifdef _WIN32
    if (hSession_) {
        InternetCloseHandle(hSession_);
    }
#endif
}

inline void HttpClient::setDefaultTimeout(int timeoutMs) {
    defaultTimeout_ = timeoutMs;
}

inline void HttpClient::setDefaultHeader(const std::string& key, const std::string& value) {
    defaultHeaders_[key] = value;
}

inline RequestBuilder HttpClient::GET(const std::string& url) {
    return RequestBuilder(Method::GET, url);
}

inline RequestBuilder HttpClient::POST(const std::string& url) {
    return RequestBuilder(Method::POST, url);
}

inline RequestBuilder HttpClient::PUT(const std::string& url) {
    return RequestBuilder(Method::PUT, url);
}

inline RequestBuilder HttpClient::DELETE(const std::string& url) {
    return RequestBuilder(Method::DELETE_METHOD, url);
}

inline RequestBuilder HttpClient::PATCH(const std::string& url) {
    return RequestBuilder(Method::PATCH, url);
}

inline RequestBuilder HttpClient::HEAD(const std::string& url) {
    return RequestBuilder(Method::HEAD, url);
}

inline RequestBuilder HttpClient::OPTIONS(const std::string& url) {
    return RequestBuilder(Method::OPTIONS, url);
}

inline HttpResponse HttpClient::get(const std::string& url, const std::map<std::string, std::string>& headers) {
    HttpRequest request(Method::GET, url);
    for (const auto& header : headers) {
        request.setHeader(header.first, header.second);
    }
    return execute(request);
}

inline HttpResponse HttpClient::post(const std::string& url, const std::string& data, const std::map<std::string, std::string>& headers) {
    HttpRequest request(Method::POST, url);
    request.setBody(data);
    for (const auto& header : headers) {
        request.setHeader(header.first, header.second);
    }
    return execute(request);
}

inline HttpResponse HttpClient::put(const std::string& url, const std::string& data, const std::map<std::string, std::string>& headers) {
    HttpRequest request(Method::PUT, url);
    request.setBody(data);
    for (const auto& header : headers) {
        request.setHeader(header.first, header.second);
    }
    return execute(request);
}

inline HttpResponse HttpClient::del(const std::string& url, const std::map<std::string, std::string>& headers) {
    HttpRequest request(Method::DELETE_METHOD, url);
    for (const auto& header : headers) {
        request.setHeader(header.first, header.second);
    }
    return execute(request);
}

inline HttpResponse HttpClient::patch(const std::string& url, const std::string& data, const std::map<std::string, std::string>& headers) {
    HttpRequest request(Method::PATCH, url);
    request.setBody(data);
    for (const auto& header : headers) {
        request.setHeader(header.first, header.second);
    }
    return execute(request);
}

inline HttpResponse HttpClient::head(const std::string& url, const std::map<std::string, std::string>& headers) {
    HttpRequest request(Method::HEAD, url);
    for (const auto& header : headers) {
        request.setHeader(header.first, header.second);
    }
    return execute(request);
}

inline HttpResponse HttpClient::options(const std::string& url, const std::map<std::string, std::string>& headers) {
    HttpRequest request(Method::OPTIONS, url);
    for (const auto& header : headers) {
        request.setHeader(header.first, header.second);
    }
    return execute(request);
}

inline HttpResponse HttpClient::postJson(const std::string& url, const std::string& json, const std::map<std::string, std::string>& headers) {
    HttpRequest request(Method::POST, url);
    request.setJsonContent().setBody(json);
    for (const auto& header : headers) {
        request.setHeader(header.first, header.second);
    }
    return execute(request);
}

inline HttpResponse HttpClient::putJson(const std::string& url, const std::string& json, const std::map<std::string, std::string>& headers) {
    HttpRequest request(Method::PUT, url);
    request.setJsonContent().setBody(json);
    for (const auto& header : headers) {
        request.setHeader(header.first, header.second);
    }
    return execute(request);
}

inline HttpResponse HttpClient::patchJson(const std::string& url, const std::string& json, const std::map<std::string, std::string>& headers) {
    HttpRequest request(Method::PATCH, url);
    request.setJsonContent().setBody(json);
    for (const auto& header : headers) {
        request.setHeader(header.first, header.second);
    }
    return execute(request);
}

inline HttpResponse HttpClient::postForm(const std::string& url, const std::map<std::string, std::string>& formData, const std::map<std::string, std::string>& headers) {
    HttpRequest request(Method::POST, url);
    request.setFormContent().setBody(UrlEncoder::buildQueryString(formData));
    for (const auto& header : headers) {
        request.setHeader(header.first, header.second);
    }
    return execute(request);
}

inline HttpResponse HttpClient::postMultipart(const std::string& url, const FormData& formData, const std::map<std::string, std::string>& headers) {
    HttpRequest request(Method::POST, url);
    request.setHeader("Content-Type", formData.getContentType());
    request.setBody(formData.encode());
    for (const auto& header : headers) {
        request.setHeader(header.first, header.second);
    }
    return execute(request);
}

inline HttpResponse HttpClient::execute(const HttpRequest& request) {
#ifdef _WIN32
    return executeWindows(request);
#else
    return executeLinux(request);
#endif
}

#ifdef _WIN32
inline HttpResponse HttpClient::executeWindows(const HttpRequest& request) {
    URL url = URL::parse(request.getUrl());
    
    HINTERNET hConnect = InternetConnectA(hSession_, url.host.c_str(), url.port, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        throw NetworkException("Failed to connect to host: " + url.host);
    }

    std::string methodStr = getMethodString(request.getMethod());
    DWORD flags = (url.scheme == "https") ? INTERNET_FLAG_SECURE : 0;
    
    std::string fullPath = url.path;
    if (!url.query.empty()) {
        fullPath += "?" + url.query;
    }
    
    HINTERNET hRequest = HttpOpenRequestA(hConnect, methodStr.c_str(), fullPath.c_str(), NULL, NULL, NULL, flags, 0);
    if (!hRequest) {
        InternetCloseHandle(hConnect);
        throw NetworkException("Failed to create HTTP request");
    }

    // Add headers
    std::string headerStr;
    for (const auto& header : request.getHeaders()) {
        headerStr += header.first + ": " + header.second + "\r\n";
    }
    for (const auto& header : defaultHeaders_) {
        if (request.getHeaders().find(header.first) == request.getHeaders().end()) {
            headerStr += header.first + ": " + header.second + "\r\n";
        }
    }

    if (!headerStr.empty()) {
        HttpAddRequestHeadersA(hRequest, headerStr.c_str(), headerStr.length(), HTTP_ADDREQ_FLAG_ADD);
    }

    // Send request
    const std::string& body = request.getBody();
    BOOL result = HttpSendRequestA(hRequest, NULL, 0, 
                                   const_cast<char*>(body.c_str()), 
                                   body.length());
    
    if (!result) {
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        throw NetworkException("Failed to send HTTP request");
    }

    // Read response
    HttpResponse response = readWindowsResponse(hRequest);
    
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    
    return response;
}

inline HttpResponse HttpClient::readWindowsResponse(HINTERNET hRequest) {
    HttpResponse response;

    // Read status code
    DWORD statusCode = 0;
    DWORD bufferLength = sizeof(statusCode);
    HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, 
                   &statusCode, &bufferLength, NULL);
    response.setStatusCode(statusCode);

    // Read headers
    DWORD headerSize = 0;
    HttpQueryInfoA(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, NULL, &headerSize, NULL);
    if (headerSize > 0) {
        std::vector<char> headerBuffer(headerSize);
        if (HttpQueryInfoA(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, 
                           headerBuffer.data(), &headerSize, NULL)) {
            parseHeaders(std::string(headerBuffer.data()), response);
        }
    }

    // Read body
    std::string body;
    char buffer[4096];
    DWORD bytesRead = 0;
    while (InternetReadFile(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        body.append(buffer, bytesRead);
    }
    response.setBody(body);

    return response;
}

inline std::string HttpClient::getMethodString(Method method) {
    switch (method) {
        case Method::GET: return "GET";
        case Method::POST: return "POST";
        case Method::PUT: return "PUT";
        case Method::DELETE_METHOD: return "DELETE";
        case Method::HEAD: return "HEAD";
        case Method::OPTIONS: return "OPTIONS";
        case Method::PATCH: return "PATCH";
        case Method::TRACE: return "TRACE";
        case Method::CONNECT: return "CONNECT";
        default: return "GET";
    }
}

inline void HttpClient::parseHeaders(const std::string& headerText, HttpResponse& response) {
    std::istringstream iss(headerText);
    std::string line;
    
    while (std::getline(iss, line)) {
        line = trim(line);
        if (line.empty()) continue;
        
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = trim(line.substr(0, colonPos));
            std::string value = trim(line.substr(colonPos + 1));
            response.setHeader(key, value);
        }
    }
}

#else
inline HttpResponse HttpClient::executeLinux(const HttpRequest& request) {
    // Linux implementation using raw sockets or curl
    // This is a simplified implementation
    throw NetworkException("Linux implementation not yet available");
}
#endif

// Global convenience functions
inline HttpResponse get(const std::string& url, const std::map<std::string, std::string>& headers = {}) {
    HttpClient client;
    return client.get(url, headers);
}

inline HttpResponse post(const std::string& url, const std::string& data = "", const std::map<std::string, std::string>& headers = {}) {
    HttpClient client;
    return client.post(url, data, headers);
}

inline HttpResponse put(const std::string& url, const std::string& data = "", const std::map<std::string, std::string>& headers = {}) {
    HttpClient client;
    return client.put(url, data, headers);
}

inline HttpResponse del(const std::string& url, const std::map<std::string, std::string>& headers = {}) {
    HttpClient client;
    return client.del(url, headers);
}

inline HttpResponse patch(const std::string& url, const std::string& data = "", const std::map<std::string, std::string>& headers = {}) {
    HttpClient client;
    return client.patch(url, data, headers);
}

inline HttpResponse head(const std::string& url, const std::map<std::string, std::string>& headers = {}) {
    HttpClient client;
    return client.head(url, headers);
}

inline HttpResponse options(const std::string& url, const std::map<std::string, std::string>& headers = {}) {
    HttpClient client;
    return client.options(url, headers);
}

inline HttpResponse postJson(const std::string& url, const std::string& json, const std::map<std::string, std::string>& headers = {}) {
    HttpClient client;
    return client.postJson(url, json, headers);
}

inline HttpResponse putJson(const std::string& url, const std::string& json, const std::map<std::string, std::string>& headers = {}) {
    HttpClient client;
    return client.putJson(url, json, headers);
}

inline HttpResponse patchJson(const std::string& url, const std::string& json, const std::map<std::string, std::string>& headers = {}) {
    HttpClient client;
    return client.patchJson(url, json, headers);
}

inline HttpResponse postForm(const std::string& url, const std::map<std::string, std::string>& formData, const std::map<std::string, std::string>& headers = {}) {
    HttpClient client;
    return client.postForm(url, formData, headers);
}

inline HttpResponse postMultipart(const std::string& url, const FormData& formData, const std::map<std::string, std::string>& headers = {}) {
    HttpClient client;
    return client.postMultipart(url, formData, headers);
}

// Execute function for global use
inline HttpResponse execute(const HttpRequest& request) {
    HttpClient client;
    return client.execute(request);
}

// URL utility functions
inline std::string urlEncode(const std::string& value) {
    return UrlEncoder::encode(value);
}

inline std::string urlDecode(const std::string& value) {
    return UrlEncoder::decode(value);
}

inline std::string buildQueryString(const std::map<std::string, std::string>& params) {
    return UrlEncoder::buildQueryString(params);
}

// Builder pattern convenience functions
inline RequestBuilder GET(const std::string& url) {
    return RequestBuilder(Method::GET, url);
}

inline RequestBuilder POST(const std::string& url) {
    return RequestBuilder(Method::POST, url);
}

inline RequestBuilder PUT(const std::string& url) {
    return RequestBuilder(Method::PUT, url);
}

inline RequestBuilder DELETE(const std::string& url) {
    return RequestBuilder(Method::DELETE_METHOD, url);
}

inline RequestBuilder PATCH(const std::string& url) {
    return RequestBuilder(Method::PATCH, url);
}

inline RequestBuilder HEAD(const std::string& url) {
    return RequestBuilder(Method::HEAD, url);
}

inline RequestBuilder OPTIONS(const std::string& url) {
    return RequestBuilder(Method::OPTIONS, url);
}

} // namespace fasthttp

#endif // FASTHTTP_HPP