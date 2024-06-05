//
// Created by xsponenta on 13.04.24.
//

#include <iostream>
#include <unordered_map>
#include <string>
#include <boost/locale.hpp>
#include "counter.hpp"

std::unordered_map<std::string, size_t> count_words(const std::string& input_str) {
    boost::locale::generator gen;
    std::locale loc = gen("");

    auto normalized = boost::locale::normalize(input_str, boost::locale::norm_nfd, loc);

    auto folded_case = boost::locale::fold_case(normalized, loc);

    boost::locale::boundary::ssegment_index split(boost::locale::boundary::word, folded_case.begin(), folded_case.end(), loc);
    split.rule(boost::locale::boundary::word_letters);

    std::unordered_map<std::string, size_t> word_counts;
    for (boost::locale::boundary::ssegment_index::iterator iter = split.begin(), e = split.end(); iter != e; ++iter) {
        std::string word = *iter;
        if (word_counts.find(word) != word_counts.end()) {
            word_counts[word]++;
        } else {
            word_counts[word] = 1;
        }
    }

    return word_counts;
}