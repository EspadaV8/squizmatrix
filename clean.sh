#!/bin/sh
#/**
#* Copyright (c) 2003 - Squiz Pty Ltd
#*
#* $Id: clean.sh,v 1.8 2003/09/29 08:00:08 brobertson Exp $
#* $Name: not supported by cvs2svn $
#*/

# Creates a clean system by removing data and cache directories 
# and clearing out the database and re-inserting the create script


SYSTEM_ROOT=`dirname "$0"`;


rm -rf "${SYSTEM_ROOT}/cache" \
		"${SYSTEM_ROOT}/data/file_repository" \
		"${SYSTEM_ROOT}/data/public/assets" \
		"${SYSTEM_ROOT}/data/public/asset_types" \
		"${SYSTEM_ROOT}/data/private/logs" \
		"${SYSTEM_ROOT}/data/private/assets" \
		"${SYSTEM_ROOT}/data/private/db" \
		"${SYSTEM_ROOT}/data/private/asset_map"

cvs up -dP cache data/public data/private


# OK, what we are doing here is using PHP to do the parsing of the DSN for us (much less error prone :)
# see the output of DB::parseDSN
php_code="<?php
require_once '${SYSTEM_ROOT}/data/private/conf/main.inc';
require_once 'DB.php';
\$dsn = DB::parseDSN(SQ_CONF_DB_DSN);
foreach(\$dsn as \$k => \$v) {
	echo 'DB_'.strtoupper(\$k).'=\"'.addslashes(\$v).'\";';
}
?>"
eval `echo "${php_code}" | php`

set | grep "^DB_"

case "${DB_PHPTYPE}" in 
	"mysql")
		args="";
		if [ "${DB_USERNAME}" != "" ]; then
			args="${args} -u ${DB_USERNAME}";
		fi
		if [ "${DB_PASSWORD}" != "" ]; then
			args="${args} -p ${DB_PASSWORD}";
		fi
		if [ "${DB_HOSTSPEC}" != "" ]; then
			args="${args} -h ${DB_HOSTSPEC}";
		fi
		if [ "${DB_PORT}" != "" ]; then
			args="${args} -P ${DB_PORT}";
		fi
set -x;
		mysql ${args} -e "SHOW TABLES;" -s -N "${DB_DATABASE}" | sed 's/^.*$/DROP TABLE &;/' | mysql ${args} "${DB_DATABASE}"
set +x;
	;;

	"pgsql")
		args="";
		if [ "${DB_USERNAME}" != "" ]; then
			args="${args} -U ${DB_USERNAME}";
		fi
		if [ "${DB_PASSWORD}" != "" ]; then
			args="${args} -W";
		fi
		if [ "${DB_HOSTSPEC}" != "" ]; then
			args="${args} -h ${DB_HOSTSPEC}";
		fi
		if [ "${DB_PORT}" != "" ]; then
			args="${args} -p ${DB_PORT}";
		fi
		psql ${args} -d "${DB_DATABASE}" -c "\d" -t -q -A -X | awk -F\| '{ print "DROP " $3 " " $2 ";" }' | psql ${args} -d "${DB_DATABASE}" -X -q
	;;

	*)
		echo "ERROR: DATABASE TYPE '${DB_TYPE}' NOT KNOWN" >&2;
		exit 1;
esac

# now just run step 2 again
php -d output_buffering=0 "${SYSTEM_ROOT}/install/step_02.php" "${SYSTEM_ROOT}"
if [ "$?" = "0" ]; then
	php -d output_buffering=0 "${SYSTEM_ROOT}/install/step_03.php" "${SYSTEM_ROOT}"
fi

chmod 775 cache
find data -type d -exec chmod 2775 {} \; 2> /dev/null
find data -type f -exec chmod 664 {} \; 2> /dev/null

