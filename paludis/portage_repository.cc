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

#include "portage_repository.hh"
#include "dir_iterator.hh"
#include "stringify.hh"
#include "internal_error.hh"
#include "fs_entry.hh"
#include "strip.hh"
#include "is_file_with_extension.hh"
#include "filter_insert_iterator.hh"
#include "translate_insert_iterator.hh"
#include "line_config_file.hh"
#include "key_value_config_file.hh"
#include "tokeniser.hh"
#include "indirect_iterator.hh"
#include "package_dep_atom.hh"
#include <map>
#include <fstream>
#include <functional>
#include <algorithm>
#include <vector>

using namespace paludis;

namespace paludis
{
    /**
     * Implementation data for a PortageRepository.
     */
    template <>
    struct Implementation<PortageRepository> :
        InternalCounted<Implementation<PortageRepository> >
    {
        /// Our base location.
        FSEntry location;

        /// Our profile.
        FSEntry profile;

        /// Have we loaded our category names?
        mutable bool has_category_names;

        /// Our category names, and whether we have a fully loaded list
        /// of package names for that category.
        mutable std::map<CategoryNamePart, bool> category_names;

        /// Our package names, and whether we have a fully loaded list of
        /// version specs for that category.
        mutable std::map<QualifiedPackageName, bool> package_names;

        /// Our version specs for each package.
        mutable std::map<QualifiedPackageName, VersionSpecCollection::Pointer> version_specs;

        /// Metadata cache.
        mutable std::map<std::pair<QualifiedPackageName, VersionSpec>, VersionMetadata::Pointer> metadata;

        /// Repository mask.
        mutable std::map<QualifiedPackageName, std::vector<PackageDepAtom::ConstPointer> > repo_mask;

        /// Have repository mask?
        mutable bool has_repo_mask;

        /// Use mask.
        mutable std::set<UseFlagName> use_mask;

        /// Use.
        mutable std::map<UseFlagName, UseFlagState> use;

        /// Have we loaded our profile yet?
        mutable bool has_profile;

        /// Constructor.
        Implementation(const FSEntry & l, const FSEntry & p) :
            location(l),
            profile(p),
            has_category_names(false),
            has_repo_mask(false)
        {
        }

        /// Add a use.mask, use from a profile directory, recursive.
        void add_profile(const FSEntry & f) const;
    };
}

void
Implementation<PortageRepository>::add_profile(const FSEntry & f) const
{
    Context context("When reading profile directory '" + stringify(f) + "':");

    if (! f.is_directory())
        throw InternalError(PALUDIS_HERE, "todo"); /// \bug exception

    if ((f / "parent").exists())
    {
        std::ifstream parent_file(stringify(f / "parent").c_str());
        if (! parent_file)
            throw InternalError(PALUDIS_HERE, "todo"); /// \bug exception
        LineConfigFile parent(&parent_file);
        if (parent.begin() != parent.end())
            add_profile((f / *parent.begin()).realpath());
        else
            throw InternalError(PALUDIS_HERE, "todo"); /// \bug exception
    }

    if ((f / "make.defaults").exists())
    {
        static Tokeniser<delim_kind::AnyOfTag, delim_mode::DelimiterTag> tokeniser(" \t\n");

        std::ifstream make_defaults_file(stringify(f / "make.defaults").c_str());
        if (! make_defaults_file)
            throw InternalError(PALUDIS_HERE, "todo"); /// \bug exception
        KeyValueConfigFile make_defaults_f(&make_defaults_file);
        std::vector<std::string> uses;
        tokeniser.tokenise(make_defaults_f.get("USE"), std::back_inserter(uses));
        for (std::vector<std::string>::const_iterator u(uses.begin()), u_end(uses.end()) ;
                u != u_end ; ++u)
        {
            if ('-' == u->at(0))
                use[UseFlagName(u->substr(1))] = use_disabled;
            else
                use[UseFlagName(*u)] = use_enabled;
        }
    }

    if ((f / "use.mask").exists())
    {
        std::ifstream use_mask_file(stringify(f / "use.mask").c_str());
        if (! use_mask_file)
            throw InternalError(PALUDIS_HERE, "todo"); /// \bug exception
        LineConfigFile use_mask_f(&use_mask_file);
        for (LineConfigFile::Iterator line(use_mask_f.begin()), line_end(use_mask_f.end()) ;
                line != line_end ; ++line)
            if ('-' == line->at(0))
                use_mask.erase(UseFlagName(line->substr(1)));
            else
                use_mask.insert(UseFlagName(*line));
    }
}

PortageRepository::PortageRepository(const FSEntry & location, const FSEntry & profile) :
    Repository(PortageRepository::fetch_repo_name(location)),
    PrivateImplementationPattern<PortageRepository>(new Implementation<PortageRepository>(location, profile))
{
    _info.insert(std::make_pair("location", location));
    _info.insert(std::make_pair("profile", profile));
    _info.insert(std::make_pair("format", "portage"));
}

PortageRepository::~PortageRepository()
{
}

bool
PortageRepository::do_has_category_named(const CategoryNamePart & c) const
{
    Context context("When checking for category '" + stringify(c) +
            "' in " + stringify(name()) + ":");

    need_category_names();
    return _implementation->category_names.end() !=
        _implementation->category_names.find(c);
}

bool
PortageRepository::do_has_package_named(const CategoryNamePart & c,
        const PackageNamePart & p) const
{
    Context context("When checking for package '" + stringify(c) + "/"
            + stringify(p) + "' in " + stringify(name()) + ":");

    need_category_names();

    std::map<CategoryNamePart, bool>::const_iterator cat_iter(
            _implementation->category_names.find(c));

    if (_implementation->category_names.end() == cat_iter)
        return false;

    /// \todo
    if (c == CategoryNamePart("virtual"))
        return true;

    const QualifiedPackageName n(c, p);

    if (cat_iter->second)
        return _implementation->package_names.find(n) !=
            _implementation->package_names.end();
    else
    {
        if (_implementation->package_names.find(n) !=
                _implementation->package_names.end())
            return true;

        FSEntry fs(_implementation->location);
        fs /= stringify(c);
        fs /= stringify(p);
        if (! fs.is_directory())
            return false;
        _implementation->package_names.insert(std::make_pair(n, false));
        return true;
    }
}

CategoryNamePartCollection::ConstPointer
PortageRepository::do_category_names() const
{
    Context context("When fetching category names in " + stringify(name()) + ":");

    need_category_names();

    CategoryNamePartCollection::Pointer result(new CategoryNamePartCollection);
    std::map<CategoryNamePart, bool>::const_iterator i(_implementation->category_names.begin()),
        i_end(_implementation->category_names.end());
    for ( ; i != i_end ; ++i)
        result->insert(i->first);
    return result;
}

QualifiedPackageNameCollection::ConstPointer
PortageRepository::do_package_names(const CategoryNamePart & c) const
{
    Context context("When fetching package names in category '" + stringify(c)
            + "' in " + stringify(name()) + ":");

    need_category_names();
    /// \todo
    throw InternalError(PALUDIS_HERE, "not implemented");
    return QualifiedPackageNameCollection::Pointer(new QualifiedPackageNameCollection);
}

VersionSpecCollection::ConstPointer
PortageRepository::do_version_specs(const QualifiedPackageName & n) const
{
    Context context("When fetching versions of '" + stringify(n) + "' in "
            + stringify(name()) + ":");

    if (has_package_named(n))
    {
        need_version_names(n);
        return _implementation->version_specs.find(n)->second;
    }
    else
        return VersionSpecCollection::Pointer(new VersionSpecCollection);
}

bool
PortageRepository::do_has_version(const CategoryNamePart & c,
        const PackageNamePart & p, const VersionSpec & v) const
{
    Context context("When checking for version '" + stringify(v) + "' in '"
            + stringify(c) + "/" + stringify(p) + "' in " + stringify(name()) + ":");

    if (has_package_named(c, p))
    {
        need_version_names(QualifiedPackageName(c, p));
        VersionSpecCollection::Pointer vv(
                _implementation->version_specs.find(QualifiedPackageName(c, p))->second);
        return vv->end() != vv->find(v);
    }
    else
        return false;
}

void
PortageRepository::need_category_names() const
{
    if (_implementation->has_category_names)
        return;

    Context context("When loading category names for " + stringify(name()) + ":");

    std::ifstream cat_file(stringify(_implementation->location /
                "profiles/categories").c_str());

    if (! cat_file)
        throw InternalError(PALUDIS_HERE, "todo"); /// \bug real exception needed

    LineConfigFile cats(&cat_file);

    for (LineConfigFile::Iterator line(cats.begin()), line_end(cats.end()) ;
            line != line_end ; ++line)
        _implementation->category_names.insert(std::make_pair(CategoryNamePart(*line), false));

    _implementation->has_category_names = true;
}

void
PortageRepository::need_version_names(const QualifiedPackageName & n) const
{
    if (_implementation->package_names[n])
        return;

    Context context("When loading versions for '" + stringify(n) + "' in "
            + stringify(name()) + ":");

    VersionSpecCollection::Pointer v(new VersionSpecCollection);

    FSEntry path(_implementation->location / stringify(n.get<qpn_category>()) /
            stringify(n.get<qpn_package>()));
    if (CategoryNamePart("virtual") == n.get<qpn_category>() && ! path.exists())
    {
        /// \todo
        v->insert(VersionSpec("0"));
    }
    else
        std::copy(DirIterator(path), DirIterator(),
                filter_inserter(
                    translate_inserter(
                        translate_inserter(
                            translate_inserter(v->inserter(),
                                StripTrailingString(".ebuild")),
                            StripLeadingString(stringify(n.get<qpn_package>()) + "-")),
                        std::mem_fun_ref(&FSEntry::basename)),
                    IsFileWithExtension(stringify(n.get<qpn_package>()) + "-", ".ebuild")));


    _implementation->version_specs.insert(std::make_pair(n, v));
    _implementation->package_names[n] = true;
}

RepositoryName
PortageRepository::fetch_repo_name(const std::string & location)
{
    try
    {
        do
        {
            FSEntry name_file(location);
            name_file /= "profiles";
            name_file /= "repo_name";

            if (! name_file.is_regular_file())
                break;

            std::ifstream file(std::string(name_file).c_str());
            LineConfigFile f(&file);
            if (f.begin() == f.end())
                break;
            return RepositoryName(*f.begin());

        } while (false);
    }
    catch (...)
    {
    }
    return RepositoryName("x-" + location);
}

VersionMetadata::ConstPointer
PortageRepository::do_version_metadata(
        const CategoryNamePart & c, const PackageNamePart & p, const VersionSpec & v) const
{
    if (_implementation->metadata.end() != _implementation->metadata.find(
                std::make_pair(QualifiedPackageName(c, p), v)))
            return _implementation->metadata.find(std::make_pair(QualifiedPackageName(c, p), v))->second;

    VersionMetadata::Pointer result(new VersionMetadata);

    FSEntry cache_file(_implementation->location);
    cache_file /= "metadata";
    cache_file /= "cache";
    cache_file /= stringify(c);
    cache_file /= stringify(p) + "-" + stringify(v);

    if (cache_file.is_regular_file())
    {
        std::ifstream cache(std::string(cache_file).c_str());
        std::string line;

        if (! cache)
            throw InternalError(PALUDIS_HERE, "todo");

        /// \bug this lot
        std::getline(cache, line); result->set(vmk_depend,      line);
        std::getline(cache, line); result->set(vmk_rdepend,     line);
        std::getline(cache, line); result->set(vmk_slot,        line);
        std::getline(cache, line); result->set(vmk_src_uri,     line);
        std::getline(cache, line); result->set(vmk_restrict,    line);
        std::getline(cache, line); result->set(vmk_homepage,    line);
        std::getline(cache, line); result->set(vmk_license,     line);
        std::getline(cache, line); result->set(vmk_description, line);
        std::getline(cache, line); result->set(vmk_keywords,    line);
        std::getline(cache, line); result->set(vmk_inherited,   line);
        std::getline(cache, line); result->set(vmk_iuse,        line);
        std::getline(cache, line);
        std::getline(cache, line); result->set(vmk_pdepend,     line);
        std::getline(cache, line); result->set(vmk_provide,     line);
        std::getline(cache, line); result->set(vmk_eapi,        line);
    }
    else if (CategoryNamePart("virtual") == c)
    {
        /// \todo
        result->set(vmk_slot, "0");
        result->set(vmk_keywords, "*");
    }
    else
        throw InternalError(PALUDIS_HERE, "not implemented"); /// \todo

    _implementation->metadata.insert(std::make_pair(std::make_pair(QualifiedPackageName(c, p), v), result));
    return result;
}

bool
PortageRepository::do_query_repository_masks(const CategoryNamePart & c,
        const PackageNamePart & p, const VersionSpec & v) const
{
    if (! _implementation->has_repo_mask)
    {
        Context context("When querying repository mask for '" + stringify(c) + "/" + stringify(p) + "-"
                + stringify(v) + "':");

        std::ifstream f(stringify(_implementation->location / "profiles" / "package.mask").c_str());
        if (! f)
            throw InternalError(PALUDIS_HERE, "todo"); /// \bug exception
        LineConfigFile ff(&f);
        for (LineConfigFile::Iterator line(ff.begin()), line_end(ff.end()) ;
                line != line_end ; ++line)
        {
            PackageDepAtom::ConstPointer a(new PackageDepAtom(*line));
            _implementation->repo_mask[a->package()].push_back(a);
        }

        _implementation->has_repo_mask = true;
    }

    std::map<QualifiedPackageName, std::vector<PackageDepAtom::ConstPointer> >::const_iterator r(
            _implementation->repo_mask.find(QualifiedPackageName(c, p)));
    if (_implementation->repo_mask.end() == r)
        return false;
    else
    {
        for (IndirectIterator<std::vector<PackageDepAtom::ConstPointer>::const_iterator, const PackageDepAtom>
                k(r->second.begin()), k_end(r->second.end()) ; k != k_end ; ++k)
        {
            if (k->package() != QualifiedPackageName(c, p))
                continue;
            if (k->version_spec_ptr() && ! ((v.*
                            (k->version_operator().as_version_spec_operator()))
                        (*k->version_spec_ptr())))
                continue;
            /// \bug slot

            return true;
        }
    }

    return false;
}

bool
PortageRepository::do_query_profile_masks(const CategoryNamePart &,
        const PackageNamePart &, const VersionSpec &) const
{
    /// \todo
    return false;
}

UseFlagState
PortageRepository::do_query_use(const UseFlagName & f) const
{
    if (! _implementation->has_profile)
    {
        Context context("When checking USE state for '" + stringify(f) + "':");
        _implementation->add_profile(_implementation->profile.realpath());
        _implementation->has_profile = true;
    }

    std::map<UseFlagName, UseFlagState>::const_iterator p;
    if (_implementation->use.end() == ((p = _implementation->use.find(f))))
        return use_unspecified;
    else
        return p->second;
}

bool
PortageRepository::do_query_use_mask(const UseFlagName & u) const
{
    if (! _implementation->has_profile)
    {
        Context context("When checking USE mask for '" + stringify(u) + "':");
        _implementation->add_profile(_implementation->profile.realpath());
        _implementation->has_profile = true;
    }

    return _implementation->use_mask.end() != _implementation->use_mask.find(u);
}

