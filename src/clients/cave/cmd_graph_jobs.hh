/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_SRC_CLIENTS_CAVE_CMD_GRAPH_JOBS_HH
#define PALUDIS_GUARD_SRC_CLIENTS_CAVE_CMD_GRAPH_JOBS_HH 1

#include "command.hh"
#include <paludis/resolver/resolved-fwd.hh>

namespace paludis
{
    namespace cave
    {
        class PALUDIS_VISIBLE GraphJobsCommand :
            public Command
        {
            public:
                CommandImportance importance() const override PALUDIS_ATTRIBUTE((warn_unused_result));

                int run(
                        const std::shared_ptr<Environment> &,
                        const std::shared_ptr<const Sequence<std::string > > & args
                        ) override;

                int run(
                        const std::shared_ptr<Environment> &,
                        const std::shared_ptr<const Sequence<std::string > > & args,
                        const std::shared_ptr<const resolver::Resolved> & maybe_resolved
                        );

                std::shared_ptr<args::ArgsHandler> make_doc_cmdline() override;
        };
    }
}

#endif
