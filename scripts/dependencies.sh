#!/bin/bash

exit_on_failure=true

declare -A RETVAL_ARR

print_r() {
    local -n arr=$1
    local -r arr_name=$1
    for key in "${!arr[@]}"; do
        printf "%s[\"%s\"]=\"%s\"\\n" "$arr_name" "$key" "${arr[$key]}"
    done
}

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
            printf "destination path '%s' already exists and is a git repository, cleaning up\\n" "$clone_path"
            rm -fr "$clone_path"
        fi
    fi
    git -c advice.detachedHead=false clone "$url" "$clone_path" --depth 1 --recurse-submodules --branch "$clone_branch" || exit 1
    git -c advice.detachedHead=false -C "$clone_path" status
}

clone_dependencies() {
    local -r working_dir=$1
    local -r dependencies="dependencies.txt"
    (
        cd "$working_dir" || exit 1
        [[ -s "$dependencies" ]] || exit 1
        while read -r repository commitish clone_path; do
            [[ "${repository:0:1}" == "#" ]] && continue
            git_clone "$repository" "$clone_path" "$commitish"
            # shellcheck disable=SC2181
            if [[ "$?" -ne 0 ]]; then
                printf "Failed dependency: %s %s %s\\n" "$repository" "$commitish" "$clone_path" >&2
                if $exit_on_failure; then
                    exit
                fi
            fi
            if $recursive; then
                clone_dependencies "$clone_path"
            fi
        done < "$dependencies"
    )
    return 0
}

recursive=false
[[ "$1" == "-r" ]] && recursive=true
[[ "$1" == "--recursive" ]] && recursive=true

clone_dependencies "."
