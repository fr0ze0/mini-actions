#include "runner.hpp"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <set>
#include <string>
#include <vector>

static std::vector<const Job *> order_jobs(const Pipeline &p) {
    std::vector<const Job *> result;
    std::set<std::string> done;

    while (result.size() < p.jobs.size()) {
        bool progress = false;
        for (const auto &j : p.jobs) {
            if (done.count(j.name))
                continue;
            bool ready = true;
            for (const auto &n : j.needs)
                if (!done.count(n))
                    ready = false;
            if (ready) {
                result.push_back(&j);
                done.insert(j.name);
                progress = true;
            }
        }
        if (!progress) {
            std::cerr << "[mini-actions] dependency cycle or missing job\n";
            break;
        }
    }
    return result;
}

static int sh(const std::string &cmd) {
    std::cout << "    $ " << cmd << "\n";
    return std::system(cmd.c_str());
}

static int run_in_container(const Job &j) {
    std::string cmd = "docker run -i --rm " + j.image + " sh -s";
    std::cout << "    $ " << cmd << "\n";

    FILE *pipe = POPEN(cmd.c_str(), "w");
    if (!pipe) {
        std::cerr << "  cannot start docker\n";
        return 1;
    }

    std::fputs("set -e\n", pipe);
    for (const auto &step : j.steps)
        std::fputs((step + "\n").c_str(), pipe);

    int status = PCLOSE(pipe);
    return status;
}

int run_pipeline(const Pipeline &p) {
    std::cout << "== pipeline: " << p.name << " ==\n";
    auto jobs = order_jobs(p);

    for (const auto *j : jobs) {
        std::cout << "\n[job] " << j->name << "  (image: " << (j->image.empty() ? "host" : j->image)
                  << ")\n";

        if (!j->image.empty()) {
            std::cout << "  pulling container...\n";
            if (sh("docker pull " + j->image) != 0) {
                std::cerr << "  failed to pull image\n";
                return 1;
            }
            for (const auto &step : j->steps)
                std::cout << "  step: " << step << "\n";
            int code = run_in_container(*j);
            if (code != 0) {
                std::cerr << "  job failed (exit " << code << ")\n";
                return code;
            }
        } else {
            for (const auto &step : j->steps) {
                std::cout << "  step: " << step << "\n";
                int code = sh(step);
                if (code != 0) {
                    std::cerr << "  step failed (exit " << code << ")\n";
                    return code;
                }
            }
        }
        std::cout << "  [ok] " << j->name << "\n";
    }

    std::cout << "\n== all jobs done ==\n";
    return 0;
}
