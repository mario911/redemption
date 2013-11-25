/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */

#include "basic_benchmark.hpp"
#include "../regex_parser.hpp"
#include "../regex_automate.hpp"

#include <regex.h>

struct test_parser
{
    bool check_pre_condition(const char * s) const
    {
        const char * msg_err = 0;
        re::StateParser().compile(s, &msg_err);
        return msg_err == 0;
    }

    void exec(const char * s) const
    {
        re::StateParser().compile(s);
    }
};

struct test_compile
{
    bool check_pre_condition(const char * s) const
    {
        const char * msg_err = 0;
        re::StateParser().compile(s, &msg_err);
        return msg_err == 0;
    }

    void exec(const char * s) const
    {
        re::StateParser p;
        p.compile(s);
        re::StateMachine2(p.states(), p.root(), p.nb_capture());
    }
};

struct test_posix_compile
{
    bool check_pre_condition(const char * s) const
    {
        regex_t rgx;
        bool ret = !regcomp(&rgx, s, REG_EXTENDED);
        regfree(&rgx);
        return ret;
    }

    void exec(const char * s) const
    {
        regex_t rgx;
        regcomp(&rgx, s, REG_EXTENDED);
        regfree(&rgx);
    }
};

template<typename Test>
class Bench
{
    static void simple_pattern(const char * s)
    {
        test(basic_benchmark<Test>(s), 2000000u, s);
    }

    static void complexe_pattern(const char * name, const char * s)
    {
        test(basic_benchmark<Test>(name), 200000u, s);
    }

    static void complexe_pattern(const char * s)
    {
        complexe_pattern(s, s);
    }

public:
    Bench()
    {
        display_timer timer;

        simple_pattern("a");
        simple_pattern("a*");
        simple_pattern("a+");
        simple_pattern("a?");
        simple_pattern(".");
        simple_pattern(".*");
        simple_pattern(".+");
        simple_pattern(".?");
        simple_pattern("^a");
        simple_pattern("a$");
        simple_pattern("^a$");
        simple_pattern("\\w");
        simple_pattern("\\d");
        simple_pattern("\\s");
        simple_pattern("\\w*");
        simple_pattern("\\d*");
        simple_pattern("\\s*");

        complexe_pattern("numalpha", "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
        complexe_pattern("numalpha", "[0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz]");
        complexe_pattern("(?:\\d\\d(?: *|(?:\\.|-)?)){5}");
        complexe_pattern("(.*):(.*):(.*)");

        std::cout << "\ntotal: ";
    }
};

int main()
{
    std::cout << "parser:\n";
    Bench<test_parser>();
    std::cout << "\n\ncompile (parser+automate):\n";
    Bench<test_compile>();
    std::cout << "\n\nposix compile:\n";
    Bench<test_posix_compile>();
}