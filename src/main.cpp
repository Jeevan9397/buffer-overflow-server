#include "crow.h"
#include <fstream>
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

// Write timestamped message to both console and log file
void log(const std::string &msg) {
    // Get current time
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S")
        << " " << msg << "\n";
    // Append to log file
    std::ofstream f("app.log", std::ios::app);
    f << oss.str();
    // Also print to console
    std::cout << oss.str();
}

int main() {
    log("Server starting");
    crow::SimpleApp app;

    // Serve the login page
CROW_ROUTE(app, "/")([](){
    std::ifstream in("UI/login.html");
    std::ostringstream ss;
    ss << in.rdbuf();
    auto res = crow::response(ss.str());
    res.set_header("Content-Type", "text/html");
    return res;
});


    // Health-check route
    CROW_ROUTE(app, "/health")([](){
        return "Server running";
    });

    // JSON test endpoint
    CROW_ROUTE(app, "/api/hello").methods("GET"_method)([](){
        log("Received GET /api/hello");
        crow::json::wvalue res;
        res["message"] = "Hello from C++ server!";
        return res;
    });

    std::cout << "Server has been successfully started on port 8080\n";
    app.bindaddr("0.0.0.0").port(8080).concurrency(4).run();

    log("Server stopping");
    return 0;
}
