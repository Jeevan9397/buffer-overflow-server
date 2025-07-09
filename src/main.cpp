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
#include <cstring>

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
        auto p = line.find(',');
        if (p == std::string::npos) continue;
        if (line.substr(0, p) != user) continue;
        auto q = line.find(',', p + 1);
        std::string stored_hash = (q == std::string::npos)
                                      ? line.substr(p + 1)
                                      : line.substr(p + 1, q - p - 1);
        if (stored_hash == sha256(pass)) return true;
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
};

std::vector<Customer> load_customers() {
    std::vector<Customer> out;
    std::ifstream f("data/customers.csv");
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        auto fields = parse_csv_line(line);
        if (fields.size() != 9) {
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
            out.push_back(c);
        } catch (...) {
            log("Error parsing line: " + line);
        }
    }
    return out;
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
          << escape_csv(c.email) << "\n";
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
        std::string user = js["user"].s();
        bool ok = check_credentials(user, js["pass"].s());
        log("POST /api/login user=" + user + (ok ? " success" : " fail"));
        crow::json::wvalue r;
        r["status"] = ok ? "ok" : "error";
        return crow::response(ok ? 200 : 401, r);
    });

    CROW_ROUTE(app, "/api/customers").methods("GET"_method)([] {
        auto customers = load_customers();
        crow::json::wvalue r;
        r["customers"] = crow::json::wvalue::list();
        auto &arr = r["customers"];
        int idx = 0;
        for (auto &c : customers) {
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
            arr[idx++] = std::move(o);
        }
        return crow::response{r};
    });

    // CROW_ROUTE(app, "/api/customers/<int>").methods("PUT"_method)([](const crow::request &req, int id) {
    //     auto js = crow::json::load(req.body);
    //     if (!js) return crow::response(400, "Invalid JSON");
    //     auto customers = load_customers();
    //     bool found = false;
    //     for (auto &c : customers) {
    //         if (c.id == id) {
    //             c.name = js["name"].s();
    //             c.father = js["father"].s();
    //             c.nationality = js["nationality"].s();
    //             c.dob = js["dob"].s();
    //             c.birthplace = js["birthplace"].s();
    //             c.address = js["address"].s();
    //             c.mobile = js["mobile"].s();
    //             c.email = js["email"].s();
    //             found = true;
    //             break;
    //         }
    //     }
    //     if (!found) return crow::response(404, "Customer not found");
    //     save_customers(customers);
    //     log("Updated customer id=" + std::to_string(id));
    //     return crow::response(200);
    // });
CROW_ROUTE(app, "/api/customers/<int>").methods("PUT"_method)(
[](const crow::request &req, int id) {
    auto js = crow::json::load(req.body);
    if (!js) return crow::response(400, "Invalid JSON");

    auto customers = load_customers();
    bool found = false;

    for (auto &c : customers) {
        if (c.id == id) {
            std::string newAddr = js["address"].s();
            log("Editing customer id=" + std::to_string(id) + " address length=" + std::to_string(newAddr.size()));

            // Combine buffer, canary, and allow overflow space
            char combined[64] = {};   // total space: 40 (buffer) + 1 (canary) + rest (overflow)
            char* addrBuffer = combined;
            char* canaryPtr = &combined[40];
            *canaryPtr = '#';  // Set canary

            // OFF-BY-ONE copy vulnerability: 41 bytes into 40 buffer + 1 canary
            strncpy(addrBuffer, newAddr.c_str(), 41);
            addrBuffer[39] = '\0';  // Null-terminate properly

            // Prepare and log the view: buffer + [CANARY:X] + overflow
            std::string loggedBuffer(combined, 64);
            std::string combinedView = loggedBuffer.substr(0, 40);  // buffer
            combinedView += "[CANARY:" + std::string(1, combined[40]) + "]"; // canary value
            combinedView += loggedBuffer.substr(41);  // overflow portion

            log("Off-by-one address buffer + canary + overflow: " + combinedView);

            // Assign safely to object
            c.name = js["name"].s();
            c.father = js["father"].s();
            c.nationality = js["nationality"].s();
            c.dob = js["dob"].s();
            c.birthplace = js["birthplace"].s();
            c.address = addrBuffer;  // Overflown content may be unsafe!
            c.mobile = js["mobile"].s();
            c.email = js["email"].s();

            found = true;
            break;
        }
    }

    if (!found) return crow::response(404, "Customer not found");

    save_customers(customers);
    log("Updated customer with address off-by-one vulnerability");
    return crow::response(200);
});



    CROW_ROUTE(app, "/api/customers").methods("POST"_method)([](const crow::request &req) {
        auto js = crow::json::load(req.body);
        if (!js) return crow::response(400, "Invalid JSON");
        auto v = load_customers();
        int nid = v.empty() ? 1 : v.back().id + 1;
        Customer c;
        c.id = nid;
        // c.name = js["name"].s();
        //Injecting bufferOverFlow ---------------- ATTACK1
        char nameBuf[32]; //fixed buffer size
        std::string inputName = js["name"].s();
        strcpy(nameBuf , inputName.c_str());
        log("Receieved POST /api/customers with name:" + std::string(nameBuf));
        c.name = std::string(nameBuf);

        // c.father = js["father"].s();
        char FatherBuf[40];
        std::string father = js["father"].s();
        snprintf(FatherBuf, sizeof(FatherBuf), father.c_str()); //Vulnerable if name has format specifiers
        log(std::string("Formatted:")+ FatherBuf);
        c.father = std::string(FatherBuf);
        c.nationality = js["nationality"].s();
        c.dob = js["dob"].s();
        c.birthplace = js["birthplace"].s();
        c.address = js["address"].s();
        c.mobile = js["mobile"].s();
        c.email = js["email"].s();
        v.push_back(std::move(c));
        save_customers(v);
        log("Added customer id=" + std::to_string(nid));
        return crow::response(201);
    });


 // ATTACK 1 Using STRCPY FUNCTION

// CROW_ROUTE(app, "/api/customers").methods("POST"_method)([](const crow::request &req) {
//     // 1) Parse JSON
//     auto js = crow::json::load(req.body);
//     if (!js) return crow::response(400, "Invalid JSON");

//     // 2) Compute new ID
//     auto v = load_customers();
//     int nid = v.empty() ? 1 : v.back().id + 1;

//     // ───── Vulnerable buffer overflow demo ─────
//     // 3) Copy the JSON “name” into a std::string (so we own the data)
//     std::string rawName = js["name"].s();      // r_string → std::string via operator std::string()

//     // 4) Tiny fixed‐size stack buffer
//     char buf[32];

//     // 5) UNSAFE copy: if rawName.length() ≥ 32, this overruns buf
//     std::strcpy(buf, rawName.c_str());         // ← NO bounds check!

//     // 6) Build your Customer struct using the overflowed buffer
//     Customer c;
//     c.id          = nid;
//     c.name        = buf;                      // overflow lands here
//     c.father      = js["father"].s();
//     c.nationality = js["nationality"].s();
//     c.dob         = js["dob"].s();
//     c.birthplace  = js["birthplace"].s();
//     c.address     = js["address"].s();
//     c.mobile      = js["mobile"].s();
//     c.email       = js["email"].s();
//     // ───── end vulnerable section ─────

//     // 7) Persist and log
//     v.push_back(std::move(c));
//     save_customers(v);
//     log("Added customer (vulnerable) id=" + std::to_string(nid)
//         + " name=" + std::string(buf));

//     return crow::response(201);
// });



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
                << "=" << escape_csv(c.mobile) << "," << escape_csv(c.email) << "\n";
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
            if (fields.size() != 9) continue;
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
