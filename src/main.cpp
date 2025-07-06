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
#include <set>

// ------------------- Utility Functions -------------------
void log(const std::string &msg) {
    auto now = std::chrono::system_clock::now();
    auto tt = std::chrono::system_clock::to_time_t(now);
    std::ostringstream ots;
    ots << std::put_time(std::localtime(&tt), "%Y-%m-%d %H:%M:%S") << " " << msg << "\n";
    std::cout << ots.str();
    std::ofstream f("app.log", std::ios::app);
    f << ots.str();
}

std::string sha256(const std::string &in) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char *>(in.data()), in.size(), hash);
    std::ostringstream o;
    o << std::hex << std::setfill('0');
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        o << std::setw(2) << static_cast<int>(hash[i]);
    return o.str();
}

bool ends_with(const std::string &str, const std::string &suffix) {
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool check_credentials(const std::string &user, const std::string &pass) {
    std::ifstream f("data/auth.csv");
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string id_str, uname, hash;
        if (!std::getline(ss, id_str, ',')) continue;
        if (!std::getline(ss, uname, ',')) continue;
        if (!std::getline(ss, hash, ',')) continue; 
        if (uname == user && hash == sha256(pass))
            return true;
    }
    return false;
}

std::string escape_csv(const std::string& s) {
    std::string escaped = s;
    size_t pos = 0;
    while ((pos = escaped.find('"', pos)) != std::string::npos) {
        escaped.replace(pos, 1, "\"");
        pos += 2;
    }
    if (escaped.find(',') != std::string::npos || escaped.find('"') != std::string::npos) {
        return '"' + escaped + '"';
    }
    return escaped;
}

std::vector<std::string> parse_csv_line(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool inQuotes = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char ch = line[i];
        if (ch == '"') {
            inQuotes = !inQuotes;
        } else if (ch == ',' && !inQuotes) {
            fields.push_back(field);
            field.clear();
        } else {
            field += ch;
        }
    }
    fields.push_back(field);
    return fields;
}

struct Customer {
    int id;
    std::string name, father, nationality, dob;
    std::string birthplace, address, mobile, email;
    std::string createdby;
};

std::vector<Customer> load_customers() {
    std::vector<Customer> out;
    std::ifstream f("data/customers.csv");
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        auto fields = parse_csv_line(line);
        if (fields.size() != 10) {
            log("Skipping malformed line: " + line);
            continue;
        }
        Customer c;
        try {
            c.id = std::stoi(fields[0]);
            c.name = fields[1];
            c.father = fields[2];
            c.nationality = fields[3];
            c.dob = fields[4];
            c.birthplace = fields[5];
            c.address = fields[6];
            c.mobile = fields[7];
            c.email = fields[8];
            c.createdby = fields[9];
            out.push_back(c);
        } catch (...) {
            log("Error parsing line: " + line);
        }
    }
    return out;
}

std::string get_cookie(const crow::request& req, const std::string& name) {
    auto cookie_header = req.get_header_value("Cookie");
    size_t pos = cookie_header.find(name + "=");
    if (pos == std::string::npos) return "";
    pos += name.length() + 1;
    size_t end = cookie_header.find(";", pos);
    return cookie_header.substr(pos, end - pos);
}


void save_customers(const std::vector<Customer> &v) {
    std::ofstream f("data/customers.csv");
    for (const auto &c : v) {
        f << c.id << ","
          << escape_csv(c.name) << ","
          << escape_csv(c.father) << ","
          << escape_csv(c.nationality) << ","
          << escape_csv(c.dob) << ","
          << escape_csv(c.birthplace) << ","
          << escape_csv(c.address) << ","
          << escape_csv(c.mobile) << ","
          << escape_csv(c.email) << ","
          << escape_csv(c.createdby) << "\n";
    }
}

int main() {
    log("Server starting");
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([] {
        std::ifstream in("UI/index.html");
        if (!in) return crow::response(404, "index.html not found");
        std::ostringstream ss; ss << in.rdbuf();
        crow::response res{ss.str()};
        res.set_header("Content-Type", "text/html");
        return res;
    });

    CROW_ROUTE(app, "/api/login").methods("POST"_method)([](const crow::request &req) {
        auto js = crow::json::load(req.body);
        if (!js || js["user"].t() != crow::json::type::String || js["pass"].t() != crow::json::type::String)
            return crow::response(400, "Invalid input");

        std::string user = js["user"].s();
        std::string pass = js["pass"].s();
        bool ok = check_credentials(user, pass);
        log("POST /api/login user=" + user + (ok ? " success" : " fail"));
        crow::json::wvalue r;
        r["status"] = ok ? "ok" : "error";
        r["message"] = ok ? "Logged in successfully" : "Invalid username or password";

        crow::response resp(ok ? 200 : 401, r);
        if (ok) {
            resp.add_header("Set-Cookie", "user=" + user + "; Path=/;");
        }
        return resp;
        
    });

    CROW_ROUTE(app, "/api/register").methods("POST"_method)([](const crow::request &req) {
        auto js = crow::json::load(req.body);
        if (!js || js["user"].t() != crow::json::type::String || js["pass"].t() != crow::json::type::String)
            return crow::response(400, "Invalid input");

        std::string user = js["user"].s();
        std::string pass = js["pass"].s();

        // Simple validation: username and password length
        if (user.empty())
            return crow::response(400, "Username can't be empty");
        if(pass.size() < 5)
            return crow::response(400, "Password must be at least 5 characters long");

        std::ifstream f_in("data/auth.csv");
        std::string line;
        int maxID = 0;
        bool exists = false;
        while (std::getline(f_in, line)) {
            std::stringstream ss(line);
            std::string id_str, uname, hash;
            if (!std::getline(ss, id_str, ',')) continue;
            if (!std::getline(ss, uname, ',')) continue;
            if (!std::getline(ss, hash, ',')) continue;

            int id = 0;
            try { id = std::stoi(id_str); } catch (...) {}

            if (id > maxID) maxID = id;
            if (uname == user) exists = true;
        }
        f_in.close();
        if (exists)
            return crow::response(409, "Username already exists");

        int newID = maxID + 1;


        std::ofstream f_out("data/auth.csv", std::ios::app);
        f_out << newID << "," << user << "," << sha256(pass) << "\n";
        log("Registered user=" + user + " id=" + std::to_string(newID));
        return crow::response(201, "{\"status\":\"ok\",\"id\":" + std::to_string(newID) + "}");
    });

    CROW_ROUTE(app, "/api/forgot").methods("POST"_method)([](const crow::request &req) {
        auto js = crow::json::load(req.body);
        if (!js || js["user"].t() != crow::json::type::String || js["pass"].t() != crow::json::type::String)
            return crow::response(400, "Invalid input");

        std::string user = js["user"].s();
        std::string new_pass = js["pass"].s();

        if (new_pass.size() < 5)
            return crow::response(400, "Password must be at least 5 characters long");

        std::ifstream f_in("data/auth.csv");
        std::vector<std::string> lines;
        std::string line;
        bool found = false;

        while (std::getline(f_in, line)) {
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::string id_str, uname, hash;
            if (!std::getline(ss, id_str, ',')) continue;
            if (!std::getline(ss, uname, ',')) continue;
            if (!std::getline(ss, hash, ',')) continue;

            if (uname == user) {
                // Reset password for this user
                std::string new_hash = sha256(new_pass);
                lines.push_back(id_str + "," + uname + "," + new_hash);
                found = true;
            } else {
                // Leave unchanged
                lines.push_back(line);
            }
        }
        f_in.close();

        if (!found) return crow::response(404, "Username not found");

        // Write all users back, including the updated password for this user
        std::ofstream f_out("data/auth.csv");
        for (auto &l : lines) f_out << l << "\n";
        log("Password reset for user=" + user);
        return crow::response(200, "{\"status\":\"ok\"}");
    });

    CROW_ROUTE(app, "/api/customers").methods("GET"_method)([](const crow::request &req) {
        std::string createdby = get_cookie(req, "user");
        if (createdby.empty())
            return crow::response(401, "Not logged in");
        auto customers = load_customers();
        volatile int debug_here = 1;
        crow::json::wvalue r;
        r["customers"] = crow::json::wvalue::list();
        auto &arr = r["customers"];
        int idx = 0;
        for (auto &c : customers) {
            if (c.createdby != createdby) continue; // Filter by createdby
            crow::json::wvalue o;
            o["id"] = c.id;
            o["name"] = c.name;
            o["father"] = c.father;
            o["nationality"] = c.nationality;
            o["dob"] = c.dob;
            o["birthplace"] = c.birthplace;
            o["address"] = c.address;
            o["mobile"] = c.mobile;
            o["email"] = c.email;
            o["createdby"] = createdby;
            arr[idx++] = std::move(o);
        }
        return crow::response{r};
    });

    CROW_ROUTE(app, "/api/customers/<int>").methods("PUT"_method)([](const crow::request &req, int id) {
        std::string createdby = get_cookie(req, "user");
        if (createdby.empty())
            return crow::response(401, "Not logged in");

        auto js = crow::json::load(req.body);
        if (!js) return crow::response(400, "Invalid JSON");
        auto customers = load_customers();
        bool found = false;
        for (auto &c : customers) {
            if (c.id == id) {
                c.name = js["name"].s();
                c.father = js["father"].s();
                c.nationality = js["nationality"].s();
                c.dob = js["dob"].s();
                c.birthplace = js["birthplace"].s();
                c.address = js["address"].s();
                c.mobile = js["mobile"].s();
                c.email = js["email"].s();
                c.createdby = createdby;
                found = true;
                break;
            }
        }
        if (!found) return crow::response(404, "Customer not found");
        save_customers(customers);
        log("Updated customer id=" + std::to_string(id));
        return crow::response(200);
    });

    CROW_ROUTE(app, "/api/customers").methods("POST"_method)([](const crow::request &req) {
        std::string createdby = get_cookie(req, "user");
        if (createdby.empty())
            return crow::response(401, "Not logged in");
        auto js = crow::json::load(req.body);
        if (!js) return crow::response(400, "Invalid JSON");
        auto v = load_customers();
        int nid = v.empty() ? 1 : v.back().id + 1;
        Customer c;
        c.id = nid;
        c.name = js["name"].s();
        c.father = js["father"].s();
        c.nationality = js["nationality"].s();
        c.dob = js["dob"].s();
        c.birthplace = js["birthplace"].s();
        c.address = js["address"].s();
        c.mobile = js["mobile"].s();
        c.email = js["email"].s();
        c.createdby = createdby;
        v.push_back(std::move(c));
        save_customers(v);
        log("Added customer id=" + std::to_string(nid));
        return crow::response(201);
    });

    CROW_ROUTE(app, "/api/customers/<int>").methods("DELETE"_method)([](int id) {
        auto customers = load_customers();
        auto it = std::find_if(customers.begin(), customers.end(), [id](const Customer &c) { return c.id == id; });
        if (it == customers.end()) return crow::response(404, "Customer not found");
        log("Deleted customer id=" + std::to_string(it->id));
        customers.erase(it);
        save_customers(customers);
        return crow::response(200);
    });

    CROW_ROUTE(app, "/api/export").methods("GET"_method)([](const crow::request &req) {
        auto customers = load_customers();
        std::vector<Customer> exportList;
        if (req.url_params.get("ids")) {
            std::set<int> idSet;
            std::string idsParam = req.url_params.get("ids");
            std::istringstream ss(idsParam);
            std::string idStr;
            while (std::getline(ss, idStr, ',')) idSet.insert(std::stoi(idStr));
            for (const auto &c : customers)
                if (idSet.count(c.id)) exportList.push_back(c);
            log("Exporting selected records: " + idsParam);
        } else {
            exportList = customers;
            log("Exporting full customer list");
        }
        std::ostringstream out;
        for (const auto &c : exportList) {
            out << c.id << "," << escape_csv(c.name) << "," << escape_csv(c.father) << "," << escape_csv(c.nationality) << ","
                << escape_csv(c.dob) << "," << escape_csv(c.birthplace) << "," << escape_csv(c.address) << ","
                << "=" << escape_csv(c.mobile) << "," << escape_csv(c.email) << "," << escape_csv(c.createdby) << "\n";
        }
        crow::response res{out.str()};
        res.set_header("Content-Type", "text/csv");
        res.set_header("Content-Disposition", "attachment; filename=\"customers.csv\"");
        return res;
    });

    CROW_ROUTE(app, "/api/import").methods("POST"_method)([](const crow::request &req) {
        std::istringstream csvData(req.body);
        std::string line;
        std::vector<Customer> records = load_customers();
        int maxId = records.empty() ? 0 : records.back().id;
        while (std::getline(csvData, line)) {
            auto fields = parse_csv_line(line);
            if (fields.size() != 10) continue;
            Customer c;
            try {
                c.id = ++maxId;
                c.name = fields[1];
                c.father = fields[2];
                c.nationality = fields[3];
                c.dob = fields[4];
                c.birthplace = fields[5];
                c.address = fields[6];
                c.mobile = fields[7];
                c.email = fields[8];
                c.createdby = fields[9];
                records.push_back(c);
            } catch (...) {}
        }
        save_customers(records);
        return crow::response(200);
    });

    CROW_ROUTE(app, "/<path>")([](const crow::request &, std::string path) {
        std::ifstream in("UI/" + path);
        if (in) {
            std::ostringstream ss; ss << in.rdbuf();
            crow::response res{ss.str()};
            if (ends_with(path, ".html")) res.set_header("Content-Type", "text/html");
            else if (ends_with(path, ".js")) res.set_header("Content-Type", "application/javascript");
            else if (ends_with(path, ".css")) res.set_header("Content-Type", "text/css");
            return res;
        }
        std::ifstream fallback("UI/index.html");
        if (!fallback) return crow::response(404, "index.html not found");
        std::ostringstream ss; ss << fallback.rdbuf();
        crow::response res{ss.str()};
        res.set_header("Content-Type", "text/html");
        return res;
    });

    CROW_ROUTE(app, "/health")([] { return "Server running"; });

    std::cout << "Server listening on http://127.0.0.1:8080\n";
    app.bindaddr("0.0.0.0").port(8080).concurrency(4).run();
    log("Server stopping");
    return 0;
}
