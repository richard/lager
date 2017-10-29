//
// lager - library for functional interactive c++ programs
// Copyright (C) 2017 Juan Pedro Bolivar Puente
//
// This file is part of lager.
//
// lager is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// lager is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with lager.  If not, see <http://www.gnu.org/licenses/>.
//

#pragma once

#include <lager/util.hpp>

#include <immer/array.hpp>
#include <immer/box.hpp>
#include <immer/vector.hpp>

#include <variant>

namespace lager {

template <typename Action, typename Model>
struct debugger
{
    using base_action = Action;
    using base_model  = Model;

    using cursor_t = std::size_t;

    struct goto_action { cursor_t cursor; };
    struct undo_action {};
    struct redo_action {};

    using action = std::variant<
        Action,
        goto_action,
        undo_action,
        redo_action>;

    struct model
    {
        struct step
        {
            Action action;
            Model model;
        };

        Model init;
        cursor_t cursor = {};
        immer::vector<step> history = {};

        model(Model i) : init{i} {}

        operator const Model& () const {
            return cursor == history.size()
                ? (history.size() ? history.back().model : init)
                : history[cursor].model;
        }
    };

    template <typename ReducerFn>
    static model update(ReducerFn&& reducer, model m, action act)
    {
        return std::visit(visitor{
                [&] (Action act) {
                    m.history = m.history
                        .take(m.cursor)
                        .push_back({act, reducer(m, act)});
                    m.cursor = m.history.size();
                    return m;
                },
                [&] (goto_action act) {
                    if (act.cursor < m.history.size())
                        m.cursor = act.cursor;
                    return m;
                },
                [&] (undo_action) {
                    if (m.cursor)
                        --m.cursor;
                    return m;
                },
                [&] (redo_action) {
                    if (m.cursor < m.history.size())
                        ++m.cursor;
                    return m;
                },
            }, act);
    }

    template <typename Server, typename ViewFn>
    static void view(Server& serv, ViewFn&& view, const model& m)
    {
        serv.view(m);
        std::forward<ViewFn>(view)(m);
    }
};

} // namespace lager
