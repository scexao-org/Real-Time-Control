#!/usr/bin/env python
#
# Telstat.py -- wrapper program to start the Telstat set of processes.
#
#[ Eric Jeschke (eric@naoj.org) --
#  Last edit: Mon Sep 20 12:20:47 HST 2010
#]
#
"""
This is startup wrapper to the Telstat program written by Fujitsu.
It sets up all the environment variables used by the various Telstat
programs, and invokes the processes in a new process group.

USAGE:

    $ Telstat.py --log=Telstat_stdout_stderr.log

    OR
    
    $ Telstat.py --stderr

Use ^C or send SIGTERM to the top level process to kill it.  Terminating
the top process properly kills the entire set of Telstat processes.
"""

import sys, os, signal, time
from optparse import OptionParser
import ssdlog
import myproc
import cfg.g2soss as g2soss

# maximum number of times we should attempt to restart telstat
max_restarts = 20


def main(options, args):
    """
    Create a subprocess to start the Telstat subsystem.  Then wait for user
    to terminate it with ^C
    """

    # Is user trying to kill an existing instance?
    if options.kill:
        try:
            proc = myproc.getproc(pidfile=options.pidfile)

        except myproc.myprocError:
            print "Failed to read PID file: %s" % options.pidfile
            sys.exit(1)

        # Check status of process.
        if proc.status().startswith('exited'):
            print "Telstat processes already exited"
            sys.exit(1)

        # Attempt to kill process group
        proc.signalpg(signal.SIGKILL)
        sys.exit(0)

    if options.logfile:
        if options.logstderr:
            print "Sorry, cannot use --stderr and --log together in this app!"
            sys.exit(1)

        try:
            f_out = open(options.logfile, 'a')

        except IOError, e:
            print "Cannot open stdout/stderr log file: %s" % str(e)
            sys.exit(1)

    else:
        # Default to stderr if no --log specified
        f_out = sys.stderr

    proc = None
    
    # This function is called if we receive a signal, terminate the app,
    # etc.
    def quit(exitcode):
        if proc:
            f_out.write("Stopping Telstat subsystem...\n")
            f_out.flush()
            try:
                # Make sure everything in the PG is killed
                proc.killpg()

            except Exception, e:
                # If user used a normal exit mechanism from GUI then
                # trying to kill it here raises an error.
                if str(e) != "[Errno 3] No such process":
                    f_out.write("Error stopping subprocesses: %s\n" % str(e))
                    exitcode = 1

        f_out.close()
        sys.exit(exitcode)

    def SigHandler(signum, frame):
        """Signal handler for all unexpected conditions."""
        f_out.write("Received signal %d !\n" % signum)
        quit(1)

    # Set signal handler for signals.  Add any other signals you want to
    # handle or terminate here.
    signal.signal(signal.SIGTERM, SigHandler)
    signal.signal(signal.SIGHUP, SigHandler)

    env = g2soss.make_env('TELSTAT', options=options)

    # Wierd little hack to make TelStat pick up a decent font
    os.system("cd /usr/share/fonts/truetype; xset +fp msttcorefonts")

    def start_telstat():
        try:
            # Run Telstat in a process group.  This makes it easy
            # to kill any and all subprocesses.
            proc = myproc.myproc('(cd %s; ./OSSO_TelStat --g2 -c status --loglevel=0  --geometry %s)' % (
                    g2soss.telstathome, options.geometry),
                                 env=env, addenv=True,
                                 close_fds=True,
                                 usepg=(not options.nopg), 
                                 pidfile=options.pidfile,
                                 stdout=f_out, stderr=f_out)

            return proc

        except Exception, e:
            f_out.write("Could not create Telstat process: %s\n" % str(e))
            quit(1)

    # Now wait for user to press ^C.  TODO: check for GUI exit.
    print "Press ^C to terminate Telstat subsystem..."
    try:
        count = 0
        while count < max_restarts:
            proc = start_telstat()
            
            status = proc.status()
            while status == 'running':
                status = proc.status()
                time.sleep(1.0)

            # Possible TODO: could try to restart Telstat here...
            f_out.write("Telstat seems to have died--attempting to restart...\n")
            time.sleep(1.0)
            count += 1

        f_out.write("Reached maximum number of restart attempts (%d); terminating...\n" % max_restarts)

    except KeyboardInterrupt:
        print "Caught keyboard interrupt!"
        quit(0)
        
    except Exception, e:
        # TODO: log the traceback
        f_out.write("Caught exception: %s\n" % str(e))

    quit(1)

        
if __name__ == '__main__':

    usage = "usage: %prog [options]"
    optprs = OptionParser(usage=usage, version=('%%prog'))
    optprs.add_option("--debug", dest="debug", default=False, action="store_true",
                      help="Enter the pdb debugger on main()")
    optprs.add_option("--display", dest="display", metavar="HOST:N",
                      help="Use X display on HOST:N")
    optprs.add_option("-g", "--geometry", dest="geometry",
                      metavar="GEOM", default="+20+100",
                      help="X geometry for initial size and placement")
    optprs.add_option("--kill", dest="kill", default=False,
                      action="store_true",
                      help="Attempt to kill running instance of VGW")
    optprs.add_option("--nopg", dest="nopg", default=False,
                      action="store_true",
                      help="Do not form a process group")
    optprs.add_option("--pidfile", dest="pidfile",
                      default='/tmp/VGW.pid', metavar="FILE",
                      help="Store process group id in FILE")
    optprs.add_option("--profile", dest="profile", action="store_true",
                      default=False,
                      help="Run the profiler on main()")
    ssdlog.addlogopts(optprs)

    (options, args) = optprs.parse_args(sys.argv[1:])

    if len(args) != 0:
        optprs.error("incorrect number of arguments")

    # Are we debugging this?
    if options.debug:
        import pdb

        pdb.run('main(options, args)')

    # Are we profiling this?
    elif options.profile:
        import profile

        print "%s profile:" % sys.argv[0]
        profile.run('main(options, args)')

    else:
        main(options, args)
       
#END


