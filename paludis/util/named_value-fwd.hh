/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_NAMED_VALUE_FWD_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_NAMED_VALUE_FWD_HH 1

#include <string>
#include <paludis/util/tribool-fwd.hh>

namespace paludis
{
    template <typename K_, typename V_>
    class NamedValue;

    template <typename K_, typename V_>
    NamedValue<K_, V_>
    value_for(const V_ & v);

    /* Hack: let "foo" work for strings, but ban other magic conversions */
    template <typename K_>
    NamedValue<K_, std::string>
    value_for(const char * const v);

    /* Hack: let indeterminate work for Tribools, but ban other magic conversions */
    template <typename K_>
    NamedValue<K_, Tribool>
    value_for(TriboolIndeterminateValueType);
}

#endif
