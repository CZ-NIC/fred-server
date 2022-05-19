#!/bin/bash

# if false, script result code is not very useful (is status of the last dependency)
exit_on_failure=true

declare -A RETVAL_ARR

default_options() {
    help=false
    usage=false

    verbose=false
    strategy="bfs" # dfs
    force=false
    check=false
}

parse_options() {
    options=$#
    opts=""
    leave=false

    while [[ $# -gt 0 && "$leave" == "false" ]]; do
        case "$1" in
            -h|--help)
                help=true
                ;;
            -v|--verbose)
                verbose=true
                ;;
            -s|--strategy|--strategy=*)
                if [[ "$1" =~ ^-.*=.*$ ]]; then strategy=${1#*=};
                else opts="$opts $1"; shift; strategy="$1"; fi
                ;;
            -f|--force)
                force=true
                ;;
            -c|--check)
                check=true
                ;;
            -r|--recursive)
                printf "obsolete option -r ignored, recursive is default now\\n" >&2
                ;;
            --)
                leave=true
                ;;
            -*)
                usage=true
                ;;
            *)
                leave=true
                ;;
        esac
        if ! $leave; then opts="$opts $1"; fi
        if ! $leave; then shift; fi
    done;
    to_shift=$((options - $#))
}

process_options() {
    if $usage; then
        usage
        exit 1;
    fi

    if $help; then
        usage
        exit 0;
    fi
}

options() {
    default_options
    parse_options "$@"
    process_options
}

usage() {
    printf "Usage: %s [--strategy <bfs|dfs>] [--force] [dir]\\n" "$0"
}

print_r() {
    local -n arr=$1
    local -r arr_name=$1
    for key in "${!arr[@]}"; do
        printf "%s[\"%s\"]=\"%s\"\\n" "$arr_name" "$key" "${arr[$key]}"
    done
}

# https://tools.ietf.org:html/rfc3986#appendix-B
#local -r url_regex='^(([^:/?#]+)://)?(((([^:/?#]+)@)?([^:/?#]+)(:([^/]+))?))?(/([^?#]*))(\?([^#]*))?(#(.*))?'

parse_url() {
    local -r url=$1
    local -r schema="(https?|ssh)://"
    local -r login="[[:alnum:]-]*"
    local -r password="[^@]*"
    local -r host="[[:alnum:].-]+"
    local -r port="[[:digit:]]+"
    local -r path=".*?"
    local -r git="\.git"
    local -r url_regex="^(($schema)?((($login)(:($password)?)?)@)?($host)((:($port))?))((/$path)($git)?)?\$"
    RETVAL_ARR=()

    [[ "$url" =~ $url_regex ]]

    local -r detected_schema=${BASH_REMATCH[2]}

    if [[ -n "$detected_schema" ]]; then
        RETVAL_ARR["type"]="url"
        RETVAL_ARR["url"]=${BASH_REMATCH[0]}
        RETVAL_ARR["base"]=${BASH_REMATCH[1]}
        RETVAL_ARR["schema"]=${BASH_REMATCH[3]}
        RETVAL_ARR["credentials@"]=${BASH_REMATCH[4]}
        RETVAL_ARR["credentials"]=${BASH_REMATCH[5]}
        RETVAL_ARR["login"]=${BASH_REMATCH[6]}
        RETVAL_ARR["password"]=${BASH_REMATCH[8]}
        RETVAL_ARR["host"]=${BASH_REMATCH[9]}
        RETVAL_ARR[":port"]=${BASH_REMATCH[11]}
        RETVAL_ARR["port"]=${BASH_REMATCH[12]}
        RETVAL_ARR["path"]=${BASH_REMATCH[13]}
    else
        local -r git_regex="^((git)@($host))(:($path)($git)?)?\$"

        [[ "$url" =~ $git_regex ]]

        if [[ ${#BASH_REMATCH[@]} -gt 0 ]]; then
            RETVAL_ARR["type"]="git"
            RETVAL_ARR["url"]=${BASH_REMATCH[0]}
            RETVAL_ARR["base"]=${BASH_REMATCH[1]}
            RETVAL_ARR["login"]=${BASH_REMATCH[2]}
            RETVAL_ARR["host"]=${BASH_REMATCH[3]}
            RETVAL_ARR["path"]=${BASH_REMATCH[5]}
        else
            RETVAL_ARR["type"]="path"
            RETVAL_ARR["path"]=$url
        fi
    fi
}

git_clone() {
    local -r repository=$1
    local -r clone_path=$2
    local -r commitish=$3
    local url

    parse_url "$repository"

    case "${RETVAL_ARR["type"]}" in
        url|git)
            url=${RETVAL_ARR["url"]}
            ;;
        path)
            local -r git_upstream=$(git config remote.origin.url)
            parse_url "$git_upstream"
            case "${RETVAL_ARR["type"]}" in
                url)
                    #url="${RETVAL_ARR["base"]}/$repository.git"
                    url="ssh://git@${RETVAL_ARR["host"]}${RETVAL_ARR[":port"]}/$repository.git"
                    ;;
                git)
                    url="${RETVAL_ARR["base"]}:$repository.git"
                    ;;
                path|*)
                    print_r RETVAL_ARR
                    printf "unexpected remote.origin.url (%s) type (url)\\n" "${RETVAL_ARR["path"]}" >&2
                    exit 1
                    ;;
            esac
            ;;
    esac
    clone_branch=$(git ls-remote --tags --refs "$url" | grep "/${commitish}\([^0-9].*\)\?$" | cut -d/ -f3 | tr '-' '~' | sort -V | tr '~' '-' | tail -1)
    clone_branch=${clone_branch:-$commitish}
    if [[ -d "$clone_path" ]]; then
        if [[ ! -d "$clone_path/.git" ]]; then
            printf "destination path '%s' already exists and is not a git repository\\n" "$clone_path" >&2
            # if the directory is not empty, cloning will fail
        else
            printf "destination path '%s' already exists and is a git repository\\n" "$clone_path"
            if $force; then
                printf "running with --force option active, cleaning up\\n"
                rm -fr "$clone_path"
            else
                printf "run with --force to clean up\\n"
                exit 1
            fi
        fi
    fi
    git -c advice.detachedHead=false clone "$url" "$clone_path" --depth 1 --recurse-submodules --branch "$clone_branch" || exit 1
    git -c advice.detachedHead=false -C "$clone_path" status
}

git_check() {
    local -r repository=$1
    local -r clone_path=$2
    local -r commitish=$3
    local url

    parse_url "$repository"

    case "${RETVAL_ARR["type"]}" in
        url|git)
            url=${RETVAL_ARR["url"]}
            ;;
        path)
            local -r git_upstream=$(git config remote.origin.url)
            parse_url "$git_upstream"
            case "${RETVAL_ARR["type"]}" in
                url)
                    #url="${RETVAL_ARR["base"]}/$repository.git"
                    url="ssh://git@${RETVAL_ARR["host"]}${RETVAL_ARR[":port"]}/$repository.git"
                    ;;
                git)
                    url="${RETVAL_ARR["base"]}:$repository.git"
                    ;;
                path|*)
                    print_r RETVAL_ARR
                    printf "unexpected remote.origin.url (%s) type (url)\\n" "${RETVAL_ARR["path"]}" >&2
                    exit 1
                    ;;
            esac
            ;;
    esac
    if ! git ls-remote "$url" 'refs/heads/*' | grep -w "refs/heads/$commitish" >/dev/null 2>&1; then
        printf "\"%s\" is not an unmerged branch (consider updating dependencies.txt if it was merged)\\n" "$commitish"
        return 1
    fi
}

clone_dependencies() {
    local -r working_dir=$1
    local -r dependencies_file="dependencies.txt"
    local -A dependencies_local
    pushd "$working_dir" || exit 1
    if [[ -s "$dependencies_file" ]]; then
        while read -r repository commitish clone_path; do
            [[ "${repository:0:1}" == "#" ]] && continue
            if [[ -v "dependencies[$repository]" ]]; then
                continue
            fi
            dependencies["$repository"]=""
            dependencies_local["$repository"]=""
            git_clone "$repository" "$clone_path" "$commitish"
            # shellcheck disable=SC2181
            if [[ "$?" -eq 0 ]]; then
                if [[ "$strategy" == "dfs" ]]; then
                    clone_dependencies "$clone_path"
                fi
            else
                printf "Failed dependency: %s %s %s\\n" "$repository" "$commitish" "$clone_path" >&2
                if $exit_on_failure; then
                    exit 1
                fi
            fi
        done < "$dependencies_file"
        if [[ "$strategy" == "bfs" ]]; then
            while read -r repository commitish clone_path; do
                [[ "${repository:0:1}" == "#" ]] && continue
                if [[ -v "dependencies_local[$repository]" ]]; then
                    clone_dependencies "$clone_path"
                fi
            done < "$dependencies_file"
        fi
    fi
    popd || exit 1
    return $?
}

check_dependencies() {
    local -r working_dir=$1
    local -r level=${2:-0}
    local -r dependencies_file="dependencies.txt"
    (
        cd "$working_dir" || exit 1
        [[ -s "$dependencies_file" ]] || exit 0
        while read -r repository commitish clone_path; do
            [[ "${repository:0:1}" == "#" ]] && continue
            if $verbose; then
                printf "%*s%s %s %s\\n" "$((level*4))" "" "$repository" "$commitish" "$clone_path"
            fi
            git_check "$repository" "$clone_path" "$commitish"
            # shellcheck disable=SC2181
            if [[ "$?" -eq 0 ]]; then
                if [[ "$strategy" == "dfs" ]]; then
                    check_dependencies "$clone_path" "$((level+1))"
                fi
            else
                printf "Failed dependency: %s %s %s\\n" "$repository" "$commitish" "$clone_path" >&2
                if $exit_on_failure; then
                    exit 1
                fi
            fi
        done < "$dependencies_file"
        if [[ "$strategy" == "bfs" ]]; then
            while read -r repository commitish clone_path; do
                [[ "${repository:0:1}" == "#" ]] && continue
                if $verbose; then
                    printf "%*s%s %s %s\\n" "$((level*4))" "" "$repository" "$commitish" "$clone_path"
                fi
                check_dependencies "$clone_path" "$((level+1))"
            done < "$dependencies_file"
        fi
    )
    return $?
}

options "$@"
if [[ "$to_shift" -gt 0 ]]; then shift $to_shift; to_shift=0; fi

declare -A dependencies

dir=${1:.}
if $check; then
    check_dependencies "$dir"
else
    clone_dependencies "$dir"
fi
