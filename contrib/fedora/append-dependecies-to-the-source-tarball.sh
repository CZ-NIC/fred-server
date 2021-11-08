#!/usr/bin/bash

set -x
set -o errexit
set -o pipefail

export GIT_UPSTREAM='https://gitlab.nic.cz'
export GLOBAL_PREFIX="$1"
export SOURCE_TARBALL="$2"
export SOURCE_DIR="${SOURCE_TARBALL%/*}"

while read -r repo commit clone_path
do [[ "$repo" != "#" ]] && {
    repofile=$(echo "$repo" | tr '/' '-')
    clone_branch=$(git ls-remote --tags "$GIT_UPSTREAM"/"$repo" |\
        grep "$commit" |\
        grep -v '\^{}' |\
        cut -d/ -f3 |\
        tr '-' '~' |\
        sort -V |\
        tr '~' '-' |\
        tail -1) || :
    clone_branch=${clone_branch:-$commit}
    git clone "$GIT_UPSTREAM"/"$repo" "$clone_path" --depth 1 --branch "$clone_branch"
    (cd "$clone_path" && \
            git archive --prefix="${GLOBAL_PREFIX}/${clone_path}"/ --format=tar HEAD -o "${SOURCE_DIR}/_${repofile}-${commit}.tar" && \
            tar --concatenate --file="${SOURCE_TARBALL}" "${SOURCE_DIR}/_${repofile}-${commit}.tar" && \
            rm "${SOURCE_DIR}/_${repofile}-${commit}.tar")
}
done < dependencies.txt

gzip < "${SOURCE_TARBALL}" > "${SOURCE_TARBALL}.gz" && rm "${SOURCE_TARBALL}"

