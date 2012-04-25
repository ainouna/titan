#! /bin/sh
#
### BEGIN INIT INFO
# Provides:             minidlna
# Required-Start:       $network
# Required-Stop:        $network
# Short-Description:    MiniDLNA Server
### END INIT INFO
 
NAME=minidlna
DESC="MiniDLNA"
DAEMON=/usr/sbin/minidlna
USER=root
GROUP=root
 
params="-f /etc/minidlna.conf -R"
 
case $1 in
  start)
    start-stop-daemon --verbose --start --exec /usr/sbin/minidlna -- ${params}
  ;;
  stop)
    start-stop-daemon --verbose --stop --exec /usr/sbin/minidlna
  ;;
  restart)
    $0 stop
    $0 start
  ;;
  *)
    echo "Usage: /etc/init.d/minidlna {start|restart|stop}"
  ;;
esac