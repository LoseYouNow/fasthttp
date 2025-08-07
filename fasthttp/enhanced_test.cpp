#include "fasthttp.hpp"
#include <iostream>
#include <iomanip>

void testEnhancedFeatures() {
    std::cout << "\n=== Testing Enhanced Features ===" << std::endl;
    
    fasthttp::HttpClient client;
    
    try {
        // Test RequestBuilder pattern
        std::cout << "Testing RequestBuilder pattern..." << std::endl;
        auto request = client.GET("http://httpbin.org/get")
                           .addHeader("X-Custom", "Builder-Pattern")
                           .addQueryParam("test", "builder")
                           .setTimeout(5000)
                           .build();
        
        auto response = client.execute(request);
        std::cout << "Builder pattern status: " << response.getStatusCode() << std::endl;
        
        // Test global builder functions
        std::cout << "Testing global builder functions..." << std::endl;
        auto globalRequest = fasthttp::POST("http://httpbin.org/post")
                                .setJsonBody(R"({"message": "global builder"})")
                                .addHeader("X-Test", "Global")
                                .build();
        
        auto globalResponse = client.execute(globalRequest);
        std::cout << "Global builder status: " << globalResponse.getStatusCode() << std::endl;
        
    } catch (const fasthttp::HttpException& e) {
        std::cout << "Enhanced features error: " << e.what() << std::endl;
    }
}

void testFormDataFeatures() {
    std::cout << "\n=== Testing Form Data Features ===" << std::endl;
    
    fasthttp::HttpClient client;
    
    try {
        // Test URL-encoded form
        std::cout << "Testing URL-encoded form..." << std::endl;
        std::map<std::string, std::string> formData = {
            {"username", "testuser"},
            {"password", "testpass"},
            {"email", "test@example.com"}
        };
        
        auto formResponse = client.postForm("http://httpbin.org/post", formData);
        std::cout << "Form post status: " << formResponse.getStatusCode() << std::endl;
        
        // Test multipart form
        std::cout << "Testing multipart form..." << std::endl;
        fasthttp::FormData multipartData;
        multipartData.addField("name", "FastHTTP")
                     .addField("version", "1.0")
                     .addField("description", "Enhanced HTTP client");
        
        auto multipartResponse = client.postMultipart("http://httpbin.org/post", multipartData);
        std::cout << "Multipart post status: " << multipartResponse.getStatusCode() << std::endl;
        
    } catch (const fasthttp::HttpException& e) {
        std::cout << "Form data error: " << e.what() << std::endl;
    }
}

void testUrlEncoding() {
    std::cout << "\n=== Testing URL Encoding ===" << std::endl;
    
    std::string testString = "Hello World! @#$%^&*()";
    std::string encoded = fasthttp::urlEncode(testString);
    std::string decoded = fasthttp::urlDecode(encoded);
    
    std::cout << "Original: " << testString << std::endl;
    std::cout << "Encoded:  " << encoded << std::endl;
    std::cout << "Decoded:  " << decoded << std::endl;
    
    // Test query string building
    std::map<std::string, std::string> params = {
        {"query", "test search"},
        {"page", "1"},
        {"limit", "10"},
        {"sort", "name desc"}
    };
    
    std::string queryString = fasthttp::buildQueryString(params);
    std::cout << "Query string: " << queryString << std::endl;
}

void testResponseEnhancements() {
    std::cout << "\n=== Testing Response Enhancements ===" << std::endl;
    
    fasthttp::HttpClient client;
    
    try {
        // Test response utilities
        auto response = client.get("http://httpbin.org/json");
        
        std::cout << "Response summary:" << std::endl;
        std::cout << response.getSummary() << std::endl;
        
        std::cout << "Content type: " << response.getContentType() << std::endl;
        std::cout << "Content length: " << response.getContentLength() << std::endl;
        std::cout << "Is JSON: " << (response.isJson() ? "true" : "false") << std::endl;
        std::cout << "Is HTML: " << (response.isHtml() ? "true" : "false") << std::endl;
        std::cout << "Status category: " << response.getStatusCategory() << std::endl;
        
    } catch (const fasthttp::HttpException& e) {
        std::cout << "Response enhancement error: " << e.what() << std::endl;
    }
}

void testEnhancedHttpMethods() {
    std::cout << "\n=== Testing Enhanced HTTP Methods ===" << std::endl;
    
    fasthttp::HttpClient client;
    
    try {
        // Test HEAD method
        std::cout << "Testing HEAD method..." << std::endl;
        auto headResponse = client.head("http://httpbin.org/get");
        std::cout << "HEAD status: " << headResponse.getStatusCode() << std::endl;
        std::cout << "HEAD body length: " << headResponse.getBody().length() << " (should be 0)" << std::endl;
        
        // Test OPTIONS method
        std::cout << "Testing OPTIONS method..." << std::endl;
        auto optionsResponse = client.options("http://httpbin.org/get");
        std::cout << "OPTIONS status: " << optionsResponse.getStatusCode() << std::endl;
        
        // Test enhanced methods with headers
        std::cout << "Testing methods with custom headers..." << std::endl;
        std::map<std::string, std::string> customHeaders = {
            {"X-API-Key", "test-key"},
            {"X-Request-ID", "12345"}
        };
        
        auto getWithHeaders = client.get("http://httpbin.org/headers", customHeaders);
        std::cout << "GET with headers status: " << getWithHeaders.getStatusCode() << std::endl;
        
    } catch (const fasthttp::HttpException& e) {
        std::cout << "Enhanced HTTP methods error: " << e.what() << std::endl;
    }
}

void testGlobalEnhancedFunctions() {
    std::cout << "\n=== Testing Global Enhanced Functions ===" << std::endl;
    
    try {
        // Test global functions with headers
        std::map<std::string, std::string> headers = {
            {"User-Agent", "FastHTTP-Enhanced/1.0"},
            {"Accept", "application/json"}
        };
        
        std::cout << "Testing global GET with headers..." << std::endl;
        auto response = fasthttp::get("http://httpbin.org/user-agent", headers);
        std::cout << "Global GET status: " << response.getStatusCode() << std::endl;
        
        // Test global JSON functions
        std::cout << "Testing global JSON functions..." << std::endl;
        auto jsonResponse = fasthttp::postJson("http://httpbin.org/post", 
                                               R"({"enhanced": true, "version": "1.0"})", 
                                               headers);
        std::cout << "Global JSON POST status: " << jsonResponse.getStatusCode() << std::endl;
        
        // Test global form functions
        std::cout << "Testing global form functions..." << std::endl;
        std::map<std::string, std::string> formData = {
            {"field1", "value1"},
            {"field2", "value2"}
        };
        auto formResponse = fasthttp::postForm("http://httpbin.org/post", formData, headers);
        std::cout << "Global form POST status: " << formResponse.getStatusCode() << std::endl;
        
    } catch (const fasthttp::HttpException& e) {
        std::cout << "Global enhanced functions error: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "FastHTTP Enhanced Features Test Suite" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    testUrlEncoding();
    testEnhancedFeatures();
    testFormDataFeatures();
    testResponseEnhancements();
    testEnhancedHttpMethods();
    testGlobalEnhancedFunctions();
    
    std::cout << "\n=== Enhanced Test Suite Completed ===" << std::endl;
    std::cout << "All enhanced features have been tested!" << std::endl;
    
    return 0;
}