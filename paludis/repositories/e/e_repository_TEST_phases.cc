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

#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/e_repository_exceptions.hh>
#include <paludis/repositories/e/e_repository_id.hh>
#include <paludis/repositories/e/vdb_repository.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/dep_spec_pretty_printer.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/system.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/map.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/set.hh>
#include <paludis/output_manager.hh>
#include <paludis/standard_output_manager.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/stringify_formatter.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/repository_factory.hh>
#include <paludis/choice.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <tr1/functional>
#include <set>
#include <string>

#include "config.h"

using namespace test;
using namespace paludis;

namespace
{
    void cannot_uninstall(const std::tr1::shared_ptr<const PackageID> & id, const UninstallActionOptions &)
    {
        if (id)
            throw InternalError(PALUDIS_HERE, "cannot uninstall");
    }

    std::tr1::shared_ptr<OutputManager> make_standard_output_manager(const Action &)
    {
        return make_shared_ptr(new StandardOutputManager);
    }

    std::string from_keys(const std::tr1::shared_ptr<const Map<std::string, std::string> > & m,
            const std::string & k)
    {
        Map<std::string, std::string>::ConstIterator mm(m->find(k));
        if (m->end() == mm)
            return "";
        else
            return mm->second;
    }

    WantPhase want_all_phases(const std::string &)
    {
        return wp_yes;
    }

    struct PhasesTest : TestCase
    {
        const std::string test;
        const bool expect_pass;
        const bool expect_expensive_test;

        PhasesTest(const std::string & s, const bool p, const bool t) :
            TestCase("ever " + s),
            test(s),
            expect_pass(p),
            expect_expensive_test(t)
        {
        }

        unsigned max_run_time() const
        {
            return 3000;
        }

        bool repeatable() const
        {
            return false;
        }

        virtual void extra_settings(TestEnvironment &)
        {
        }

        void run()
        {
#ifdef ENABLE_VIRTUALS_REPOSITORY
            ::setenv("PALUDIS_ENABLE_VIRTUALS_REPOSITORY", "yes", 1);
#else
            ::setenv("PALUDIS_ENABLE_VIRTUALS_REPOSITORY", "", 1);
#endif
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_phases_dir" / "repo1"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_phases_dir" / "repo1/profiles/profile"));
            keys->insert("layout", "exheres");
            keys->insert("eapi_when_unknown", "exheres-0");
            keys->insert("eapi_when_unspecified", "exheres-0");
            keys->insert("profile_eapi", "exheres-0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "e_repository_TEST_phases_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_phases_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::tr1::shared_ptr<FakeInstalledRepository> installed_repo(new FakeInstalledRepository(&env, RepositoryName("installed")));
            installed_repo->add_version("cat", "pretend-installed", "0")->provide_key()->set_from_string("virtual/virtual-pretend-installed");
            installed_repo->add_version("cat", "pretend-installed", "1")->provide_key()->set_from_string("virtual/virtual-pretend-installed");
            env.package_database()->add_repository(2, installed_repo);

#ifdef ENABLE_VIRTUALS_REPOSITORY
            std::tr1::shared_ptr<Map<std::string, std::string> > iv_keys(new Map<std::string, std::string>);
            iv_keys->insert("root", "/");
            iv_keys->insert("format", "installed_virtuals");
            env.package_database()->add_repository(-2, RepositoryFactory::get_instance()->create(&env,
                        std::tr1::bind(from_keys, iv_keys, std::tr1::placeholders::_1)));
            std::tr1::shared_ptr<Map<std::string, std::string> > v_keys(new Map<std::string, std::string>);
            v_keys->insert("format", "virtuals");
            env.package_database()->add_repository(-2, RepositoryFactory::get_instance()->create(&env,
                        std::tr1::bind(from_keys, v_keys, std::tr1::placeholders::_1)));
#endif

            extra_settings(env);

            InstallAction action(make_named_values<InstallActionOptions>(
                        value_for<n::destination>(installed_repo),
                        value_for<n::make_output_manager>(&make_standard_output_manager),
                        value_for<n::perform_uninstall>(&cannot_uninstall),
                        value_for<n::replacing>(make_shared_ptr(new PackageIDSequence)),
                        value_for<n::want_phase>(&want_all_phases)
                    ));

            const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("cat/" + test,
                                    &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
            TEST_CHECK(id);
            TEST_CHECK_EQUAL(!! id->choices_key()->value()->find_by_name_with_prefix(
                        ChoiceNameWithPrefix("build_options:expensive_tests")), expect_expensive_test);

            if (expect_pass)
                id->perform_action(action);
            else
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
        }
    };
}

namespace test_cases
{
    PhasesTest test_no("no-expensive-test", true, false);
    PhasesTest test_pass("expensive-test", true, true);
    PhasesTest test_fail("expensive-test-fail", true, true);

    struct TestFailEnabled : PhasesTest
    {
        TestFailEnabled() :
            PhasesTest("expensive-test-fail", false, true)
        {
        }

        void extra_settings(TestEnvironment & env)
        {
            env.set_want_choice_enabled(ChoicePrefixName("build_options"), UnprefixedChoiceName("expensive_tests"), true);
        }
    } test_fail_enabled;
}

