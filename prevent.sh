#!/bin/sh
PRODUCT="`basename \`dirname \\\`pwd\\\`\``"
WORKDIR="/tmp/${PRODUCT}/"
COVDB="/opt/prevent-database-dir"
COVROOT="/opt/prevent-linux-4.5.0/"

echo "product name:     $PRODUCT"
echo "intermediate dir: $WORKDIR"
echo "cov database:     $COVDB"

${COVROOT}/bin/cov-build --dir ${WORKDIR} --config ${COVROOT}/config/coverity_config.xml make && \
${COVROOT}/bin/cov-analyze --all --dir ${WORKDIR} --config ${COVROOT}/config/coverity_config.xml && \
${COVROOT}/bin/cov-commit-defects --datadir ${COVDB} --product ${PRODUCT} --user admin --dir ${WORKDIR}
