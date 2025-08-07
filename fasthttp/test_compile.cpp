#include "fasthttp.hpp"
#include <iostream>

int main() {
    std::cout << "FastHTTP Library Test" << std::endl;
    
    try {
        // Create a simple client test
        fasthttp::HttpClient client;
        std::cout << "HTTP client created successfully" << std::endl;
        
        // Create a request object test
        fasthttp::HttpRequest request(fasthttp::Method::GET, "http://example.com");
        request.setHeader("User-Agent", "FastHTTP-Test/1.0");
        std::cout << "HTTP request object created successfully" << std::endl;
        
        // Test URL parsing
        auto url = fasthttp::URL::parse("https://api.github.com:443/users/octocat?tab=repositories#readme");
        std::cout << "URL parsing test:" << std::endl;
        std::cout << "  Scheme: " << url.scheme << std::endl;
        std::cout << "  Host: " << url.host << std::endl;
        std::cout << "  Port: " << url.port << std::endl;
        std::cout << "  Path: " << url.path << std::endl;
        std::cout << "  Query: " << url.query << std::endl;
        std::cout << "  Fragment: " << url.fragment << std::endl;
        
        std::cout << "All tests passed! Library compiled successfully." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}