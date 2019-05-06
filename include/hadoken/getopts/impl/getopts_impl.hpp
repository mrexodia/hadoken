/**
 * Copyright (c) 2018, Adrien Devresse <adrien.devresse@epfl.ch>
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
#pragma once

#include <algorithm>
#include <numeric>

#include "../getopts.hpp"

#include "../../format/format.hpp"

namespace hadoken{


constexpr int _option_flag_require_none = 0;
constexpr int _option_flag_require_str = 1;
constexpr int _option_flag_require_int = 2;

const std::string _help_opt = "--help";


template<typename Iterator>
void _call_next_argument_string(const option & opt, bool call, Iterator begin_arg, Iterator end_arg){
    auto argument = _get_next_argument_generic(opt.name(), begin_arg, end_arg);

    if(_is_prefixed_option(argument)){
        throw parse_options_error(scat("invalid option argument for ", opt.name()));
    }

    try{
        if(call){
            opt._call(argument);
        }
    }catch(std::exception & e){
        throw parse_options_error(scat("invalid value ", *begin_arg, " for ", opt.name(), " <string> argument"));
    }
}

template<typename Iterator>
std::size_t _get_max_print_table_column_length(std::size_t min, std::size_t min_right_margin, Iterator begin, Iterator end){
    std::size_t max_size = 0;
    std::for_each(begin, end, [&](const decltype(*begin) & elem){
       max_size = std::max<std::size_t>(max_size, elem.size());
    });
    max_size += min_right_margin;
    return std::max<std::size_t>(max_size, min);
}

void _concat_as_colmun(std::ostream & ss, string_view text, std::size_t column_length){
    for(const auto & c : text){
        ss.put(c);
        column_length = std::max<std::size_t>(column_length, 1) - 1;
    }

    for(std::size_t i = 0; i < column_length; ++i){
        ss.put(' ');
    }

}

inline options_handler::options_handler(std::string name, std::string help_msg) :
    _flags(),
    _name(std::move(name)),
    _help_msg(std::move(help_msg)){}


inline void options_handler::add_option(option opt){
    _opts.emplace_back(std::move(opt));
}

inline void options_handler::add_subcommand(sub_command sub_com){
    _subcmd.emplace_back(std::move(sub_com));
}

inline void options_handler::set_positional_argument_handler(std::function<void (const std::string &)> handler){
    _positional_argument = handler;
}

inline void options_handler::set_flags(flag f, bool v){
    _flags[int(f)] = v;
}

inline bool options_handler::get_flag(flag f) const{
    return _flags[int(f)];
}


const std::vector<sub_command> & options_handler::sub_commands() const{
    return _subcmd;
}

const std::vector<option> & options_handler::options() const{
    return _opts;
}

inline string_view options_handler::name() const{
    return _name;
}

inline string_view options_handler::help_message() const{
    return _help_msg;
}


std::string options_handler::help(const help_format & fmt) const{
    const std::string padding_str(fmt.padding, ' ');
    const std::string margin_str(fmt.margin, ' ');

    std::vector<string_view> options_strings, commands_string;
    std::transform(_opts.begin(), _opts.end(), std::back_inserter(options_strings), [](const option & o){ return o.name(); });
    std::transform(_subcmd.begin(), _subcmd.end(), std::back_inserter(commands_string), [](const sub_command & s){ return s.name(); });

    std::size_t colmun_length = std::max<std::size_t>(
                _get_max_print_table_column_length(8, 2, options_strings.begin(), options_strings.end()),
                _get_max_print_table_column_length(8, 2, commands_string.begin(), commands_string.end()));

    std::ostringstream ss;
    ss << margin_str << name() << ": " << help_message() << "\n\n";
    ss << margin_str << "Commands:" << "\n";

    for(const auto & cmd : sub_commands() ){
        ss << margin_str << padding_str;
        _concat_as_colmun(ss, cmd.name(), colmun_length);
        ss << cmd.help_message() << "\n";
    }

    ss << "\n";
    ss << padding_str << "Options:" << "\n";



    if(get_flag(flag::no_help) == false){
        ss << margin_str << padding_str;
        _concat_as_colmun(ss, _help_opt, colmun_length);
        ss << "print help \n";
    }
    for(const auto & opt : options() ){
        ss << margin_str << padding_str;
        _concat_as_colmun(ss, opt.name(), colmun_length);
        ss << opt.help_message() << "\n";
    }

    return ss.str();
}


void options_handler::_call_positional(std::string arg)  const{
    if(! _positional_argument){
        throw parse_options_error(hadoken::scat("invalid positional argument ", arg));
    }
    _positional_argument(arg);
}


inline sub_command::sub_command(std::string subcommand_name, std::function<void (void)> callback, std::string help_msg) :
    options_handler(subcommand_name, help_msg),    _action(callback){

}


inline void sub_command::_call() const{
    _action();
}


inline option::option(std::string option_name, std::function<void (const std::string &)> callback, std::string help_msg) :
    _names({ std::move(option_name)} ),
    _help_msg(std::move(help_msg)),
    _action([callback](string_view arg) -> void{
    callback(to_string(arg));
    })
{
    _flags[_option_flag_require_str] = true;
}


inline option::option(std::string option_name, std::function<void (int)> callback, std::string help_msg) :
    _names({ std::move(option_name)}),
    _help_msg(std::move(help_msg)),
    _action([callback, this](string_view arg) -> void{
    callback(std::stoi(to_string(arg)));
})
{
    _flags[_option_flag_require_int] = true;
}

inline option::option(std::string option_name, std::function<void ()> callback, std::string help_msg) :
    _names({std::move(option_name)}),
    _help_msg(std::move(help_msg)),
    _action([callback, this](string_view) -> void{
    callback();
})
{
    _flags[_option_flag_require_none] = true;
}


inline  string_view option::name() const{
    return _names.at(0);
}

inline std::vector<string_view> option::name_and_aliases() const{
    std::vector<string_view> res;
    res.reserve(_names.size());

    for(const auto & n : _names){
        res.emplace_back(n);
    }
    return res;
}

inline void option::add_alias(std::string alias){
    _names.emplace_back(alias);
}

inline bool option::match(string_view opt) const{
    return std::any_of(_names.begin(), _names.end(), [&](const std::string & v){
        return opt.compare(v) == 0;
    });
}

inline void option::_call(string_view value) const{
    _action(to_string(value));
}

inline bool option::_get_flag(int v) const{
    return _flags[v];
}

inline string_view option::help_message() const{
    return _help_msg;
}

bool _is_valid_subcommand(string_view candidate_sub, string_view sub){
    if(candidate_sub.size() == 0){
        throw std::invalid_argument("empty option is not suppored");
    }
    return candidate_sub.compare(sub) == 0;
}

bool _is_prefixed_option(string_view candidate_opt){
    return (candidate_opt.size() > 0 && candidate_opt[0] == '-');
}

bool _end_of_arguments(string_view candidate_opt){
    return (candidate_opt.size() == 2 && candidate_opt[0] == '-' && candidate_opt[1] == '-');
}

template<typename Iterator>
string_view _get_next_argument_generic(string_view option_name, Iterator begin_arg, Iterator end_arg){
    if(begin_arg +1 >= end_arg){
        throw parse_options_error(scat("missing value for option ", option_name));
    }
    return *(begin_arg +1);
}

template<typename Iterator>
void _call_next_argument_int(const option & opt, bool call, Iterator begin_arg, Iterator end_arg){
    auto argument = _get_next_argument_generic(opt.name(), begin_arg, end_arg);

    try{
        (void) std::stoi(to_string(argument));
        if(call){
            opt._call(argument);
        }
    }catch(std::exception & e){
        throw parse_options_error(scat("invalid value ", *begin_arg, " for ", opt.name(), " <int> argument"));
    }
}



template<typename Iterator>
inline bool _validate_and_call(std::vector<options_handler const *> stack, string_view prog_name,
                               Iterator begin_arg, Iterator end_arg,
                               bool call, bool positional_only){

    const options_handler & opt_handler = *stack.back();

    if (begin_arg >= end_arg){

        if(opt_handler.get_flag(options_handler::flag::only_subcmd)){
            throw parse_options_error("invalid argument: no subcommand");
        }
        return true;
    }

    if(_end_of_arguments(*begin_arg)){
        return _validate_and_call(stack, prog_name, begin_arg +1, end_arg, call, true);
    }

    // check for subcommands
    for(const sub_command & sub : opt_handler.sub_commands()){
        if(_is_valid_subcommand(sub.name(), *begin_arg)){
            stack.emplace_back(&sub);
            if(call){
                sub._call();
            }
            return _validate_and_call(stack, prog_name, begin_arg +1, end_arg, call, positional_only);
        }
    }

    // check for options
    if(_is_prefixed_option(*begin_arg)){
        if(opt_handler.get_flag(options_handler::flag::no_help) == false && begin_arg->compare(_help_opt) == 0){
            std::ostringstream ss;
            ss << "Usage : ";
            for(const auto & s : stack){
                ss << s->name() << " ";
            }
            ss << "[OPTIONS]" << "\n";
            ss << "\n";
            ss << opt_handler.help();
            throw parse_options_error(ss.str());
        }

        for(const option & opt : opt_handler.options()){
            if(opt.match(*begin_arg)){
                off_t offset = 1;
                if(opt._get_flag(_option_flag_require_none)){
                    if(call){
                        opt._call("");
                    }
                } else if(opt._get_flag(_option_flag_require_int)){
                    _call_next_argument_int(opt, call, begin_arg, end_arg);
                    offset = 2;
                }else if(opt._get_flag(_option_flag_require_str)){
                    _call_next_argument_string(opt, call, begin_arg, end_arg);
                    offset = 2;
                }else{
                    throw parse_options_error("invalid parser configuration");
                }
                return _validate_and_call(stack, prog_name, begin_arg + offset, end_arg, call, positional_only);
            }
        }
        throw parse_options_error(scat("argument ", *begin_arg, " is not supported"));
    }

    // positional argument
    if(call){
        opt_handler._call_positional(to_string(*begin_arg));
    }
    return _validate_and_call(stack, prog_name, begin_arg +1, end_arg, call, positional_only);
}



inline void parse_options(const options_handler & opt_handler, int argc, char** argv){
    std::vector<string_view> options;
    for(int i = 1; i < argc; ++i){
        options.emplace_back(argv[i]);
    }
    parse_options(opt_handler, string_view(argv[0]), options);
}


inline void parse_options(const options_handler & opt_handler, string_view prog_name, const std::vector<string_view> & options){

    std::vector<options_handler const *> stack;
    stack.push_back(&opt_handler);

    // dry run, check options
    try{
         _validate_and_call(stack, prog_name, options.begin(), options.end(), false, false);
    }catch(parse_options_error & e){
        std::ostringstream ss;
        ss << prog_name << ": "<<  e.what();
        ss << "\n";
        ss << prog_name << ": " << "try `" << prog_name << " --help`" << " for more information ";
        throw parse_options_error(ss.str());
    }

    // call callbacks
    _validate_and_call(stack, prog_name, options.begin(), options.end(), true, false);
}

} // hadoken
