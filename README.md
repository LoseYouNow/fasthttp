
# FastHTTP - Single Header C++ HTTP Client Library
<img width="200" height="200" alt="fasthttp" src="https://github.com/user-attachments/assets/b399c309-2f45-4b43-b6d1-d313bc718c91" />

FastHTTP is a lightweight, easy-to-use C++ HTTP client library that requires only including a single header file. Supports HTTP and HTTPS requests and is compatible with C++14 standard.

## Features

- **Single Header File**: Only need to include `fasthttp.hpp`
- **Complete HTTP Support**: Supports GET, POST, PUT, DELETE, HEAD, OPTIONS, PATCH methods
- **HTTPS Support**: Uses WinINet on Windows and OpenSSL on Linux
- **Easy to Use**: Provides fluent API and global convenience functions
- **Custom Headers**: Support for setting arbitrary HTTP headers
- **JSON Support**: Built-in convenient methods for JSON requests
- **Authentication**: Supports Basic Auth and Bearer Token
- **Timeout Control**: Configurable request timeout
- **Exception Handling**: Comprehensive exception handling mechanism
- **Cross-Platform**: Supports Windows and Linux

## Quick Start

### Basic Usage

```cpp
#include "fasthttp.hpp"
#include <iostream>

int main() {
    try {
        // Simple GET request
        auto response = fasthttp::get("https://api.github.com/users/octocat");
        
        if (response.isSuccess()) {
            std::cout << "Response: " << response.getBody() << std::endl;
        }
    } catch (const fasthttp::HttpException& e) {
        std::cerr << "Request failed: " << e.what() << std::endl;
    }
    
    return 0;
}
```

### Using HttpClient Class

```cpp
#include "fasthttp.hpp"

int main() {
    fasthttp::HttpClient client;
    
    // Set default timeout
    client.setDefaultTimeout(10000); // 10 seconds
    
    // Set default headers
    client.setDefaultHeader("User-Agent", "MyApp/1.0");
    
    // Send request
    auto response = client.get("https://httpbin.org/get");
    
    return 0;
}
```

## API Reference

### Global Convenience Functions

```cpp
// GET request
fasthttp::HttpResponse get(const std::string& url);

// POST request
fasthttp::HttpResponse post(const std::string& url, const std::string& data = "");

// PUT request
fasthttp::HttpResponse put(const std::string& url, const std::string& data = "");

// DELETE request
fasthttp::HttpResponse del(const std::string& url);

// POST JSON data
fasthttp::HttpResponse postJson(const std::string& url, const std::string& json);
```

### HttpClient Class

```cpp
class HttpClient {
public:
    // Constructor and destructor
    HttpClient();
    ~HttpClient();
    
    // Configuration methods
    void setDefaultTimeout(int timeoutMs);
    void setDefaultHeader(const std::string& key, const std::string& value);
    
    // Request execution
    HttpResponse execute(const HttpRequest& request);
    
    // Convenience methods
    HttpResponse get(const std::string& url);
    HttpResponse post(const std::string& url, const std::string& data = "");
    HttpResponse put(const std::string& url, const std::string& data = "");
    HttpResponse del(const std::string& url);
    HttpResponse patch(const std::string& url, const std::string& data = "");
    
    // JSON methods
    HttpResponse postJson(const std::string& url, const std::string& json);
    HttpResponse putJson(const std::string& url, const std::string& json);
};
```

### HttpRequest Class

```cpp
class HttpRequest {
public:
    // Constructor
    HttpRequest(Method method, const std::string& url);
    
    // Fluent configuration methods
    HttpRequest& setBody(const std::string& body);
    HttpRequest& setTimeout(int timeoutMs);
    HttpRequest& setHeader(const std::string& key, const std::string& value);
    HttpRequest& addHeader(const std::string& key, const std::string& value);
    
    // Content type settings
    HttpRequest& setContentType(const std::string& contentType);
    HttpRequest& setJsonContent();
    HttpRequest& setFormContent();
    
    // Authentication
    HttpRequest& setBasicAuth(const std::string& username, const std::string& password);
    HttpRequest& setBearerToken(const std::string& token);
};
```

### HttpResponse Class

```cpp
class HttpResponse {
public:
    // Status retrieval
    int getStatusCode() const;
    const std::string& getStatusMessage() const;
    const std::string& getBody() const;
    
    // Header operations
    std::string getHeader(const std::string& key) const;
    bool hasHeader(const std::string& key) const;
    const std::map<std::string, std::string>& getHeaders() const;
    
    // Status checks
    bool isSuccess() const;        // 2xx status codes
    bool isRedirect() const;       // 3xx status codes
    bool isClientError() const;    // 4xx status codes
    bool isServerError() const;    // 5xx status codes
    
    // Convert to string
    std::string toString() const;
};
```

## Usage Examples

### Sending JSON Data

```cpp
fasthttp::HttpClient client;

std::string jsonData = R"({
    "name": "John Doe",
    "age": 25,
    "email": "john.doe@example.com"
})";

auto response = client.postJson("https://api.example.com/users", jsonData);

if (response.isSuccess()) {
    std::cout << "User created successfully: " << response.getBody() << std::endl;
} else {
    std::cout << "Creation failed: " << response.getStatusCode() << std::endl;
}
```

### Using Authentication

```cpp
fasthttp::HttpRequest request(fasthttp::Method::GET, "https://api.example.com/protected");

// Basic authentication
request.setBasicAuth("username", "password");

// Or use Bearer Token
// request.setBearerToken("your-jwt-token");

fasthttp::HttpClient client;
auto response = client.execute(request);
```

### Custom Headers

```cpp
fasthttp::HttpRequest request(fasthttp::Method::POST, "https://api.example.com/data");

request.setHeader("Content-Type", "application/xml")
       .setHeader("X-API-Key", "your-api-key")
       .setHeader("Accept", "application/json")
       .setBody("<xml>data</xml>");

fasthttp::HttpClient client;
auto response = client.execute(request);
```

### Error Handling

```cpp
try {
    auto response = fasthttp::get("https://invalid-url.example.com");
    
    if (response.isSuccess()) {
        std::cout << "Request successful" << std::endl;
    } else if (response.isClientError()) {
        std::cout << "Client error: " << response.getStatusCode() << std::endl;
    } else if (response.isServerError()) {
        std::cout << "Server error: " << response.getStatusCode() << std::endl;
    }
    
} catch (const fasthttp::NetworkException& e) {
    std::cerr << "Network error: " << e.what() << std::endl;
} catch (const fasthttp::TimeoutException& e) {
    std::cerr << "Request timeout: " << e.what() << std::endl;
} catch (const fasthttp::HttpException& e) {
    std::cerr << "HTTP error: " << e.what() << std::endl;
}
```

## Build Instructions

### Windows (Visual Studio)

```bash
# Direct compilation, library automatically links required Windows libraries
cl /EHsc /std:c++14 example.cpp
```

### Linux

```bash
# Requires linking OpenSSL library for HTTPS support
g++ -std=c++14 -o example example.cpp -lssl -lcrypto
```

### CMake Example

```cmake
cmake_minimum_required(VERSION 3.10)
project(MyProject)

set(CMAKE_CXX_STANDARD 14)

if(WIN32)
    # Windows doesn't require additional libraries
    add_executable(myapp main.cpp)
else()
    # Linux requires OpenSSL
    find_package(OpenSSL REQUIRED)
    add_executable(myapp main.cpp)
    target_link_libraries(myapp OpenSSL::SSL OpenSSL::Crypto)
endif()
```

## Dependencies

- **Windows**: Uses WinINet API, no additional dependencies required
- **Linux**: Requires OpenSSL library for HTTPS support
- **C++ Standard**: Requires C++14 or higher

## License

MIT License

## Contributing

Welcome to submit Issues and Pull Requests to improve this library.

---

# FastHTTP - 单头文件C++ HTTP客户端库

FastHTTP是一个轻量级、易用的C++ HTTP客户端库，只需要包含一个头文件即可使用。支持HTTP和HTTPS请求，兼容C++14标准。

## 特性

- **单头文件**: 只需包含 `fasthttp.hpp` 即可使用
- **完整的HTTP支持**: 支持GET, POST, PUT, DELETE, HEAD, OPTIONS, PATCH方法
- **HTTPS支持**: 在Windows上使用WinINet，在Linux上使用OpenSSL
- **简单易用**: 提供链式API和全局便捷函数
- **自定义请求头**: 支持设置任意HTTP请求头
- **JSON支持**: 内置JSON请求便捷方法
- **身份验证**: 支持Basic Auth和Bearer Token
- **超时控制**: 可配置请求超时时间
- **异常处理**: 完善的异常处理机制
- **跨平台**: 支持Windows和Linux

## 快速开始

### 基本用法

```cpp
#include "fasthttp.hpp"
#include <iostream>

int main() {
    try {
        // 简单的GET请求
        auto response = fasthttp::get("https://api.github.com/users/octocat");
        
        if (response.isSuccess()) {
            std::cout << "响应: " << response.getBody() << std::endl;
        }
    } catch (const fasthttp::HttpException& e) {
        std::cerr << "请求失败: " << e.what() << std::endl;
    }
    
    return 0;
}
```

### 使用HttpClient类

```cpp
#include "fasthttp.hpp"

int main() {
    fasthttp::HttpClient client;
    
    // 设置默认超时时间
    client.setDefaultTimeout(10000); // 10秒
    
    // 设置默认请求头
    client.setDefaultHeader("User-Agent", "MyApp/1.0");
    
    // 发送请求
    auto response = client.get("https://httpbin.org/get");
    
    return 0;
}
```

## API参考

### 全局便捷函数

```cpp
// GET请求
fasthttp::HttpResponse get(const std::string& url);

// POST请求
fasthttp::HttpResponse post(const std::string& url, const std::string& data = "");

// PUT请求
fasthttp::HttpResponse put(const std::string& url, const std::string& data = "");

// DELETE请求
fasthttp::HttpResponse del(const std::string& url);

// POST JSON数据
fasthttp::HttpResponse postJson(const std::string& url, const std::string& json);
```

### HttpClient类

```cpp
class HttpClient {
public:
    // 构造函数和析构函数
    HttpClient();
    ~HttpClient();
    
    // 配置方法
    void setDefaultTimeout(int timeoutMs);
    void setDefaultHeader(const std::string& key, const std::string& value);
    
    // 请求执行
    HttpResponse execute(const HttpRequest& request);
    
    // 便捷方法
    HttpResponse get(const std::string& url);
    HttpResponse post(const std::string& url, const std::string& data = "");
    HttpResponse put(const std::string& url, const std::string& data = "");
    HttpResponse del(const std::string& url);
    HttpResponse patch(const std::string& url, const std::string& data = "");
    
    // JSON方法
    HttpResponse postJson(const std::string& url, const std::string& json);
    HttpResponse putJson(const std::string& url, const std::string& json);
};
```

### HttpRequest类

```cpp
class HttpRequest {
public:
    // 构造函数
    HttpRequest(Method method, const std::string& url);
    
    // 链式配置方法
    HttpRequest& setBody(const std::string& body);
    HttpRequest& setTimeout(int timeoutMs);
    HttpRequest& setHeader(const std::string& key, const std::string& value);
    HttpRequest& addHeader(const std::string& key, const std::string& value);
    
    // 内容类型设置
    HttpRequest& setContentType(const std::string& contentType);
    HttpRequest& setJsonContent();
    HttpRequest& setFormContent();
    
    // 身份验证
    HttpRequest& setBasicAuth(const std::string& username, const std::string& password);
    HttpRequest& setBearerToken(const std::string& token);
};
```

### HttpResponse类

```cpp
class HttpResponse {
public:
    // 状态获取
    int getStatusCode() const;
    const std::string& getStatusMessage() const;
    const std::string& getBody() const;
    
    // 请求头操作
    std::string getHeader(const std::string& key) const;
    bool hasHeader(const std::string& key) const;
    const std::map<std::string, std::string>& getHeaders() const;
    
    // 状态检查
    bool isSuccess() const;        // 2xx状态码
    bool isRedirect() const;       // 3xx状态码
    bool isClientError() const;    // 4xx状态码
    bool isServerError() const;    // 5xx状态码
    
    // 转换为字符串
    std::string toString() const;
};
```

## 使用示例

### 发送JSON数据

```cpp
fasthttp::HttpClient client;

std::string jsonData = R"({
    "name": "张三",
    "age": 25,
    "email": "zhangsan@example.com"
})";

auto response = client.postJson("https://api.example.com/users", jsonData);

if (response.isSuccess()) {
    std::cout << "用户创建成功: " << response.getBody() << std::endl;
} else {
    std::cout << "创建失败: " << response.getStatusCode() << std::endl;
}
```

### 使用身份验证

```cpp
fasthttp::HttpRequest request(fasthttp::Method::GET, "https://api.example.com/protected");

// Basic认证
request.setBasicAuth("username", "password");

// 或者使用Bearer Token
// request.setBearerToken("your-jwt-token");

fasthttp::HttpClient client;
auto response = client.execute(request);
```

### 自定义请求头

```cpp
fasthttp::HttpRequest request(fasthttp::Method::POST, "https://api.example.com/data");

request.setHeader("Content-Type", "application/xml")
       .setHeader("X-API-Key", "your-api-key")
       .setHeader("Accept", "application/json")
       .setBody("<xml>data</xml>");

fasthttp::HttpClient client;
auto response = client.execute(request);
```

### 错误处理

```cpp
try {
    auto response = fasthttp::get("https://invalid-url.example.com");
    
    if (response.isSuccess()) {
        std::cout << "请求成功" << std::endl;
    } else if (response.isClientError()) {
        std::cout << "客户端错误: " << response.getStatusCode() << std::endl;
    } else if (response.isServerError()) {
        std::cout << "服务器错误: " << response.getStatusCode() << std::endl;
    }
    
} catch (const fasthttp::NetworkException& e) {
    std::cerr << "网络错误: " << e.what() << std::endl;
} catch (const fasthttp::TimeoutException& e) {
    std::cerr << "请求超时: " << e.what() << std::endl;
} catch (const fasthttp::HttpException& e) {
    std::cerr << "HTTP错误: " << e.what() << std::endl;
}
```

## 编译说明

### Windows (Visual Studio)

```bash
# 直接编译，库会自动链接所需的Windows库
cl /EHsc /std:c++14 example.cpp
```

### Linux

```bash
# 需要链接OpenSSL库用于HTTPS支持
g++ -std=c++14 -o example example.cpp -lssl -lcrypto
```

### CMake示例

```cmake
cmake_minimum_required(VERSION 3.10)
project(MyProject)

set(CMAKE_CXX_STANDARD 14)

if(WIN32)
    # Windows不需要额外库
    add_executable(myapp main.cpp)
else()
    # Linux需要OpenSSL
    find_package(OpenSSL REQUIRED)
    add_executable(myapp main.cpp)
    target_link_libraries(myapp OpenSSL::SSL OpenSSL::Crypto)
endif()
```

## 依赖说明

- **Windows**: 使用WinINet API，无需额外依赖
- **Linux**: 需要OpenSSL库用于HTTPS支持
- **C++标准**: 需要C++14或更高版本

## 许可证

MIT License

## 贡献

欢迎提交Issue和Pull Request来改进这个库。

