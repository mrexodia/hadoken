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

#ifndef HADOKEN_STATE_MACHINE_IMPL_HPP
#define HADOKEN_STATE_MACHINE_IMPL_HPP

#include "../state_machine.hpp"

namespace hadoken{


namespace impl{

} // impl

template<typename State>
int to_state_index_position(const State & s){
    return int(s);
}


template<typename State>
state_machine<State>::state_machine(State init_state) :
    _current_state(init_state){
    _resize(init_state);
}

template<typename State>
void state_machine<State>::_resize(const State & st){
    std::size_t pos = to_state_index_position(st);
    if(pos >= _handlers.size()){
       _handlers.resize(pos+1);
    }
}

template<typename State>
State state_machine<State>::get_current_state() const{
    return _current_state;
}


template<typename State>
State state_machine<State>::trigger(){
    auto old_state = _current_state;

    auto & handler = _handlers.at(to_state_index_position(_current_state));
    auto & transitions = handler._transitions;

    // for every registered transition
    // check if the conditional function is validated
    for(auto & transition : transitions ){

        if(transition._cond && transition._cond()){
            const State new_transtion = transition._next;
            auto & new_handler = _handlers.at(to_state_index_position(new_transtion));

            if(handler._on_exit){
                handler._on_exit(_current_state, new_transtion);
            }

            const State old_transition = _current_state;
            _current_state = new_transtion;

            if(new_handler._on_entry){
                new_handler._on_entry(old_transition, _current_state);
            }

            return old_state;
        }
    }

    return old_state;
}

template<typename State>
State state_machine<State>::force_state(State state){
    using namespace std;
    _resize(state);
    swap(_current_state, state);
    return state;
}

template<typename State>
void state_machine<State>::add_transition(State from, State to, std::function<bool ()> condition){
    _resize(from);
    _resize(to);

    auto & state_hdle = _handlers.at(to_state_index_position(from));

    impl::transition_handler<State> handler;
    handler._cond = condition;
    handler._next = to;

    state_hdle._transitions.push_back(handler);
}

template<typename State>
void state_machine<State>::on_entry(State st, std::function<void (State, State)> event){
    _resize(st);

    auto & state_hdle = _handlers.at(to_state_index_position(st));
    state_hdle._on_entry = std::move(event);
}

template<typename State>
void state_machine<State>::on_exit(State st, std::function<void (State, State)> event){
    _resize(st);

    auto & state_hdle = _handlers.at(to_state_index_position(st));
    state_hdle._on_exit = std::move(event);
}

template<typename State>
void state_machine<State>::clear(){
    _handlers.clear();
    _resize(_current_state);
}

template<typename Object>
edge_trigger<Object>::edge_trigger(Object &&o) :
    _o(o),
    _is_new(false){}

template<typename Object>
void edge_trigger<Object>::trigger(Object &&o){
    _o = std::move(o);
    _is_new = true;
}

template<typename Object>
hadoken::optional<Object> edge_trigger<Object>::consume(){
    hadoken::optional<Object> result;
    if(_is_new){
        _is_new = false;
        result = _o;
    }
    return result;
}

} // hadoken

#endif // HADOKEN_STATE_MACHINE_IMPL_HPP