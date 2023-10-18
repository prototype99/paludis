#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2009, 2010, 2011, 2012, 2021, 2023 Ali Polatel <alip@chesswob.org>
#
# Based in part upon ebuild.sh from Portage, which is Copyright 1995-2005
# Gentoo Foundation and distributed under the terms of the GNU General
# Public License v2.
#
# This file is part of the Paludis package manager. Paludis is free software;
# you can redistribute it and/or modify it under the terms of the GNU General
# Public License, version 2, as published by the Free Software Foundation.
#
# Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA  02111-1307  USA

sydbox_internal_api()
{
    if [[ -e /dev/sydbox/1 ]]; then
        echo -n 1
    else
        # FIXME: This is not ideal but otherwise some builds
        # fail. Since "esandbox check" is always called before
        # "sydbox_internal_api", this is safe until API 4
        # happens and there is a good chance API 4 will never
        # happen.
        echo -n 3
    fi
}

sydbox_internal_path_1()
{
    local cmd="${1}"
    local op="${2}"

    case "${op}" in
    '+'|'-')
        ;;
    *)
        die "${FUNCNAME}: invalid operation character '${op}'"
        ;;
    esac

    shift 2

    local path
    for path in "${@}"; do
        [[ "${path:0:1}" == '/' ]] || die "${FUNCNAME} expects absolute path, got: ${path}"
        [[ -e /dev/sydbox/"${cmd}${op}${path}" ]]
    done
}

sydbox_internal_path_3()
{
    local cmd="${1}"
    local op="${2}"

    case "${op}" in
    '+'|'-')
        ;;
    *)
        die "${FUNCNAME}: invalid operation character '${op}'"
        ;;
    esac

    shift 2

    local path
    for path in "${@}"; do
        [[ "${path:0:1}" == '/' ]] || die "${FUNCNAME} expects absolute path, got: ${path}"
        [[ -e /dev/syd/"${cmd}${op}${path}" ]]
    done
}

sydbox_internal_net_1()
{
    local cmd="${1}"
    local op="${2}"

    case "${op}" in
    '+'|'-')
        ;;
    *)
        die "${FUNCNAME}: invalid operation character '${op}'"
        ;;
    esac

    shift 2

    local addr
    for addr in "${@}"; do
        [[ -e /dev/sydbox/"${cmd}${op}${addr}" ]]
    done
}

sydbox_internal_net_3()
{
    local cmd="${1}"
    local op="${2}"

    case "${op}" in
    '+'|'-')
        ;;
    *)
        die "${FUNCNAME}: invalid operation character '${op}'"
        ;;
    esac

    shift 2
    while [[ ${#} > 0 ]] ; do
        case "${1}" in
        inet6:*)
            [[ -e "/dev/syd/${cmd}${op}${1##inet6:}" ]]
            ;;
        inet:*)
            [[ -e "/dev/syd/${cmd}${op}${1##inet:}" ]]
            ;;
        unix-abstract:*)
            [[ -e "/dev/syd/${cmd}${op}${1##unix-abstract:}" ]]
            ;;
        unix:*)
            [[ -e "/dev/syd/${cmd}${op}${1##unix:}" ]]
            ;;
        *)
            # Expect network alias.
            # Sydbox does input validation so we don't do any here.
            [[ -e "/dev/syd/${cmd}${op}${1}" ]]
            ;;
        esac
        shift
    done
}

esandbox_3()
{
    local cmd="${1}"

    shift
    case "${cmd}" in
    api)
        echo -n 3
        ;;
    check)
        [[ -e /dev/syd ]]
        ;;
    lock)
        [[ -e "/dev/syd/lock:on" ]]
        ;;
    exec_lock)
        [[ -e "/dev/syd/lock:exec" ]]
        ;;
    wait_all)
        ebuild_notice "warning" "${FUNCNAME} ${cmd} is not implemented for sydbox-3"
        false;;
    wait_eldest)
        ebuild_notice "warning" "${FUNCNAME} ${cmd} is not implemented for sydbox-3"
        false;;
    enabled|enabled_path)
        [[ -e "/dev/syd/sandbox/write?" ]]
        ;;
    enable|enable_path)
        [[ -e "/dev/syd/sandbox/write:on" ]]
        ;;
    disable|disable_path)
        [[ -e "/dev/syd/sandbox/write:off" ]]
        ;;
    enabled_exec)
        [[ -e "/dev/syd/sandbox/exec?" ]]
        ;;
    enable_exec)
        [[ -e "/dev/syd/sandbox/exec:on" ]]
        ;;
    disable_exec)
        [[ -e "/dev/syd/sandbox/exec:off" ]]
        ;;
    enabled_net)
        [[ -e "/dev/syd/sandbox/net?" ]]
        ;;
    enable_net)
        [[ -e "/dev/syd/sandbox/net:on" ]]
        ;;
    disable_net)
        [[ -e "/dev/syd/sandbox/net:off" ]]
        ;;
    allow|allow_path)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path_3 "allowlist/write" '+' "${@}"
        ;;
    disallow|disallow_path)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path_3 "allowlist/write" '-' "${@}"
        ;;
    allow_exec)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path_3 "allowlist/exec" '+' "${@}"
        ;;
    disallow_exec)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path_3 "allowlist/exec" '-' "${@}"
        ;;
    allow_net)
        local c="allowlist/net/bind"
        [[ "${1}" == "--connect" ]] && c="allowlist/net/connect" && shift
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_net_3 "${c}" '+' "${@}"
        ;;
    disallow_net)
        local c="allowlist/net/bind"
        [[ "${1}" == "--connect" ]] && c="allowlist/net/connect" && shift
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_net_3 "${c}" '-' "${@}"
        ;;
    addfilter|addfilter_path)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path_1 "filter/write" '+' "${@}"
        ;;
    rmfilter|rmfilter_path)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path_1 "filter/write" '-' "${@}"
        ;;
    addfilter_exec)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path_1 "filter/exec" '+' "${@}"
        ;;
    rmfilter_exec)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path_1 "filter/exec" '-' "${@}"
        ;;
    addfilter_net)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_net_3 "filter/net" '+' "${@}"
        ;;
    rmfilter_net)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_net_3 "filter/net" '-' "${@}"
        ;;
    exec)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        [[ -e "$(sydbox exec ${@})" ]]
        ;;
    kill)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path_3 "exec/kill" "+" "${@}"
        ;;
    resume)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        ebuild_notice "warning" "${FUNCNAME} ${cmd} is not implemented for sydbox-3"
        false;;
    hack_toolong|nohack_toolong)
        ebuild_notice "warning" "${FUNCNAME} ${cmd} is not implemented for sydbox-3"
        false;;
    *)
        die "${FUNCNAME} subcommand ${cmd} unrecognised"
        ;;
    esac
}

esandbox_1()
{
    local cmd="${1}"

    shift
    case "${cmd}" in
    api)
        echo -n 1
        ;;
    check)
        [[ -e /dev/sydbox ]]
        ;;
    lock)
        [[ -e "/dev/sydbox/core/trace/magic_lock:on" ]]
        ;;
    exec_lock)
        [[ -e "/dev/sydbox/core/trace/magic_lock:exec" ]]
        ;;
    wait_all)
        [[ -e "/dev/sydbox/core/trace/exit_wait_all:true" ]]
        ;;
    wait_eldest)
        [[ -e "/dev/sydbox/core/trace/exit_wait_all:false" ]]
        ;;
    enabled|enabled_path)
        [[ -e "/dev/sydbox/core/sandbox/write?" ]]
        ;;
    enable|enable_path)
        [[ -e "/dev/sydbox/core/sandbox/write:deny" ]]
        ;;
    disable|disable_path)
        [[ -e "/dev/sydbox/core/sandbox/write:off" ]]
        ;;
    enabled_exec)
        [[ -e "/dev/sydbox/core/sandbox/exec?" ]]
        ;;
    enable_exec)
        [[ -e "/dev/sydbox/core/sandbox/exec:deny" ]]
        ;;
    disable_exec)
        [[ -e "/dev/sydbox/core/sandbox/exec:off" ]]
        ;;
    enabled_net)
        [[ -e "/dev/sydbox/core/sandbox/network?" ]]
        ;;
    enable_net)
        [[ -e "/dev/sydbox/core/sandbox/network:deny" ]]
        ;;
    disable_net)
        [[ -e "/dev/sydbox/core/sandbox/network:off" ]]
        ;;
    allow|allow_path)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path_1 "whitelist/write" '+' "${@}"
        ;;
    disallow|disallow_path)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path_1 "whitelist/write" '-' "${@}"
        ;;
    allow_exec)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path_1 "whitelist/exec" '+' "${@}"
        ;;
    disallow_exec)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path_1 "whitelist/exec" '-' "${@}"
        ;;
    allow_net)
        local c="whitelist/network/bind"
        [[ "${1}" == "--connect" ]] && c="whitelist/network/connect" && shift
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_net_1 "${c}" '+' "${@}"
        ;;
    disallow_net)
        local c="whitelist/network/bind"
        [[ "${1}" == "--connect" ]] && c="whitelist/network/connect" && shift
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_net_1 "${c}" '-' "${@}"
        ;;
    addfilter|addfilter_path)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path_1 "filter/write" '+' "${@}"
        ;;
    rmfilter|rmfilter_path)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path_1 "filter/write" '-' "${@}"
        ;;
    addfilter_exec)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path_1 "filter/exec" '+' "${@}"
        ;;
    rmfilter_exec)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path_1 "filter/exec" '-' "${@}"
        ;;
    addfilter_net)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_net_1 "filter/network" '+' "${@}"
        ;;
    rmfilter_net)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_net_1 "filter/network" '-' "${@}"
        ;;
    exec)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        [[ -e "$(sydfmt exec -- ${@})" ]]
        ;;
    kill)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path_1 "exec/kill_if_match" "+" "${@}"
        ;;
    resume)
        [[ ${#} < 1 ]] && die "${FUNCNAME} ${cmd} takes at least one extra argument"
        sydbox_internal_path_1 "exec/resume_if_match" "+" "${@}"
        ;;
    hack_toolong|nohack_toolong)
        ebuild_notice "warning" "${FUNCNAME} ${cmd} is not implemented for sydbox-1"
        false;;
    *)
        die "${FUNCNAME} subcommand ${cmd} unrecognised"
        ;;
    esac
}

esandbox() {
    local api

    # We must run check before API check because it's special.
    if [[ "${1}" == check ]]; then
        if test -e /dev/syd || test -e /dev/sydbox; then
            return 0
        else
            return 1
        fi
    fi

    api="$(sydbox_internal_api)"
    case "${api}" in
    3)
        esandbox_3 "${@}";;
    1)
        esandbox_1 "${@}";;
    0)
        die "${FUNCNAME}: unsupported sydbox API '${api}'"
        ;;
    *)
        die "${FUNCNAME}: unrecognised sydbox API '${api}'"
        ;;
    esac
}

sydboxcheck()
{
    ebuild_notice "warning" "${FUNCNAME} is deprecated, use \"esandbox check\" instead"
    esandbox check
}

sydboxcmd()
{
    die "${FUNCNAME} is dead, use \"esandbox <command>\" instead"
}

addread()
{
    die "${FUNCNAME} not implemented for sydbox yet"
}

addwrite()
{
    ebuild_notice "warning" "${FUNCNAME} is deprecated, use \"esandbox allow\" instead"
    esandbox allow "${1}"
}

adddeny()
{
    die "${FUNCNAME} not implemented for sydbox yet"
}

addpredict()
{
    die "${FUNCNAME} is dead, use \"esandbox addfilter\" instead"
}

rmwrite()
{
    ebuild_notice "warning" "${FUNCNAME} is deprecated, use \"esandbox disallow\" instead"
    esandbox disallow "${1}"
}

rmpredict()
{
    die "${FUNCNAME} is dead, use \"esandbox rmfilter\" instead"
}

addfilter()
{
    ebuild_notice "warning" "${FUNCNAME} is deprecated, use \"esandbox addfilter\" instead"
    esandbox addfilter "${1}"
}

rmfilter()
{
    ebuild_notice "warning" "${FUNCNAME} is deprecated, use \"esandbox rmfilter\" instead"
    esandbox rmfilter "${1}"
}

