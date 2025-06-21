#include "crow.h"
#include <openssl/sha.h>
#include <fstream>
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <vector>
#include <algorithm>
#include <utility>            // for std::move

// Simple timestamped logger
void log(const std::string &msg) {
    auto now  = std::chrono::system_clock::now();
    auto tt   = std::chrono::system_clock::to_time_t(now);
    std::ostringstream ots;
    ots << std::put_time(std::localtime(&tt), "%Y-%m-%d %H:%M:%S")
        << " " << msg << "\n";
    std::cout << ots.str();
    std::ofstream f("app.log", std::ios::app);
    f << ots.str();
}

// SHA-256 helper (hex output)
std::string sha256(const std::string &in) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(in.data()),
           in.size(), hash);
    std::ostringstream o;
    o << std::hex << std::setfill('0');
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        o << std::setw(2) << static_cast<int>(hash[i]);
    return o.str();
}

// Validate credentials against data/auth.csv (username,hash,role)
bool check_credentials(const std::string &user, const std::string &pass) {
    std::ifstream f("data/auth.csv");
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        // Find first comma (after username)
        auto p = line.find(',');
        if (p == std::string::npos) continue;
        // Username must match
        if (line.substr(0, p) != user) continue;

        // Find second comma (end of hash)
        auto q = line.find(',', p + 1);
        // Extract stored hash (between commas, or to end if no role)
        std::string stored_hash = (q == std::string::npos)
            ? line.substr(p + 1)
            : line.substr(p + 1, q - p - 1);

        // Compare against SHA256 of supplied password
        if (stored_hash == sha256(pass))
            return true;
    }
    return false;
}

// Customer model
struct Customer {
    int    id;
    std::string name, father, nationality, dob;
    std::string birth_village, birth_district, birth_province;
    std::string street, city, home_district, home_province;
    std::string mobile, email;
};

// Load all customers from CSV
std::vector<Customer> load_customers() {
    std::vector<Customer> out;
    std::ifstream f("data/customers.csv");
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        std::istringstream ss(line);
        Customer c;
        std::string fld;
        std::getline(ss, fld, ','); c.id = std::stoi(fld);
        std::getline(ss, c.name,   ',');
        std::getline(ss, c.father, ',');
        std::getline(ss, c.nationality, ',');
        std::getline(ss, c.dob,    ',');
        std::getline(ss, c.birth_village,    ',');
        std::getline(ss, c.birth_district,   ',');
        std::getline(ss, c.birth_province,   ',');
        std::getline(ss, c.street, ',');
        std::getline(ss, c.city,   ',');
        std::getline(ss, c.home_district, ',');
        std::getline(ss, c.home_province, ',');
        std::getline(ss, c.mobile, ',');
        std::getline(ss, c.email,  '\n');
        out.push_back(std::move(c));
    }
    return out;
}

// Save all customers back to CSV
void save_customers(const std::vector<Customer>& v) {
    std::ofstream f("data/customers.csv");
    for (auto &c: v) {
        f << c.id << ","
          << c.name << "," << c.father << "," << c.nationality << "," 
          << c.dob << "," << c.birth_village << "," << c.birth_district << ","
          << c.birth_province << "," << c.street << "," << c.city << ","
          << c.home_district << "," << c.home_province << ","
          << c.mobile << "," << c.email << "\n";
    }
}

int main() {
    log("Server starting");
    crow::SimpleApp app;

    // Serve login page
    CROW_ROUTE(app, "/")([](){
        std::ifstream in("UI/login.html");
        std::ostringstream ss; ss << in.rdbuf();
        auto res = crow::response(ss.str());
        res.set_header("Content-Type","text/html");
        return res;
    });

    // Login API
    CROW_ROUTE(app, "/api/login").methods("POST"_method)
    ([](const crow::request &req){
        auto js = crow::json::load(req.body);
        bool ok = check_credentials(js["user"].s(), js["pass"].s());
        std::string user = js["user"].s();
        log("POST /api/login user=" + user + (ok ? " success" : " fail"));
        crow::json::wvalue r;
        r["status"] = ok ? "ok" : "error";
        return crow::response(ok ? 200 : 401, r);
    });

    // GET all customers
    CROW_ROUTE(app, "/api/customers").methods("GET"_method)
    ([](){
        log("GET /api/customers");
        auto customers = load_customers();
        crow::json::wvalue r;
        r["customers"] = crow::json::wvalue::list();
        auto &arr = r["customers"];
        int idx = 0;
        for (auto &c: customers) {
            crow::json::wvalue o;
            o["id"]             = c.id;
            o["name"]           = c.name;
            o["father"]         = c.father;
            o["nationality"]    = c.nationality;
            o["dob"]            = c.dob;
            o["birth_village"]  = c.birth_village;
            o["birth_district"] = c.birth_district;
            o["birth_province"] = c.birth_province;
            o["street"]         = c.street;
            o["city"]           = c.city;
            o["home_district"]  = c.home_district;
            o["home_province"]  = c.home_province;
            o["mobile"]         = c.mobile;
            o["email"]          = c.email;
            // Move-assign into the JSON array
            arr[idx++] = std::move(o);
        }
        return crow::response{r};
    });

    // POST new customer
    CROW_ROUTE(app, "/api/customers").methods("POST"_method)
    ([](const crow::request &req){
        auto js = crow::json::load(req.body);
        auto v = load_customers();
        int nid = v.empty() ? 1 : v.back().id + 1;
        Customer c; c.id = nid;
        c.name           = js["name"].s();
        c.father         = js["father"].s();
        c.nationality    = js["nationality"].s();
        c.dob            = js["dob"].s();
        c.birth_village  = js["birth_village"].s();
        c.birth_district = js["birth_district"].s();
        c.birth_province = js["birth_province"].s();
        c.street         = js["street"].s();
        c.city           = js["city"].s();
        c.home_district  = js["home_district"].s();
        c.home_province  = js["home_province"].s();
        c.mobile         = js["mobile"].s();
        c.email          = js["email"].s();
        v.push_back(std::move(c));
        save_customers(v);
        log("Added customer id=" + std::to_string(nid));
        return crow::response{201};
    });

    // PUT update customer
    CROW_ROUTE(app, "/api/customers/<int>").methods("PUT"_method)
    ([](const crow::request &req, crow::response &res, int id){
        auto js = crow::json::load(req.body);
        auto v  = load_customers();
        bool found = false;
        for (auto &c: v) {
            if (c.id == id) {
                c.name           = js["name"].s();
                c.father         = js["father"].s();
                c.nationality    = js["nationality"].s();
                c.dob            = js["dob"].s();
                c.birth_village  = js["birth_village"].s();
                c.birth_district = js["birth_district"].s();
                c.birth_province = js["birth_province"].s();
                c.street         = js["street"].s();
                c.city           = js["city"].s();
                c.home_district  = js["home_district"].s();
                c.home_province  = js["home_province"].s();
                c.mobile         = js["mobile"].s();
                c.email          = js["email"].s();
                found = true;
                break;
            }
        }
        if (!found) {
            res = crow::response(404);
            res.end();
            return;
        }
        save_customers(v);
        log("Updated customer id=" + std::to_string(id));
        res = crow::response(200);
        res.end();
    });

    // DELETE customer
    CROW_ROUTE(app, "/api/customers/<int>").methods("DELETE"_method)
    ([](const crow::request &req, crow::response &res, int id){
        auto v = load_customers();
        auto before = v.size();
        v.erase(std::remove_if(v.begin(), v.end(),
                   [&](const Customer &c){ return c.id == id; }),
                v.end());
        if (v.size() == before) {
            res = crow::response(404);
            res.end();
            return;
        }
        save_customers(v);
        log("Deleted customer id=" + std::to_string(id));
        res = crow::response(200);
        res.end();
    });

    // Export CSV
    CROW_ROUTE(app, "/api/export").methods("GET"_method)
    ([](){
        log("GET /api/export");
        std::ifstream f("data/customers.csv");
        std::ostringstream ss; ss << f.rdbuf();
        auto res = crow::response{ss.str()};
        res.set_header("Content-Type","text/csv");
        res.set_header("Content-Disposition","attachment; filename=\"customers.csv\"");
        return res;
    });

    // Health & test endpoints
    CROW_ROUTE(app, "/health")([](){ return "Server running"; });
    CROW_ROUTE(app, "/api/hello").methods("GET"_method)([](){
        log("GET /api/hello");
        crow::json::wvalue r; r["message"] = "Hello!";
        return r;
    });

    std::cout << "Server listening on port 8080\n";
    app.bindaddr("0.0.0.0").port(8080).concurrency(4).run();
    log("Server stopping");
    return 0;
}