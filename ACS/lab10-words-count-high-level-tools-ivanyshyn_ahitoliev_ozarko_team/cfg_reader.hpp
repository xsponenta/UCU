#ifndef INTEGRATE_PARALLEL_TPOOL_CFG_READER_HPP
#define INTEGRATE_PARALLEL_TPOOL_CFG_READER_HPP

#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>
#include <oneapi/tbb/cache_aligned_allocator.h>
#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_unordered_map.h>
#include <tbb/task_group.h>
#include <tbb/parallel_for.h>

namespace po = boost::program_options;

enum parsing_errors {
    OK = 0,
    TO_MANY_ARGUMENTS = 1,
    TO_BIG_FUNCNUM = 2,
    FAILED_FILE_OPENING = 3,
    ERROR_READING_CFG = 5
};

struct integral_config {
    std::string indir;
    std::string out_by_a;
    std::string out_by_n;
    std::string indexing_extensions;
    std::string archives_extensions;
    size_t max_file_size;
    size_t indexing_threads;
    size_t merging_threads;
    size_t filenames_queue_size;
    size_t raw_files_queue_size;
    size_t dictionaries_queue_size;

};

parsing_errors read_cfg(integral_config& cfg, std::string& cfgfile) {
    try {
        po::options_description config_options("integral_config");
        config_options.add_options()
                ("indir", po::value<std::string>())
                ("out_by_a", po::value<std::string>())
                ("out_by_n", po::value<std::string>())
                ("indexing_extensions", po::value<std::string>())
                ("archives_extensions", po::value<std::string>())
                ("max_file_size", po::value<size_t>())
                ("indexing_threads", po::value<size_t>())
                ("merging_threads", po::value<size_t>())
                ("filenames_queue_size", po::value<size_t>())
                ("raw_files_queue_size", po::value<size_t>())
                ("dictionaries_queue_size", po::value<size_t>());

        po::variables_map vm;
        std::ifstream config_file(cfgfile);

        if (!config_file) {
            std::cerr << "Failed to open config file." << std::endl;
            return FAILED_FILE_OPENING;
        }

        po::store(po::parse_config_file(config_file, config_options), vm);
        config_file.close();
        if (vm.count("indir")) cfg.indir = vm["indir"].as<std::string>();
        if (vm.count("out_by_a")) cfg.out_by_a = vm["out_by_a"].as<std::string>();
        if (vm.count("out_by_n")) cfg.out_by_n = vm["out_by_n"].as<std::string>();
        if (vm.count("indexing_extensions")) cfg.indexing_extensions = vm["indexing_extensions"].as<std::string>();
        if (vm.count("archives_extensions")) cfg.archives_extensions = vm["archives_extensions"].as<std::string>();
        if (vm.count("max_file_size")) cfg.max_file_size = vm["max_file_size"].as<size_t>();
        if (vm.count("indexing_threads")) cfg.indexing_threads = vm["indexing_threads"].as<size_t>();
        if (vm.count("merging_threads")) cfg.merging_threads = vm["merging_threads"].as<size_t>();
        if (vm.count("filenames_queue_size")) cfg.filenames_queue_size = vm["filenames_queue_size"].as<size_t>();
        if (vm.count("raw_files_queue_size")) cfg.raw_files_queue_size = vm["raw_files_queue_size"].as<size_t>();
        if (vm.count("dictionaries_queue_size")) cfg.dictionaries_queue_size = vm["dictionaries_queue_size"].as<size_t>();
    } catch (const po::error &ex) {
        std::cerr << "Error parsing config file: " << ex.what() << std::endl;
        return ERROR_READING_CFG;
    }
    return OK;
}
#endif
