#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

#
# Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
#
# This file is part of the Paludis package manager. Paludis is free software;
# you can redistribute it and/or modify it under the terms of the GNU General
# Public License version 2, as published by the Free Software Foundation.
#
# Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA  02111-1307  USA
#

require 'test/unit'
require 'Paludis'

Paludis::Log.instance.log_level = Paludis::LogLevel::Warning

module Paludis
    class TestCase_VersionMetadata < Test::Unit::TestCase
        def test_no_create
            assert_raise NoMethodError do
                p = VersionMetadata.new
            end
        end

        def env
            unless @env
                @env = NoConfigEnvironment.new("version_metadata_TEST_dir/testrepo/")
            end
            @env
        end

        def env_vdb
            unless @env_vdb
                @env_vdb = NoConfigEnvironment.new("version_metadata_TEST_dir/installed/")
            end
            @env_vdb
        end

        def vmd version
            env.package_database.fetch_repository("testrepo").version_metadata("foo/bar", version)
        end

        def vmd_vdb package, version
            env_vdb.package_database.fetch_repository("installed").version_metadata(package, version)
        end

        def test_license
            assert_kind_of DepSpec, vmd("1.0").license
        end

        def test_interfaces
            assert vmd("1.0").ebuild_interface
            assert ! vmd("1.0").cran_interface
            assert ! vmd("1.0").virtual_interface
        end

        def test_members
            assert_equal "Test package", vmd("1.0").description
#            assert_equal "http://paludis.pioto.org/", vmd("1.0").homepage
            assert_kind_of DepSpec, vmd('1.0').homepage
            assert_equal "0", vmd("1.0").slot
#            assert_equal "0", vmd("1.0").eapi
#            assert_equal "GPL-2", vmd("1.0").license_string
            assert_kind_of DepSpec, vmd('1.0').license
            assert !vmd('1.0').interactive?
        end

        def test_ebuild_members
#            assert_equal "", vmd("1.0").provide_string
            assert_kind_of DepSpec, vmd('1.0').provide
#            assert_equal "http://example.com/bar-1.0.tar.bz2", vmd("1.0").src_uri_string
            assert_kind_of DepSpec, vmd('1.0').src_uri
#            assert_equal "monkey", vmd("1.0").restrict_string
            assert_kind_of DepSpec, vmd('1.0').restrictions
#            assert_equal "test", vmd("1.0").keywords_string.gsub(%r/\s/, "")
            assert_kind_of Array, vmd('1.0').keywords
            assert_equal 1, vmd('1.0').keywords.length
            assert_equal 'test', vmd('1.0').keywords.first
#            assert_equal "", vmd("1.0").iuse.gsub(%r/\s/, "")
            assert_kind_of Array, vmd('1.0').iuse
            assert vmd('1.0').iuse.empty?
        end

        def test_deps
            assert_kind_of AllDepSpec, vmd("1.0").build_depend
            assert_kind_of AllDepSpec, vmd("1.0").run_depend
            assert_kind_of AllDepSpec, vmd("1.0").suggested_depend
            assert_kind_of AllDepSpec, vmd("1.0").post_depend

            assert_equal 1, vmd("1.0").build_depend.to_a.length
            assert vmd("1.0").run_depend.to_a.empty?
            assert vmd("1.0").suggested_depend.to_a.empty?
            assert vmd("1.0").post_depend.to_a.empty?
        end

        def test_origin_source
            assert_equal PackageDatabaseEntry.new('cat-one/pkg-one', '1', 'origin_test'),
                vmd_vdb('cat-one/pkg-one','1').origin_source
            assert_nil vmd_vdb('cat-one/pkg-two','1').origin_source
        end
    end
end


