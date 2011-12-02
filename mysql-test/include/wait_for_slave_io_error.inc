# ==== Purpose ====
#
# Waits until the IO thread of the current connection has got an
# error, or until a timeout is reached. Also waits until the IO
# thread has completely stopped.
#
#
# ==== Usage ====
#
# --let $slave_io_errno= NUMBER [, NUMBER ...]
# [--let $show_slave_io_error= 1]
# [--let $slave_io_error_is_nonfatal= 1]
# [--let $rpl_debug= 1]
# [--let $slave_timeout= NUMBER]
# --source include/wait_for_slave_io_error.inc
#
# Parameters:
#   $slave_io_errno
#     The expected IO error numbers. This can be either a single
#     number, or a comma-separated list of numbers. Examples:
#       --let $slave_io_errno= 1040, 1053, 2002, 2003, 2006, 2013
#       --let $slave_io_errno= 1045
#     (After BUG#41956 has been fixed, this will be required to be
#     symbolic names instead of numbers.)
#
#   $show_slave_io_error
#     If set, will print the error to the query log.
#
#   $slave_io_error_is_nonfatal
#     By default, we wait for the slave IO thread to stop completely
#     (i.e., until Slave_IO_State is empty).  If this variable is set,
#     then we don't wait. This is useful if the error is non-fatal
#     (e.g., temporary connection error) and does not cause the slave
#     IO thread to stop.
# 
#   $slave_timeout
#     See include/wait_for_slave_param.inc
# 
#   $rpl_debug
#     See include/rpl_init.inc


--let $include_filename= wait_for_slave_io_error.inc [errno=$slave_io_errno]
--source include/begin_include_file.inc


let $old_slave_param_comparison= $slave_param_comparison;
let $slave_param= Last_IO_Errno;
let $slave_param_comparison= !=;
let $slave_param_value= 0;
source include/wait_for_slave_param.inc;
let $slave_param_comparison= $old_slave_param_comparison;

let $_wfsie_errno= query_get_value(SHOW SLAVE STATUS, Last_IO_Errno, 1);

if ($slave_io_errno == '') {
  --echo !!!ERROR IN TEST: you must set \$slave_io_errno before you source
  --echo !!!wait_for_slave_sql_error.inc. The error we got this time was '$_wfsie_errno',
  --echo !!!so you probably want to add the following line to your test case:
  --echo !!!  --let \$slave_io_errno= $_wfsie_errno
  --die !!!ERROR IN TEST: you must set \$slave_io_errno before sourcing wait_for_slave_io_error.inc
}

if (`SELECT $_wfsie_errno NOT IN ($slave_io_errno)`) {
  --echo **** Slave stopped with wrong error code: $_wfsie_errno (expected $slave_io_errno) ****
  source include/show_rpl_debug_info.inc;
  --echo **** Slave stopped with wrong error code: $_wfsie_errno (expected $slave_io_errno) ****
  --die Slave stopped with wrong error code
}

if ($show_slave_io_error)
{
  --let $_wait_for_slave_io_error_error= query_get_value("SHOW SLAVE STATUS", Last_IO_Error, 1)
  # remove timestamp and mask absolute path
  --let $_wait_for_slave_io_error_error= `SELECT REPLACE("$_wait_for_slave_io_error_error", '$MYSQL_TEST_DIR', 'MYSQL_TEST_DIR')`
  --echo Last_IO_Error = '$_wait_for_slave_io_error_error'
}

if (!$slave_io_error_is_nonfatal)
{
  --let $slave_param= Slave_IO_State
  --let $slave_param_value=
  --source include/wait_for_slave_param.inc
}
--let $slave_io_error_is_nonfatal= 0


--let $include_filename= wait_for_slave_io_error.inc [errno=$slave_io_errno]
--source include/end_include_file.inc