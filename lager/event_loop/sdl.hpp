//
// lager - library for functional interactive c++ programs
// Copyright (C) 2017 Juan Pedro Bolivar Puente
//
// This file is part of lager.
//
// lager is free software: you can redistribute it and/or modify
// it under the terms of the MIT License, as detailed in the LICENSE
// file located at the root of this source code distribution,
// or here: <https://github.com/arximboldi/lager/blob/master/LICENSE>
//

#pragma once

#include <SDL.h>

#include <atomic>
#include <functional>

#include <cassert>
#include <cstddef>

namespace lager {

struct with_sdl_event_loop;

struct sdl_event_loop
{
    using event_fn = std::function<void()>;

    template <typename Fn>
    void run(Fn&& handler)
    {
        auto continue_ = true;
        while (continue_ && !done_) {
            auto event = SDL_Event{};
            if (SDL_WaitEvent(&event)) {
                if (event.type == post_event_type_) {
                    auto fnp = reinterpret_cast<event_fn*>(&event.user.data1);
                    (*fnp)();
                    fnp->~event_fn();
                } else {
                    continue_ = handler(event);
                }
            }
        }
    }

    void post(event_fn ev)
    {
        static_assert(sizeof(event_fn) <=
                      sizeof(SDL_Event) - offsetof(SDL_Event, user.data1),
                      "Ooops! A funciton does not fit in the SDL_Event");
        auto event = SDL_Event{};
        event.type = post_event_type_;
        auto region = static_cast<void*>(&event.user.data1);
        auto obj = new (region) event_fn{std::move(ev)};
        assert((void*) obj == (void*) &event.user.data1);
        SDL_PushEvent(&event);
    }

    void finish()
    {
        done_ = true;
    }

private:
    friend with_sdl_event_loop;

    std::atomic<bool> done_ {false};
    std::uint32_t post_event_type_ = SDL_RegisterEvents(1);
};

struct with_sdl_event_loop
{
    std::reference_wrapper<sdl_event_loop> loop;

    template <typename Fn>
    void async(Fn&& fn)
    {
        throw std::logic_error{"not implemented!"};
    }

    template <typename Fn>
    void post(Fn&& fn)
    {
        loop.get().post(std::forward<Fn>(fn));
    }

    void finish()
    {
        loop.get().finish();
    }
 };

} // namespace lager