#include "config.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

static std::string trim(const std::string &s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos)
        return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

// Очень простой построчный формат. Одна строка = одна директива.
//   pipeline <name>
//   job <name>
//   image <docker-image>
//   needs <job-name>
//   run <shell command>
//   # comment
Pipeline parse_config(const std::string &path) {
    std::ifstream f(path);
    if (!f)
        throw std::runtime_error("cannot open config: " + path);

    Pipeline p;
    p.name = "pipeline";
    Job *cur = nullptr;
    std::string line;
    int lineno = 0;

    while (std::getline(f, line)) {
        lineno++;

        // комментарии
        auto hash = line.find('#');
        if (hash != std::string::npos)
            line = line.substr(0, hash);
        line = trim(line);
        if (line.empty())
            continue;

        std::istringstream ss(line);
        std::string key;
        ss >> key;
        std::string rest;
        std::getline(ss, rest);
        rest = trim(rest);

        if (key == "pipeline") {
            p.name = rest;
        } else if (key == "job") {
            p.jobs.push_back(Job{});
            cur = &p.jobs.back();
            cur->name = rest;
        } else if (key == "image") {
            if (!cur)
                throw std::runtime_error("image outside job, line " + std::to_string(lineno));
            cur->image = rest;
        } else if (key == "needs") {
            if (!cur)
                throw std::runtime_error("needs outside job, line " + std::to_string(lineno));
            cur->needs.push_back(rest);
        } else if (key == "run") {
            if (!cur)
                throw std::runtime_error("run outside job, line " + std::to_string(lineno));
            cur->steps.push_back(rest);
        } else {
            throw std::runtime_error("unknown directive '" + key + "' at line " +
                                     std::to_string(lineno));
        }
    }
    return p;
}
