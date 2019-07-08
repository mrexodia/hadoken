/**
 * Copyright (c) 2016, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 * Boost Software License - Version 1.0
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#define BOOST_TEST_MODULE stringTests
#define BOOST_TEST_MAIN

#include <chrono>
#include <iostream>
#include <map>
#include <stdexcept>

#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>


#include <hadoken/string/algorithm.hpp>
#include <hadoken/string/string_view.hpp>
#include <hadoken/string/wildcard.hpp>

#include <hadoken/utility/range.hpp>


#include "test_helpers.hpp"



BOOST_AUTO_TEST_CASE(string_view_simple) {
    using namespace hadoken;

    const char* msg = "hello bob #42~€é";

    string_view empty, truncated(msg, 5), full(msg);

    BOOST_CHECK_EQUAL(empty.empty(), true);
    BOOST_CHECK_EQUAL(truncated.empty(), false);
    BOOST_CHECK_EQUAL(full.empty(), false);


    BOOST_CHECK_EQUAL(empty.size(), 0);
    BOOST_CHECK_EQUAL(empty.size(), empty.length());
    BOOST_CHECK_EQUAL(truncated.size(), 5);
    BOOST_CHECK_EQUAL(truncated.size(), truncated.length());
    BOOST_CHECK_EQUAL(full.size(), std::string(msg).size());

    string_view new_view_copied = full;
    string_view truncated_copy = truncated;
    string_view new_view_moved = std::move(truncated_copy);

    BOOST_CHECK_EQUAL(full, new_view_copied);
    BOOST_CHECK_EQUAL(new_view_moved, truncated);

    std::ostringstream ss;
    ss << truncated;
    BOOST_CHECK_EQUAL(ss.str(), to_string(truncated));
}



BOOST_AUTO_TEST_CASE(string_tokenize_view) {
    using namespace hadoken;

    const char* msg = "hello bob #42~€é tada";

    std::vector<std::string> res = string::split_string(msg, " ");

    BOOST_CHECK_EQUAL(res.size(), 4);
}


void test_wildcard(const std::string & wildcard, const std::string & str, bool result){
    const bool res = hadoken::match_wildcard(wildcard, str);
    std::cout << " test " << wildcard << " " << str << "\n";
    BOOST_CHECK_EQUAL(res, result);
}

BOOST_AUTO_TEST_CASE(wildcard_simple) {

    auto t1 = std::chrono::steady_clock::now();

    std::vector<std::tuple<std::string, std::string, bool> > test_cases = {
        { "hello*ld", "hello world", true },
        { "dude" , "hello world", false},
        { "h**", "hello world", true },
        { "*hello*", "hello world", true },
        { "world*", "hello world", false },
        { "*", "hello world", true },
        { "**dude**", "hello world", false },
        { "**blabla", "blafdfd", false },
        { "***", "hello*world*", true },
        { "**deb", "debug1", false },
        { "hello wo*rld", "hello world", true},
        { "hello world*", "hello world", true}
    };

    for(const auto & t : test_cases){
        test_wildcard(std::get<0>(t), std::get<1>(t), std::get<2>(t));
    }


    auto t2 = std::chrono::steady_clock::now();

    std::cout << "time taken wildcard " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count())
              << " µs" << std::endl;
}


BOOST_AUTO_TEST_CASE(wildcard_stack_explosion_test) {

    std::string message, pattern, bad_pattern;

    message = random_string_generator(16 * 1024 * 1024, 42);

    std::cout << " " << message.size() << std::endl;

    pattern = message;

    pattern[pattern.size() / 2] = '*';
    pattern[pattern.size() / 4] = '*';
    pattern[pattern.size() / 8] = '*';

    bad_pattern = pattern;
    bad_pattern.back() = bad_pattern.back() + 1;



    auto t1 = std::chrono::steady_clock::now();
    BOOST_CHECK(hadoken::match_wildcard(pattern, message));

    BOOST_CHECK(hadoken::match_wildcard(bad_pattern, message) == false);

    auto t2 = std::chrono::steady_clock::now();

    std::cout << "time taken "
              << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000.0 / 1000.0 << " s"
              << std::endl;
}
