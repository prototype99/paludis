/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Kim Højgaard-Hansen
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "command_line.hh"
#include <paludis/args/man.hh>
#include <paludis/args/do_help.hh>

#include <iostream>
#include <cstdlib>

using std::cout;
using std::endl;

namespace
{
    struct ManCommandLine :
        paludis::args::ArgsHandler
    {
        virtual std::string app_name() const
        {
            return "";
        }

        virtual std::string app_description() const
        {
            return "";
        }

        virtual std::string app_synopsis() const
        {
            return "";
        }
    };
}

int
main(int argc, char * argv[])
{
    ManCommandLine cmdline;
    cmdline.run(argc, argv, "", "", "");

    auto w(std::make_shared<paludis::args::AsciidocWriter>(cout));

    paludis::args::generate_doc(*w, CommandLine::get_instance());

    return EXIT_SUCCESS;
}

