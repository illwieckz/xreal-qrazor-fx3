#
# Regular cron jobs for the xreal package
#
0 4	* * *	root	[ -x /usr/bin/xreal_maintenance ] && /usr/bin/xreal_maintenance
