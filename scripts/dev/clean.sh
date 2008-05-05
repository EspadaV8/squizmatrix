#!/bin/sh
#
#/**
#* +--------------------------------------------------------------------+
#* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
#* | ACN 084 670 600                                                    |
#* +--------------------------------------------------------------------+
#* | IMPORTANT: Your use of this Software is subject to the terms of    |
#* | the Licence provided in the file licence.txt. If you cannot find   |
#* | this file please contact Squiz (www.squiz.net) so we may provide   |
#* | you a copy.                                                        |
#* +--------------------------------------------------------------------+
#*
#* $Id: clean.sh,v 1.13 2008/05/05 22:55:50 gsherwood Exp $
#*/

# Creates a clean system by removing data and cache directories
# and clearing out the database and re-inserting the create script


SYSTEM_ROOT=`pwd`;


rm -rf "${SYSTEM_ROOT}/cache" \
		"${SYSTEM_ROOT}/data/file_repository" \
		"${SYSTEM_ROOT}/data/public/assets" \
		"${SYSTEM_ROOT}/data/public/asset_types" \
		"${SYSTEM_ROOT}/data/private/logs" \
		"${SYSTEM_ROOT}/data/private/assets" \
		"${SYSTEM_ROOT}/data/private/db" \
		"${SYSTEM_ROOT}/data/private/events" \
		"${SYSTEM_ROOT}/data/private/asset_map" \
		"${SYSTEM_ROOT}/data/private/maps" \
		"${SYSTEM_ROOT}/data/private/conf/system_assets.inc"

cvs up -dP cache data/public data/private

# OK, what we are doing here is using PHP to do the parsing of the DSN for us (much less error prone :)
php_code="<?php
\$db_conf = require_once('${SYSTEM_ROOT}/data/private/conf/db.inc');
\$dsn = \$db_conf['db2'];
foreach(\$dsn as \$k => \$v) {
	echo 'DB_'.strtoupper(\$k).'=\"'.addslashes(\$v).'\";';
}

if (isset(\$dsn['DSN'])) {
	\$dsn_parts = Array();
	list(\$db_type, \$dsn_split) = preg_split('/:/', \$dsn['DSN']);
	\$dsn_split = preg_split('/[\\s;]+/', \$dsn_split);
	foreach (\$dsn_split as \$dsn_part) {
		\$split = preg_split('/=/', \$dsn_part, 2);
		echo 'DB_DSN_'.strtoupper(\$split[0]).'=\"'.\$split[1].'\";';
	}
}
?>"

eval `echo "${php_code}" | php`

set | grep "^DB_"

case "${DB_TYPE}" in
	"pgsql")
		args="";
		if [ "${DB_USER}" != "" ]; then
			args="${args} -U ${DB_USER}";
		fi
		if [ "${DB_PASSWORD}" != "" ]; then
			args="${args} -W";
		fi
		if [ "${DB_DSN_HOST}" != "" ]; then
			args="${args} -h ${DB_DSN_HOST}";
		fi
		if [ "${DB_DSN_PORT}" != "" ]; then
			args="${args} -p ${DB_DSN_PORT}";
		fi
		psql ${args} -d "${DB_DSN_DBNAME}" -c "\d" -t -q -A -X | awk -F\| '{ print "DROP " $3 " " $2 " CASCADE;" }' | psql ${args} -d "${DB_DSN_DBNAME}" -X -q
	;;

	"oci8")
		args="${DB_USER}/${DB_PASSWORD}@${DB_DSN_HOST}";
		echo ${args};
		sqlplus -S "${args}" "@${SYSTEM_ROOT}/scripts/dev/oracle_drop.sql" "${DB_USER}";
	;;

	*)
		echo "ERROR: DATABASE TYPE '${DB_TYPE}' NOT KNOWN" >&2;
		exit 1;
esac

# now just run step 2 again
php -d output_buffering=0 "${SYSTEM_ROOT}/install/step_02.php" "${SYSTEM_ROOT}"
if [ "$?" = "0" ]; then
	php -d output_buffering=0 "${SYSTEM_ROOT}/install/compile_locale.php" "${SYSTEM_ROOT}" "--locale=en"
	php -d output_buffering=0 "${SYSTEM_ROOT}/install/step_03.php" "${SYSTEM_ROOT}"
	# if step-3 failed then stop it here
	if [ "$?" != "0" ]; then
		exit 1
	fi
	# again to ensure that all type descendants are able to be found
	php -d output_buffering=0 "${SYSTEM_ROOT}/install/step_03.php" "${SYSTEM_ROOT}"
fi

php -d output_buffering=0 "${SYSTEM_ROOT}/install/compile_locale.php" "${SYSTEM_ROOT}" "--locale=en"

chmod 775 cache
find data -type d -exec chmod 2775 {} \; 2> /dev/null
find data -type f -exec chmod 664 {} \; 2> /dev/null

