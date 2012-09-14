dnl vim: set ft=m4 et :
dnl This file is used by Makefile.am.m4 and paludis.hh.m4. You should
dnl use the provided autogen.bash script to do all the hard work.
dnl
dnl This file is used to avoid having to make lots of repetitive changes in
dnl Makefile.am every time we add a source or test file. The first parameter is
dnl the base filename with no extension; later parameters can be `hh', `cc',
dnl `gtest', `impl', `testscript'. Note that there isn't much error checking done
dnl on this file at present...

add(`about',                                       `hh', `gtest')
add(`about_metadata',                              `hh', `cc', `fwd')
add(`action',                                      `hh', `cc', `fwd', `se')
add(`action_names',                                `hh', `cc', `fwd')
add(`additional_package_dep_spec_requirement',     `hh', `cc', `fwd')
add(`always_enabled_dependency_label',             `hh', `cc', `fwd')
add(`broken_linkage_configuration',                `hh', `cc', `gtest', `testscript')
add(`broken_linkage_finder',                       `hh', `cc')
add(`buffer_output_manager',                       `hh', `cc', `fwd')
add(`call_pretty_printer',                         `hh', `cc', `fwd')
add(`changed_choices',                             `hh', `cc', `fwd')
add(`choice',                                      `hh', `cc', `se', `fwd')
add(`comma_separated_dep_parser',                  `hh', `cc', `gtest')
add(`comma_separated_dep_pretty_printer',          `hh', `cc', `fwd')
add(`command_output_manager',                      `hh', `cc', `fwd')
add(`common_sets',                                 `hh', `cc', `fwd')
add(`contents',                                    `hh', `cc', `fwd')
add(`create_output_manager_info',                  `hh', `cc', `fwd', `se')
add(`dep_label',                                   `hh', `cc', `fwd')
add(`dep_spec',                                    `hh', `cc', `gtest', `fwd')
add(`dep_spec_annotations',                        `hh', `cc', `fwd', `se')
add(`dep_spec_data',                               `hh', `cc', `fwd')
add(`dep_spec_flattener',                          `hh', `cc')
add(`distribution',                                `hh', `cc', `impl', `fwd')
add(`elf_linkage_checker',                         `hh', `cc')
add(`elike_blocker',                               `hh', `cc', `fwd', `se')
add(`elike_choices',                               `hh', `cc', `fwd', `se')
add(`elike_dep_parser',                            `hh', `cc', `fwd', `gtest', `se')
add(`elike_conditional_dep_spec',                  `hh', `cc', `fwd')
add(`elike_package_dep_spec',                      `hh', `cc', `fwd', `se')
add(`elike_slot_requirement',                      `hh', `cc', `fwd')
add(`elike_use_requirement',                       `hh', `cc', `fwd', `se', `gtest')
add(`environment',                                 `hh', `fwd', `cc')
add(`environment_factory',                         `hh', `fwd', `cc')
add(`environment_implementation',                  `hh', `cc', `gtest')
add(`file_output_manager',                         `hh', `cc', `fwd')
add(`filter',                                      `hh', `cc', `fwd', `gtest')
add(`filter_handler',                              `hh', `cc', `fwd')
add(`filtered_generator',                          `hh', `cc', `fwd', `gtest')
add(`format_messages_output_manager',              `hh', `fwd', `cc')
add(`formatted_pretty_printer',                    `hh', `fwd', `cc')
add(`forward_at_finish_output_manager',            `hh', `fwd', `cc')
add(`fs_merger',                                   `hh', `cc', `fwd', `se', `gtest', `testscript')
add(`fuzzy_finder',                                `hh', `cc', `gtest')
add(`generator',                                   `hh', `cc', `fwd', `gtest')
add(`generator_handler',                           `hh', `cc', `fwd')
add(`hook',                                        `hh', `cc', `fwd', `se')
add(`hooker',                                      `hh', `cc', `gtest', `testscript')
add(`ipc_output_manager',                          `hh', `cc', `fwd')
add(`libtool_linkage_checker',                     `hh', `cc')
add(`linkage_checker',                             `hh', `cc')
add(`literal_metadata_key',                        `hh', `cc')
add(`maintainer',                                  `hh', `cc', `fwd')
add(`mask',                                        `hh', `cc', `fwd', `se')
add(`mask_utils',                                  `hh', `cc', `fwd')
add(`match_package',                               `hh', `cc', `se', `fwd')
add(`merger',                                      `hh', `cc', `se', `fwd')
add(`merger_entry_type',                           `hh', `cc', `se')
add(`metadata_key',                                `hh', `cc', `se', `fwd')
add(`metadata_key_holder',                         `hh', `cc', `fwd')
add(`name',                                        `hh', `cc', `fwd', `gtest')
add(`ndbam',                                       `hh', `cc', `fwd')
add(`ndbam_merger',                                `hh', `cc')
add(`ndbam_unmerger',                              `hh', `cc')
add(`notifier_callback',                           `hh', `cc', `fwd')
add(`output_manager',                              `hh', `fwd', `cc', `se')
add(`output_manager_factory',                      `hh', `fwd', `cc')
add(`output_manager_from_environment',             `hh', `fwd', `cc')
add(`package_dep_spec_collection',                 `hh', `cc', `fwd')
add(`package_dep_spec_properties',                 `hh', `cc', `fwd')
add(`package_id',                                  `hh', `cc', `fwd', `se')
add(`paludis',                                     `hh')
add(`paludislike_options_conf',                    `hh', `cc', `fwd')
add(`partially_made_package_dep_spec',             `hh', `cc', `fwd', `se')
add(`permitted_choice_value_parameter_values',     `hh', `cc', `fwd')
add(`pretty_print_options',                        `hh', `cc', `fwd', `se')
add(`pretty_printer',                              `hh', `cc', `fwd')
add(`repository',                                  `hh', `fwd', `cc', `se')
add(`repository_factory',                          `hh', `fwd', `cc')
add(`repository_name_cache',                       `hh', `cc', `gtest', `testscript')
add(`selection',                                   `hh', `cc', `fwd', `gtest')
add(`selection_handler',                           `hh', `cc', `fwd')
add(`serialise',                                   `hh', `cc', `fwd', `impl')
add(`set_file',                                    `hh', `cc', `se', `gtest', `testscript')
add(`slot',                                        `hh', `fwd', `cc')
add(`slot_requirement',                            `hh', `fwd', `cc')
add(`spec_tree',                                   `hh', `fwd', `cc')
add(`standard_output_manager',                     `hh', `cc', `fwd')
add(`stripper',                                    `hh', `cc', `fwd', `gtest', `testscript')
add(`syncer',                                      `hh', `cc')
add(`tar_merger',                                  `hh', `cc', `fwd', `gtest', `testscript', `se')
add(`tee_output_manager',                          `hh', `cc', `fwd')
add(`unchoices_key',                               `hh', `cc', `fwd')
add(`unformatted_pretty_printer',                  `hh', `cc', `fwd')
add(`unmerger',                                    `hh', `cc')
add(`user_dep_spec',                               `hh', `cc', `se', `fwd', `gtest')
add(`version_operator',                            `hh', `cc', `fwd', `se', `gtest')
add(`version_requirements',                        `hh', `cc', `fwd')
add(`version_spec',                                `hh', `cc', `se', `fwd', `gtest')

