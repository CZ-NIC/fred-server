#!/bin/sh
PRODUCT="`basename \`dirname \\\`pwd\\\`\``"
WORKDIR="/var/home/coverity/intermediate/${PRODUCT}/"
COVDB="/var/home/coverity/prevent-database"
COVROOT="/opt/prevent-linux64-4.5.1/"

echo "product name:     $PRODUCT"
echo "intermediate dir: $WORKDIR"
echo "cov database:     $COVDB"

${COVROOT}/bin/cov-build --dir ${WORKDIR} --config ${COVROOT}/config/coverity_config.xml make && \
${COVROOT}/bin/cov-analyze --all --dir ${WORKDIR} --config ${COVROOT}/config/coverity_config.xml && \
${COVROOT}/bin/cov-commit-defects --datadir ${COVDB} --product ${PRODUCT} --user admin --dir ${WORKDIR}
