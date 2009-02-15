/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_OUTPUT_MANAGER_HH
#define PALUDIS_GUARD_PALUDIS_OUTPUT_MANAGER_HH 1

#include <paludis/output_manager-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/instantiation_policy.hh>
#include <iosfwd>

namespace paludis
{
    class PALUDIS_VISIBLE OutputManager :
        private InstantiationPolicy<OutputManager, instantiation_method::NonCopyableTag>
    {
        public:
            virtual ~OutputManager() = 0;

            virtual std::ostream & stdout_stream() PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
            virtual std::ostream & stderr_stream() PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * An out of band message that might want to be logged or handled
             * in a special way.
             *
             * The caller must still also display the message to
             * stdout_stream() as appropriate.
             */
            virtual void message(const MessageType, const std::string &) = 0;

            /**
             * Called if an action succeeds. This can be used to, for example,
             * unlink the files behind a to-disk logged output manager.
             *
             * If an OutputManager is destroyed without having had this method
             * called, it should assume failure. This might mean keeping rather
             * than removing log files, for example.
             *
             * Further messages and output may occur even after a call to this
             * method.
             *
             * Calls to this method are done by the caller, not by whatever
             * carries out the action in question.
             */
            virtual void succeeded() = 0;
    };
}

#endif
