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

#define BOOST_TEST_MODULE cryptoTests
#define BOOST_TEST_MAIN

#include <iostream>
#include <string>


#include <boost/array.hpp>
#include <boost/integer.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>

#include <hadoken/crypto/crypto.hpp>



void test_base64_base(const std::string & origin, const std::string & encoded){
    auto mencoded = hadoken::base64::encode(origin);
    BOOST_CHECK_EQUAL(mencoded, encoded);

    auto mdecoded = hadoken::base64::decode(mencoded);
    BOOST_CHECK_EQUAL(mdecoded, origin);
}



BOOST_AUTO_TEST_CASE(base64_tests) {

    // test empty
    test_base64_base("", "");

    // test simple \n double complement
    test_base64_base("\n", "Cg==");

    // test simple complement
    test_base64_base("hello world", "aGVsbG8gd29ybGQ=");

    // test no complement
    test_base64_base("hel", "aGVs");


    // test invalid content decode
    std::string binary_content;
    for(unsigned char a = 0; a < std::numeric_limits<unsigned char>::max(); ++a){
        binary_content.push_back(char(a));
    }

    BOOST_CHECK_THROW({
        auto res = hadoken::base64::decode(binary_content);
        BOOST_FAIL("should not reach here");

    }, hadoken::base64::base64_exception
    );
}

BOOST_AUTO_TEST_CASE(sha1_basic_tests) {
    std::string str("hello world");
    std::string hash_str("2aae6c35c94fcfb415dbe95f408b9ce91ee846ed");

    hadoken::sha1 sha_compute;

    sha_compute.process_block(str.c_str(), str.size());

    std::string res = sha_compute.to_string();

    BOOST_CHECK_EQUAL(hash_str, res);


    std::cout << "sha1_hello_world:" << res << "\n";

    // give a second try to check corruption protection
    res = sha_compute.to_string();

    BOOST_CHECK_EQUAL(hash_str, res);
}


BOOST_AUTO_TEST_CASE(sha1_basic_tests2) {
    std::string str("The quick brown fox jumps over the lazy cog");
    std::string hash_str("de9f2c7fd25e1b3afad3e85a0bd17d9b100db4b3");

    hadoken::sha1 sha_compute;

    sha_compute.process_block(str.c_str(), str.size());

    std::string res = sha_compute.to_string();

    BOOST_CHECK_EQUAL(hash_str, res);
}


BOOST_AUTO_TEST_CASE(hash_integer) {
    int value = 42;

    {
        hadoken::sha1 sha_compute;
        std::string res = sha_compute.to_string();

        std::cout << "empty_hash " << res << "\n";
        BOOST_CHECK_EQUAL("da39a3ee5e6b4b0d3255bfef95601890afd80709", res);
    }

    {
        hadoken::sha1 sha_compute;

        sha_compute.process(boost::uint32_t(value));

        std::string res = sha_compute.to_string();



        std::cout << "integer_hash " << res << "\n";
        BOOST_CHECK_EQUAL("25f0c736f1fad0770bbb9a265ded159517c1e68c", res);
    }
}
