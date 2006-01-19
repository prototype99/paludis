/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include "dep_list.hh"
#include "dep_list_error.hh"
#include "dep_list_stack_too_deep_error.hh"
#include "dep_parser.hh"
#include "all_masked_error.hh"
#include "no_resolvable_option_error.hh"
#include "circular_dependency_error.hh"
#include "internal_error.hh"
#include "all_dep_atom.hh"
#include "any_dep_atom.hh"
#include "block_dep_atom.hh"
#include "use_dep_atom.hh"
#include "package_dep_atom.hh"
#include "stringify.hh"
#include "container_entry.hh"
#include "save.hh"
#include "indirect_iterator.hh"
#include "block_error.hh"
#include "join.hh"
#include "filter_insert_iterator.hh"

#include <algorithm>
#include <functional>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<DepList> :
        InstantiationPolicy<Implementation<DepList>, instantiation_method::NonCopyableTag>,
        InternalCounted<Implementation<DepList> >
    {
        const Environment * const environment;

        std::list<DepListEntry> merge_list;
        std::list<DepListEntry> pending_list;
        bool check_existing_only;
        bool in_pdepend;
        bool match_found;
        const PackageDatabaseEntry * current_package;
        int stack_depth;

        DepListRdependOption rdepend_post;
        bool drop_circular;
        bool drop_self_circular;
        bool ignore_installed;
        bool recursive_deps;
        int max_stack_depth;

        Implementation(const Environment * const e) :
            environment(e),
            check_existing_only(false),
            in_pdepend(false),
            match_found(false),
            current_package(0),
            stack_depth(0),
            rdepend_post(dlro_as_needed),
            drop_circular(false),
            drop_self_circular(false),
            ignore_installed(false),
            recursive_deps(false),
            max_stack_depth(100)
        {
        }
    };
}

DepList::DepList(const Environment * const e) :
    PrivateImplementationPattern<DepList>(new Implementation<DepList>(e))
{
}

DepList::~DepList()
{
}

void
DepList::add(DepAtom::ConstPointer atom)
{
    /* keep track of stack depth */
    Save<int> old_stack_depth(&_implementation->stack_depth,
            _implementation->stack_depth + 1);
    if (_implementation->stack_depth > _implementation->max_stack_depth)
        throw DepListStackTooDeepError(_implementation->stack_depth);

    /* we need to make sure that merge_list doesn't get h0rked in the
     * event of a failure. */
    bool merge_list_was_empty(_implementation->merge_list.empty());
    std::list<DepListEntry>::iterator m_save(merge_list_was_empty ?
            _implementation->merge_list.end() : --(_implementation->merge_list.end()));

    try
    {
        atom->accept(this);
    }
    catch (...)
    {
        if (merge_list_was_empty)
            _implementation->merge_list.clear();
        else
            _implementation->merge_list.erase(++m_save, _implementation->merge_list.end());
        throw;
    }
}

void
DepList::_add_in_role(DepAtom::ConstPointer atom, const std::string & role)
{
    Context context("When adding " + role + ":");
    add(atom);
}

DepList::Iterator
DepList::begin() const
{
    return _implementation->merge_list.begin();
}

DepList::Iterator
DepList::end() const
{
    return _implementation->merge_list.end();
}

void
DepList::visit(const AllDepAtom * const v)
{
    for (CompositeDepAtom::Iterator i(v->begin()), i_end(v->end()) ;
            i != i_end ; ++i)
        (*i)->accept(this);
}

#ifndef DOXYGEN
struct DepListEntryMatcher :
    public std::unary_function<bool, const DepListEntry &>
{
    const PackageDepAtom & atom;

    DepListEntryMatcher(const PackageDepAtom & p) :
        atom(p)
    {
    }

    bool operator() (const DepListEntry & e) const
    {
        if (e.get<dle_name>() != atom.package())
            return false;
        if (atom.slot_ptr())
            if (e.get<dle_slot>() != *atom.slot_ptr())
                return false;
        if (atom.version_spec_ptr())
            if (! (((e.get<dle_version>()).*(atom.version_operator().as_version_spec_operator()))(
                        *atom.version_spec_ptr())))
                return false;
        return true;
    }
};
#endif

void
DepList::visit(const PackageDepAtom * const p)
{
    Context context("When resolving package dependency '" + stringify(*p) + "':");

    bool already_there = false;
    bool do_rdepend_post = (_implementation->rdepend_post == dlro_always);

    /* are we already installed? */
    if ((! _implementation->ignore_installed) &&
            (! _implementation->environment->installed_db()->query(p)->empty()))
        already_there = true;

    /* will we be installed by this point? */
    if ((! already_there) && (_implementation->merge_list.end() != std::find_if(
                _implementation->merge_list.begin(), _implementation->merge_list.end(),
                DepListEntryMatcher(*p))))
        return;

    if (already_there && ((! _implementation->recursive_deps) || (_implementation->check_existing_only)))
        return;

    /* are we allowed to install things? */
    if (_implementation->check_existing_only && ! _implementation->in_pdepend)
    {
        _implementation->match_found = false;
        return;
    }

    /* are we pending? (circular dep check) */
    {
        std::list<DepListEntry>::iterator i;
        if (_implementation->pending_list.end() != ((i = std::find_if(
                    _implementation->pending_list.begin(), _implementation->pending_list.end(),
                    DepListEntryMatcher(*p)))))
        {
            std::list<std::string> entries;
            entries.push_front(stringify(*p));
            std::transform(_implementation->pending_list.begin(), ++i,
                    std::back_inserter(entries), &stringify<DepListEntry>);
            if (_implementation->in_pdepend)
                return;
            else if (_implementation->drop_circular)
                return;
            else if (_implementation->drop_self_circular && entries.size() <= 2)
                return;
            else
                throw CircularDependencyError(entries.begin(), entries.end());
        }
    }

    /* are we allowed to install things? */
    if (_implementation->check_existing_only)
    {
        _implementation->match_found = false;
        return;
    }

    /* find the matching package */
    PackageDatabaseEntryCollection::Pointer matches(
            _implementation->environment->package_db()->query(p));

    const PackageDatabaseEntry * match(0);
    VersionMetadata::ConstPointer metadata(0);
    for (PackageDatabaseEntryCollection::ReverseIterator e(matches->rbegin()),
            e_end(matches->rend()) ; e != e_end ; ++e)
    {
        /* check masks */
        if (_implementation->environment->mask_reasons(*e).any())
            continue;

        metadata = _implementation->environment->package_db()->fetch_metadata(*e);
        match = &*e;
        break;
    }

    if (! match)
        throw AllMaskedError(stringify(*p));

    Save<const PackageDatabaseEntry *> old_current_package(&_implementation->current_package, match);

    context.change_context("When resolving package dependency '" + stringify(*p) +
            "' -> '" + stringify(*match) + "':");

    /* make merge entry */
    DepListEntry merge_entry(match->get<pde_package>(), match->get<pde_version>(),
            SlotName(metadata->get(vmk_slot)), match->get<pde_repository>());

    {
        /* pending */
        ContainerEntry<std::list<DepListEntry> > pending(&_implementation->pending_list, merge_entry);

        /* merge dependencies */
        _add_in_role(DepParser::parse(metadata->get(vmk_depend)), "DEPEND");

        if (_implementation->rdepend_post != dlro_always)
        {
            try
            {
                 _add_in_role(DepParser::parse(metadata->get(vmk_rdepend)), "RDEPEND");
            }
            catch (const CircularDependencyError &)
            {
                if (_implementation->rdepend_post != dlro_never)
                    do_rdepend_post = true;
                else
                    throw;
            }
        }
    }

    /* merge package */
    if (! already_there)
        _implementation->merge_list.push_back(merge_entry);

    /* merge post dependencies */
    {
        Save<bool> save_ignore_cdep(&_implementation->in_pdepend, true);
        if (do_rdepend_post)
            _add_in_role(DepParser::parse(metadata->get(vmk_rdepend)), "RDEPEND (as PDEPEND)");
        _add_in_role(DepParser::parse(metadata->get(vmk_pdepend)), "PDEPEND");
    }
}

void
DepList::visit(const UseDepAtom * const u)
{
    if (! _implementation->current_package)
        throw InternalError(PALUDIS_HERE, "current_package is 0");

    if (_implementation->environment->query_use(u->flag(),
                *_implementation->current_package) ^ u->inverse())
        std::for_each(u->begin(), u->end(), std::bind1st(std::mem_fun(&DepList::add), this));
}

struct IsViable :
    public std::unary_function<bool, DepAtom::ConstPointer>
{
    const Implementation<DepList> & _impl;

    IsViable(const Implementation<DepList> & impl) :
        _impl(impl)
    {
    }

    bool operator() (DepAtom::ConstPointer a)
    {
        /// \todo don't use dynamic_cast<>, it sucks
        const UseDepAtom * u(0);
        if (0 != ((u = dynamic_cast<const UseDepAtom *>(a.raw_pointer()))))
            return _impl.environment->query_use(u->flag(),
                        *_impl.current_package) ^ u->inverse();
        else
            return true;
    }
};

void
DepList::visit(const AnyDepAtom * const a)
{
    /* try to resolve each of our children in return. note the annoying
     * special case for use? () flags:
     *
     *   || ( ) -> nothing
     *   || ( off1? ( blah1 ) off2? ( blah2 ) blah3 ) -> blah3
     *   || ( off1? ( blah1 ) off2? ( blah2 ) ) -> nothing
     *   || ( ( off1? ( blah1 ) ) blah2 ) -> nothing
     *
     * we handle this by keeping a list of 'viable children'.
     */

    std::list<DepAtom::ConstPointer> viable_children;
    if (0 == _implementation->current_package)
        throw InternalError(PALUDIS_HERE, "current_package is 0");
    std::copy(a->begin(), a->end(), filter_inserter(
                std::back_inserter(viable_children), IsViable(*_implementation)));

    if (viable_children.empty())
        return;

    bool found(false);
    for (CompositeDepAtom::Iterator i(viable_children.begin()),
            i_end(viable_children.end()) ; i != i_end ; ++i)
    {
        Save<bool> save_check(&_implementation->check_existing_only, true);
        Save<bool> save_match(&_implementation->match_found, true);
        add(*i);
        if ((found = _implementation->match_found))
            break;
    }
    if (found)
        return;

    if (_implementation->check_existing_only)
    {
        _implementation->match_found = false;
        return;
    }

    /* try to merge each of our viable children in turn. */
    for (CompositeDepAtom::Iterator i(viable_children.begin()), i_end(viable_children.end()) ;
            i != i_end ; ++i)
    {
        try
        {
            add(*i);
            return;
        }
        catch (const DepListStackTooDeepError &)
        {
            /* don't work around a stack too deep error. our item may be
             * resolvable with a deeper stack. */
            throw;
        }
        catch (const DepListError &)
        {
        }
    }

    /* no match */
    throw NoResolvableOptionError();
}

void
DepList::visit(const BlockDepAtom * const d)
{
    Context context("When checking block '!" + stringify(*(d->blocked_atom())) + "':");

    /* special case: the provider of virtual/blah can DEPEND upon !virtual/blah. */
    /// \bug This may have issues if a virtual is provided by a virtual...

    PackageDatabaseEntryCollection::ConstPointer q(0);
    std::list<DepListEntry>::const_iterator m;

    /* are we already installed? */
    if (! ((q = _implementation->environment->installed_db()->query(d->blocked_atom())))->empty())
    {
        if (! _implementation->current_package)
            throw BlockError("'" + stringify(*(d->blocked_atom())) + "' blocked by installed package '"
                    + stringify(*q->begin()) + "'");

        VersionMetadata::ConstPointer metadata(
                _implementation->environment->installed_db()->fetch_metadata(
                    *_implementation->current_package));
        if (metadata->end_provide() == std::find(
                    metadata->begin_provide(), metadata->end_provide(),
                    d->blocked_atom()->package()))
            throw BlockError("'" + stringify(*(d->blocked_atom())) + "' blocked by installed package '"
                    + stringify(*q->begin()) + "'");
    }

    /* will we be installed by this point? */
    if (_implementation->merge_list.end() != ((m = std::find_if(
                _implementation->merge_list.begin(), _implementation->merge_list.end(),
                DepListEntryMatcher(*(d->blocked_atom()))))))
    {
        if (! _implementation->current_package)
            throw BlockError("'" + stringify(*(d->blocked_atom())) + "' blocked by pending package '"
                    + stringify(*m));

        VersionMetadata::ConstPointer metadata(_implementation->environment->package_db()->fetch_metadata(
                    *_implementation->current_package));
        if (metadata->end_provide() == std::find(
                    metadata->begin_provide(), metadata->end_provide(),
                    d->blocked_atom()->package()))
            throw BlockError("'" + stringify(*(d->blocked_atom())) + "' blocked by pending package '"
                    + stringify(*m) + "' ~ " + join(metadata->begin_provide(), metadata->end_provide(), " ~ "));
    }
}

void
DepList::set_rdepend_post(const DepListRdependOption value)
{
    _implementation->rdepend_post = value;
}

void
DepList::set_drop_circular(const bool value)
{
    _implementation->drop_circular = value;
}

void
DepList::set_drop_self_circular(const bool value)
{
    _implementation->drop_self_circular = value;
}

void
DepList::set_ignore_installed(const bool value)
{
    _implementation->ignore_installed = value;
}

void
DepList::set_recursive_deps(const bool value)
{
    _implementation->recursive_deps = value;
}

void
DepList::set_max_stack_depth(const int value)
{
    _implementation->max_stack_depth = value;
}

