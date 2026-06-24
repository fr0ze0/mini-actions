#pragma once
#include <string>
#include <vector>

struct Job {
    std::string name;
    std::string image;              // docker image
    std::vector<std::string> needs; // jobs names
    std::vector<std::string> steps; // commands
};

struct Pipeline {
    std::string name;
    std::vector<Job> jobs;
};

Pipeline parse_config(const std::string &path);
