#!/usr/bin/env bash

check_db_healthy()
{
    docker exec -i "$CONTAINER_NAME" /usr/bin/env sh -c "pg_isready -qt 10 -U postgres -d postgres && \
    pg_isready -qt 10 -h 127.0.0.1 -U postgres -d postgres"
}

wait_for_db()
{
    local cntdn=50
    while [ 0 -lt $cntdn ] && ! check_db_healthy; do
        sleep .5
        ((--cntdn))
    done
}

stop_db()
{
    if docker container ls --filter=status=running --format "{{.Names}}"|grep -q "^${CONTAINER_NAME}$"; then
        echo -ne "Stopping container $CONTAINER_NAME ..."
        docker container stop "$CONTAINER_NAME" > /dev/null
        echo -ne "\r                                                           \r"
    fi
}

start_db()
{
    echo -ne "Starting Postgres $POSTGRES_VERSION server ..."
    docker run \
        -d --rm \
        -p ${DB_HOST}:${DB_PORT}:5432 \
        -e "POSTGRES_PASSWORD=$DB_ADMIN_PASSWORD" \
        --name "$CONTAINER_NAME" \
        postgres:$POSTGRES_VERSION > /dev/null
    wait_for_db
    echo -e "\rIn container $CONTAINER_NAME on $DB_HOST:$DB_PORT is listening Postgres $POSTGRES_VERSION server"
}

exec_sql()
{
    docker exec -i "$CONTAINER_NAME" psql -wXqbf - -o /dev/null "$@"
}

init_db()
{
    local root_dir=$(dirname "$0")
    cat "$root_dir/create_db.sql" | sed "s,{PASSWORD},$DB_FRED_PASSWORD,g" | exec_sql -U postgres -d postgres
    cat "$root_dir/create_extensions.sql" | exec_sql -U fred -d fred
    "$FRED_DB_INSTALL_SCRIPT" --without-log | exec_sql -U fred -d fred

}

set_default()
{
    local varname="$1"
    if [ -z "${!varname}" ]; then
        local default_value="$2"
        export $varname="$default_value"
    fi
}

print_usage()
{
    echo -e "Usage:\n\n" \
        $(basename "$0") "[--container-name <NAME>] [--fred-db-install-script <SCRIPT>] [--db-host <IP>] [--db-port <PORT>] [--db-admin-password <ADMIN>] [--db-fred-password <USER>] [--postgres-version <VERSION>]\n\n" \
        "\tStarts an empty fred database in container <NAME> listening on <IP>:<PORT> initialized by output of '<SCRIPT> --without-log' invocation and running on PostgreSQL server of\n" \
        "\tspecified <VERSION>. Password for database user postgres on database postgres is <ADMIN> and password for database user fred on database fred is <USER>.\n\n" \
        "\tDefault values:\n" \
        "\t\tNAME - CONTAINER_NAME environment variable or 'fred-database'\n" \
        "\t\tSCRIPT - FRED_DB_INSTALL_SCRIPT environment variable\n" \
        "\t\tIP - DB_HOST environment variable or '127.0.0.1'\n" \
        "\t\tPORT - DB_PORT environment variable or 25432\n" \
        "\t\tADMIN - DB_ADMIN_PASSWORD environment variable or 'eyeing4eeTha'\n" \
        "\t\tUSER - DB_FRED_PASSWORD environment variable or 'iWae2pho5noh'\n" \
         "\t\tVERSION - POSTGRES_VERSION environment variable or 13\n\n" \
        "or\n\n" \
        $(basename "$0") "-h|--help - print this help"
}

parse_args()
{
    while [ 0 -lt $# ]; do
        case "$1" in
            -h|--help)
                print_usage
                exit 0
                ;;
            --container-name)
                CONTAINER_NAME="$2"
                shift 2
                ;;
            --fred-db-install-script)
                FRED_DB_INSTALL_SCRIPT="$2"
                shift 2
                ;;
            --db-host)
                DB_HOST="$2"
                shift 2
                ;;
            --db-port)
                DB_PORT="$2"
                shift 2
                ;;
            --db-admin-password)
                DB_ADMIN_PASSWORD="$2"
                shift 2
                ;;
            --db-fred-password)
                DB_FRED_PASSWORD="$2"
                shift 2
                ;;
            --postgres-version)
                POSTGRES_VERSION="$2"
                shift 2
                ;;
            *)
                echo -e "Invalid option $1.\n" >&2
                print_usage
                exit 1
                ;;
        esac
    done
    if [ -z "$FRED_DB_INSTALL_SCRIPT" ]; then
        echo -e "FRED_DB_INSTALL_SCRIPT not specified.\n" >&2
        print_usage
        exit 1
    fi
    set_default CONTAINER_NAME "fred-database"
    set_default DB_HOST "127.0.0.1"
    set_default DB_PORT 25432
    set_default DB_ADMIN_PASSWORD "eyeing4eeTha"
    set_default DB_FRED_PASSWORD "iWae2pho5noh"
    set_default POSTGRES_VERSION "13"
}

main()
{
    parse_args "$@"
    stop_db
    start_db
    init_db
}

main "$@"