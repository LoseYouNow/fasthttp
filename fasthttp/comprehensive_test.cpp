#include "fasthttp.hpp"
#include <iostream>
#include <string>

// 测试所有HTTP方法
void testHttpMethods() {
    std::cout << "\n=== Testing HTTP Methods ===" << std::endl;
    
    fasthttp::HttpClient client;
    client.setDefaultTimeout(10000); // 10 seconds
    
    try {
        // Test GET
        std::cout << "Testing GET..." << std::endl;
        auto getResponse = client.get("http://httpbin.org/get");
        std::cout << "GET Status: " << getResponse.getStatusCode() << std::endl;
        
        // Test POST with data
        std::cout << "Testing POST..." << std::endl;
        auto postResponse = client.post("http://httpbin.org/post", "test data");
        std::cout << "POST Status: " << postResponse.getStatusCode() << std::endl;
        
        // Test PUT
        std::cout << "Testing PUT..." << std::endl;
        auto putResponse = client.put("http://httpbin.org/put", "update data");
        std::cout << "PUT Status: " << putResponse.getStatusCode() << std::endl;
        
        // Test DELETE
        std::cout << "Testing DELETE..." << std::endl;
        auto delResponse = client.del("http://httpbin.org/delete");
        std::cout << "DELETE Status: " << delResponse.getStatusCode() << std::endl;
        
    } catch (const fasthttp::HttpException& e) {
        std::cout << "HTTP Error: " << e.what() << std::endl;
    }
}

// 测试JSON功能
void testJsonMethods() {
    std::cout << "\n=== Testing JSON Methods ===" << std::endl;
    
    fasthttp::HttpClient client;
    
    try {
        std::string jsonData = R"({"name": "FastHTTP", "version": "1.0", "type": "HTTP Client"})";
        
        std::cout << "Testing POST JSON..." << std::endl;
        auto response = client.postJson("http://httpbin.org/post", jsonData);
        std::cout << "POST JSON Status: " << response.getStatusCode() << std::endl;
        
        if (response.hasHeader("Content-Type")) {
            std::cout << "Response Content-Type: " << response.getHeader("Content-Type") << std::endl;
        }
        
    } catch (const fasthttp::HttpException& e) {
        std::cout << "JSON Error: " << e.what() << std::endl;
    }
}

// 测试自定义请求头
void testCustomHeaders() {
    std::cout << "\n=== Testing Custom Headers ===" << std::endl;
    
    try {
        fasthttp::HttpRequest request(fasthttp::Method::GET, "http://httpbin.org/headers");
        request.setHeader("X-Custom-Header", "FastHTTP-Test")
               .setHeader("X-Version", "1.0")
               .setBearerToken("test-token-123");
        
        fasthttp::HttpClient client;
        auto response = client.execute(request);
        std::cout << "Custom Headers Status: " << response.getStatusCode() << std::endl;
        
    } catch (const fasthttp::HttpException& e) {
        std::cout << "Custom Headers Error: " << e.what() << std::endl;
    }
}

// 测试Basic认证
void testBasicAuth() {
    std::cout << "\n=== Testing Basic Authentication ===" << std::endl;
    
    try {
        fasthttp::HttpRequest request(fasthttp::Method::GET, "http://httpbin.org/basic-auth/user/pass");
        request.setBasicAuth("user", "pass");
        
        fasthttp::HttpClient client;
        auto response = client.execute(request);
        std::cout << "Basic Auth Status: " << response.getStatusCode() << std::endl;
        
        if (response.isSuccess()) {
            std::cout << "Basic Authentication successful!" << std::endl;
        }
        
    } catch (const fasthttp::HttpException& e) {
        std::cout << "Basic Auth Error: " << e.what() << std::endl;
    }
}

// 测试状态检查
void testStatusChecks() {
    std::cout << "\n=== Testing Status Checks ===" << std::endl;
    
    fasthttp::HttpClient client;
    
    try {
        // Test different status codes
        auto response200 = client.get("http://httpbin.org/status/200");
        std::cout << "200 isSuccess: " << (response200.isSuccess() ? "true" : "false") << std::endl;
        
        auto response404 = client.get("http://httpbin.org/status/404");
        std::cout << "404 isClientError: " << (response404.isClientError() ? "true" : "false") << std::endl;
        
        auto response500 = client.get("http://httpbin.org/status/500");
        std::cout << "500 isServerError: " << (response500.isServerError() ? "true" : "false") << std::endl;
        
    } catch (const fasthttp::HttpException& e) {
        std::cout << "Status Check Error: " << e.what() << std::endl;
    }
}

// 测试全局便捷函数
void testGlobalFunctions() {
    std::cout << "\n=== Testing Global Functions ===" << std::endl;
    
    try {
        // Test global GET function
        auto response = fasthttp::get("http://httpbin.org/get");
        std::cout << "Global GET Status: " << response.getStatusCode() << std::endl;
        
        // Test global POST function
        auto postResponse = fasthttp::post("http://httpbin.org/post", "global post data");
        std::cout << "Global POST Status: " << postResponse.getStatusCode() << std::endl;
        
        // Test global POST JSON function
        auto jsonResponse = fasthttp::postJson("http://httpbin.org/post", R"({"global": true})");
        std::cout << "Global POST JSON Status: " << jsonResponse.getStatusCode() << std::endl;
        
    } catch (const fasthttp::HttpException& e) {
        std::cout << "Global Functions Error: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "FastHTTP Comprehensive Test Suite" << std::endl;
    std::cout << "=================================" << std::endl;
    
    // Run basic compilation test first
    try {
        fasthttp::HttpClient client;
        std::cout << "? Basic compilation test passed" << std::endl;
        
        // Test URL parsing
        auto url = fasthttp::URL::parse("https://api.example.com:8080/v1/users?page=1&limit=10#section");
        std::cout << "? URL parsing test passed" << std::endl;
        std::cout << "  Parsed URL: " << url.scheme << "://" << url.host << ":" << url.port 
                  << url.path << "?" << url.query << "#" << url.fragment << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "? Basic test failed: " << e.what() << std::endl;
        return 1;
    }
    
    // Run network tests (these require internet connection)
    std::cout << "\nRunning network tests (requires internet connection)..." << std::endl;
    
    testHttpMethods();
    testJsonMethods();
    testCustomHeaders();
    testBasicAuth();
    testStatusChecks();
    testGlobalFunctions();
    
    std::cout << "\n=== Test Suite Completed ===" << std::endl;
    std::cout << "Note: Some network tests may fail if httpbin.org is not accessible." << std::endl;
    
    return 0;
}