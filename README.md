# FastHTTP - ��ͷ�ļ�C++ HTTP�ͻ��˿�

FastHTTP��һ�������������õ�C++ HTTP�ͻ��˿⣬ֻ��Ҫ����һ��ͷ�ļ�����ʹ�á�֧��HTTP��HTTPS���󣬼���C++14��׼��

## ����

- **��ͷ�ļ�**: ֻ����� `fasthttp.hpp` ����ʹ��
- **������HTTP֧��**: ֧��GET, POST, PUT, DELETE, HEAD, OPTIONS, PATCH����
- **HTTPS֧��**: ��Windows��ʹ��WinINet����Linux��ʹ��OpenSSL
- **������**: �ṩ��ʽAPI��ȫ�ֱ�ݺ���
- **�Զ�������ͷ**: ֧����������HTTP����ͷ
- **JSON֧��**: ����JSON�����ݷ���
- **�����֤**: ֧��Basic Auth��Bearer Token
- **��ʱ����**: ����������ʱʱ��
- **�쳣����**: ���Ƶ��쳣�������
- **��ƽ̨**: ֧��Windows��Linux

## ���ٿ�ʼ

### �����÷�

```cpp
#include "fasthttp.hpp"
#include <iostream>

int main() {
    try {
        // �򵥵�GET����
        auto response = fasthttp::get("https://api.github.com/users/octocat");
        
        if (response.isSuccess()) {
            std::cout << "��Ӧ: " << response.getBody() << std::endl;
        }
    } catch (const fasthttp::HttpException& e) {
        std::cerr << "����ʧ��: " << e.what() << std::endl;
    }
    
    return 0;
}
```

### ʹ��HttpClient��

```cpp
#include "fasthttp.hpp"

int main() {
    fasthttp::HttpClient client;
    
    // ����Ĭ�ϳ�ʱʱ��
    client.setDefaultTimeout(10000); // 10��
    
    // ����Ĭ������ͷ
    client.setDefaultHeader("User-Agent", "MyApp/1.0");
    
    // ��������
    auto response = client.get("https://httpbin.org/get");
    
    return 0;
}
```

## API�ο�

### ȫ�ֱ�ݺ���

```cpp
// GET����
fasthttp::HttpResponse get(const std::string& url);

// POST����
fasthttp::HttpResponse post(const std::string& url, const std::string& data = "");

// PUT����
fasthttp::HttpResponse put(const std::string& url, const std::string& data = "");

// DELETE����
fasthttp::HttpResponse del(const std::string& url);

// POST JSON����
fasthttp::HttpResponse postJson(const std::string& url, const std::string& json);
```

### HttpClient��

```cpp
class HttpClient {
public:
    // ���캯������������
    HttpClient();
    ~HttpClient();
    
    // ���÷���
    void setDefaultTimeout(int timeoutMs);
    void setDefaultHeader(const std::string& key, const std::string& value);
    
    // ����ִ��
    HttpResponse execute(const HttpRequest& request);
    
    // ��ݷ���
    HttpResponse get(const std::string& url);
    HttpResponse post(const std::string& url, const std::string& data = "");
    HttpResponse put(const std::string& url, const std::string& data = "");
    HttpResponse del(const std::string& url);
    HttpResponse patch(const std::string& url, const std::string& data = "");
    
    // JSON����
    HttpResponse postJson(const std::string& url, const std::string& json);
    HttpResponse putJson(const std::string& url, const std::string& json);
};
```

### HttpRequest��

```cpp
class HttpRequest {
public:
    // ���캯��
    HttpRequest(Method method, const std::string& url);
    
    // ��ʽ���÷���
    HttpRequest& setBody(const std::string& body);
    HttpRequest& setTimeout(int timeoutMs);
    HttpRequest& setHeader(const std::string& key, const std::string& value);
    HttpRequest& addHeader(const std::string& key, const std::string& value);
    
    // ������������
    HttpRequest& setContentType(const std::string& contentType);
    HttpRequest& setJsonContent();
    HttpRequest& setFormContent();
    
    // �����֤
    HttpRequest& setBasicAuth(const std::string& username, const std::string& password);
    HttpRequest& setBearerToken(const std::string& token);
};
```

### HttpResponse��

```cpp
class HttpResponse {
public:
    // ״̬��ȡ
    int getStatusCode() const;
    const std::string& getStatusMessage() const;
    const std::string& getBody() const;
    
    // ����ͷ����
    std::string getHeader(const std::string& key) const;
    bool hasHeader(const std::string& key) const;
    const std::map<std::string, std::string>& getHeaders() const;
    
    // ״̬���
    bool isSuccess() const;        // 2xx״̬��
    bool isRedirect() const;       // 3xx״̬��
    bool isClientError() const;    // 4xx״̬��
    bool isServerError() const;    // 5xx״̬��
    
    // ת��Ϊ�ַ���
    std::string toString() const;
};
```

## ʹ��ʾ��

### ����JSON����

```cpp
fasthttp::HttpClient client;

std::string jsonData = R"({
    "name": "����",
    "age": 25,
    "email": "zhangsan@example.com"
})";

auto response = client.postJson("https://api.example.com/users", jsonData);

if (response.isSuccess()) {
    std::cout << "�û������ɹ�: " << response.getBody() << std::endl;
} else {
    std::cout << "����ʧ��: " << response.getStatusCode() << std::endl;
}
```

### ʹ�������֤

```cpp
fasthttp::HttpRequest request(fasthttp::Method::GET, "https://api.example.com/protected");

// Basic��֤
request.setBasicAuth("username", "password");

// ����ʹ��Bearer Token
// request.setBearerToken("your-jwt-token");

fasthttp::HttpClient client;
auto response = client.execute(request);
```

### �Զ�������ͷ

```cpp
fasthttp::HttpRequest request(fasthttp::Method::POST, "https://api.example.com/data");

request.setHeader("Content-Type", "application/xml")
       .setHeader("X-API-Key", "your-api-key")
       .setHeader("Accept", "application/json")
       .setBody("<xml>data</xml>");

fasthttp::HttpClient client;
auto response = client.execute(request);
```

### ������

```cpp
try {
    auto response = fasthttp::get("https://invalid-url.example.com");
    
    if (response.isSuccess()) {
        std::cout << "����ɹ�" << std::endl;
    } else if (response.isClientError()) {
        std::cout << "�ͻ��˴���: " << response.getStatusCode() << std::endl;
    } else if (response.isServerError()) {
        std::cout << "����������: " << response.getStatusCode() << std::endl;
    }
    
} catch (const fasthttp::NetworkException& e) {
    std::cerr << "�������: " << e.what() << std::endl;
} catch (const fasthttp::TimeoutException& e) {
    std::cerr << "����ʱ: " << e.what() << std::endl;
} catch (const fasthttp::HttpException& e) {
    std::cerr << "HTTP����: " << e.what() << std::endl;
}
```

## ����˵��

### Windows (Visual Studio)

```bash
# ֱ�ӱ��룬����Զ����������Windows��
cl /EHsc example.cpp
```

### Linux

```bash
# ��Ҫ����OpenSSL������HTTPS֧��
g++ -std=c++14 -o example example.cpp -lssl -lcrypto
```

### CMakeʾ��

```cmake
cmake_minimum_required(VERSION 3.10)
project(MyProject)

set(CMAKE_CXX_STANDARD 14)

if(WIN32)
    # Windows����Ҫ�����
    add_executable(myapp main.cpp)
else()
    # Linux��ҪOpenSSL
    find_package(OpenSSL REQUIRED)
    add_executable(myapp main.cpp)
    target_link_libraries(myapp OpenSSL::SSL OpenSSL::Crypto)
endif()
```

## ����˵��

- **Windows**: ʹ��WinINet API�������������
- **Linux**: ��ҪOpenSSL������HTTPS֧��
- **C++��׼**: ��ҪC++14����߰汾

## ���֤

MIT License

## ����

��ӭ�ύIssue��Pull Request���Ľ�����⡣