#!/bin/sh
# Copyright (C) 2020  Ali Abdallah <ali.abdallah@suse.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

ESCONFD_DIR=$1
TEST=$2
ESCONFD=$ESCONFD_DIR/esconfd

ESCONF_RUN_IN_TEST_MODE=1; export ESCONF_RUN_IN_TEST_MODE;

exec_esconfd()
{
	rm $ESCONFD_DIR/.esconfd-test-pid >/dev/null 2>&1
	rm $ESCONFD_DIR/.esconfd-sum >/dev/null 2>&1

	exec ${ESCONFD} &
	echo $! > $ESCONFD_DIR/.esconfd-test-pid
	pid=`cat $ESCONFD_DIR/.esconfd-test-pid`
	sum $ESCONFD > $ESCONFD_DIR/.esconfd-sum
}

cleanup()
{
	pid=`cat $ESCONFD_DIR/.esconfd-test-pid`

	rm $ESCONFD_DIR/.esconfd-test-pid >/dev/null 2>&1
	rm $ESCONFD_DIR/.esconfd-sum >/dev/null 2>&1

	while kill -0 $pid >/dev/null 2>&1; do
		kill -s TERM $pid >/dev/null 2>&1
		sleep 0.1
	done
}

prepare()
{
	if [ ! -f $ESCONFD ]; then
		exit 1
	fi

	# Start esconfd only if it is not started already
	if [ ! -f $ESCONFD_DIR/.esconfd-test-pid ]; then
		exec_esconfd
	elif [ ! -f $ESCONFD_DIR/.esconfd-sum ]; then
		cleanup
		exec_esconfd
	else
		oldsum=`cat $ESCONFD_DIR/.esconfd-sum`
		newsum=`sum ${ESCONFD}`

		# Did esconfd changes ?
		if [ "$newsum" != "$oldsum" ]; then
			cleanup
			exec_esconfd
		else
			pid=`cat $ESCONFD_DIR/.esconfd-test-pid`
			kill -s 0 $pid >/dev/null 2>&1

			# Is it still running ?
			if [ $? != 0 ]; then
				cleanup
				exec_esconfd
			fi
		fi
	fi
}

sigint()
{
	cleanup
	exit 1
}

trap 'sigint'  INT

# Last test, cleanup
TEST_NAME=$(basename $TEST)

if [ "$TEST_NAME" = "t-tests-end" ]; then
	cleanup
	exit 0
fi
# Prepare esconfd
prepare

$TEST
ret=$?
# Test failed, cleanup
if [ $ret -ne 0 ]; then
  cleanup
fi
exit $ret
