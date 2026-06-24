#include "runner.hpp"
#include <cstdlib>
#include <iostream>
#include <map>
#include <set>
#include <vector>

// упорядочить job'ы по зависимостям (needs).
// наивный topo-sort: крутимся, пока всё не разложим.
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
        }

        for (const auto &step : j->steps) {
            std::cout << "  step: " << step << "\n";
            std::string full;
            if (!j->image.empty())
                full = "docker run --rm " + j->image + " sh -c \"" + step + "\"";
            else
                full = step;
            int code = sh(full);
            if (code != 0) {
                std::cerr << "  step failed (exit " << code << ")\n";
                return code;
            }
        }
        std::cout << "  [ok] " << j->name << "\n";
    }

    std::cout << "\n== all jobs done ==\n";
    return 0;
}
