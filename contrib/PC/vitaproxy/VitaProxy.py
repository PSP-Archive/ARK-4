#!/usr/bin/env python3

__version__ = "0.0.0"

import time, datetime
import BaseHTTPServer, select, socket, SocketServer, urlparse
import logging
import logging.handlers
import getopt
import sys
import os
import signal
import threading
from types import FrameType, CodeType
from time import sleep
import ftplib
import re

DEFAULT_LOG_FILENAME = "proxy.log"

class RangeError(Exception):
    pass

class ProxyHandler (BaseHTTPServer.BaseHTTPRequestHandler):
    __base = BaseHTTPServer.BaseHTTPRequestHandler
    __base_handle = __base.handle

    server_version = "Apache"
    rbufsize = 0                        # self.rfile Be unbuffered
    enable_psv_fix = 1
    expert_mode = 1
    bufsize = 65536
    show_speed = 1
    update_interval = 3

    def handle(self):
        (ip, port) =  self.client_address
        # self.server.logger.log (logging.INFO, "Request from '%s'", ip)
        if hasattr(self, 'allowed_clients') and ip not in self.allowed_clients:
            self.raw_requestline = self.rfile.readline()
            if self.parse_request(): self.send_error(403)
        else:
            self.__base_handle()

    def _connect_to(self, netloc, soc):
        i = netloc.find(':')
        if i >= 0:
            host_port = netloc[:i], int(netloc[i+1:])
        else:
            host_port = netloc, 80
        # self.server.logger.log (logging.INFO, "connect to %s:%d", host_port[0], host_port[1])
        try: soc.connect(host_port)
        except socket.error, arg:
            try: msg = arg[1]
            except: msg = arg
            self.send_error(503, msg)
            return 0
        return 1

    def fix_path(self):
        if not self.enable_psv_fix:
            return

        last_http = self.path.rfind("http://")

        if last_http != 0 and last_http != -1:
            if self.expert_mode:
                self.server.logger.log(logging.DEBUG, "fixing path %s", self.path)
            self.path =  self.path[last_http:]

    def do_CONNECT(self):
        self.log_message("CONNECT %s", self.path)
        soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        try:
            if self._connect_to(self.path, soc):
                self.send_response(200, "OK")
                self.end_headers()
                self._read_write(soc, 300)
        finally:
            soc.close()
            self.connection.close()

    def get_file_length(self, fn):
        return os.stat(fn).st_size

    def get_range(self, replace_fn, range_str, file_length):
        if not range_str.startswith("bytes="):
            raise RangeError

        range_s_start, range_s_end = range_str[len("bytes="):].split('-')
        range_start = int(range_s_start) if range_s_start else -1
        range_end = int(range_s_end) if range_s_end else -1

        if range_start != -1:
            if range_start >= file_length:
                raise RangeError
            start = range_start
        else:
            start = 0

        if range_end != -1:
            if range_end >= file_length:
                raise RangeError
            end = range_end
        else:
            end = file_length - 1

        if range_start != -1 and range_end != -1 and range_start > range_end:
            raise RangeError

        return start, end

    def get_local_cache(self, replace_fn, head_only):
        try:
            file_length = self.get_file_length(replace_fn)
            start, end = 0, file_length - 1
            datestring = os.path.getmtime(replace_fn)
            datestring = datetime.datetime.utcfromtimestamp(datestring)
            datestring = datestring.strftime('%a, %d %b %Y %H:%M:%S GMT')
        except IOError as e:
            self.send_error(500, "Internal Server Error")
            return

        if 'Range' in self.headers:
            try:
                start, end = self.get_range(replace_fn, self.headers['Range'], file_length)
            except RangeError as e:
                self.send_error(416, 'Requested Range Not Satisfiable')
                return

            self.send_response(206, "Partial Content")
            self.send_header("Content-Range", "bytes %d-%d/%d" % (start, end, file_length))
        else:
            self.send_response(200, "OK")

        self.send_header("Accept-Ranges", "bytes")
        self.send_header("Content-Type", "application/octet-stream")
        self.send_header("Content-Length", "%d" % (end - start + 1))
        self.send_header("Cache-Control", "max-age=3600")
        self.send_header("Connection", "close")
        self.send_header("Last-Modified", datestring)
        self.end_headers()

        if head_only:
            return

        self.server.logger.log(logging.DEBUG, "Range: from %d to %d", start, end)

        with open(replace_fn, "rb") as fd:
            self._file_read_write(fd, start, end)

    def do_GET(self, head_only=False):
        self.close_connection = 1
        self.fix_path()

        if self.expert_mode:
            self.log_message("%s", self.path)
        else:
            if ".pkg" in self.path:
                self.log_message("%s", self.path)

        (scm, netloc, path, params, query, fragment) = urlparse.urlparse(self.path, 'http')

        if not path:
            path = '/'

        if scm not in ('http', 'ftp') or fragment or not netloc:
            self.send_error(400, "bad url %s" % self.path)
            return

        soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        try:
            for e in self.server.replace_list:
                if e[0].startswith('re:'):
                    r = re.compile(e[0][3:])

                    if r.match(self.path):
                        replace_url = self.path
                        replace_fn = e[1]
                        self.log_message("cache: %s -> %s", replace_url, replace_fn)
                        self.get_local_cache(replace_fn, head_only)
                        return

                if e[0].startswith('search:') and e[0][7:] in self.path:
                    replace_url = self.path
                    replace_fn = e[1]
                    self.log_message("cache: %s -> %s", replace_url, replace_fn)
                    self.get_local_cache(replace_fn, head_only)
                    return

                if e[0] == self.path:
                    replace_url = self.path
                    replace_fn = e[1]
                    self.log_message("cache: %s -> %s", replace_url, replace_fn)
                    self.get_local_cache(replace_fn, head_only)
                    return

            if scm == 'http':
                if self._connect_to(netloc, soc):
                    soc.send("%s %s %s\r\n" % (self.command,
                                               urlparse.urlunparse(('', '', path,
                                                                    params, query,
                                                                    '')),
                                               self.request_version))
                    self.headers['Connection'] = 'close'
                    del self.headers['Proxy-Connection']
                    for key_val in self.headers.items():
                        soc.send("%s: %s\r\n" % key_val)
                    soc.send("\r\n")
                    self._read_write(soc)
            elif scm == 'ftp':
                # fish out user and password information
                i = netloc.find ('@')
                if i >= 0:
                    login_info, netloc = netloc[:i], netloc[i+1:]
                    try: user, passwd = login_info.split (':', 1)
                    except ValueError: user, passwd = "anonymous", None
                else: user, passwd ="anonymous", None

                try:
                    ftp = ftplib.FTP (netloc)
                    ftp.login (user, passwd)
                    if self.command == "GET":
                        ftp.retrbinary ("RETR %s"%path, self.connection.send)
                    ftp.quit ()
                except Exception, e:
                    self.server.logger.log (logging.WARNING, "FTP Exception: %s",
                                            e)
        finally:
            soc.close()
            self.connection.close()

    def _file_read_write(self, fd, start, end):
        count = 0
        max_count = end - start + 1
        rest = max_count - count

        if self.show_speed:
            tm_a = [time.time(), count]
            tm_b = [time.time(), count]

        fd.seek(start)

        while rest > 0:
            data = fd.read(min(self.bufsize, rest))
#           self.server.logger.log(logging.DEBUG, "read %d bytes" %(len(data)))

            if not data:
                break

            count += len(data)
            rest -= len(data)
            self.connection.send(data)

            if self.show_speed:
                tm_b = [time.time(), count]
                delta = tm_b[0] - tm_a[0]

                if delta >= self.update_interval:
                    speed = (tm_b[1] - tm_a[1]) / delta
                    self.log_message("Speed: %.2fKB/S, Transfered: %d bytes, Remaining: %d bytes" % (speed / 1000, count, rest))
                    self.log_message("ETA: %d seconds" % (rest / speed))
                    tm_a = tm_b

    def _read_write(self, soc, max_idling=20, local=False):
        iw = [self.connection, soc]
        local_data = ""
        ow = []
        count = 0
        while 1:
            count += 1
            (ins, _, exs) = select.select(iw, ow, iw, 1)
            if exs: break
            if ins:
                for i in ins:
                    if i is soc: out = self.connection
                    else: out = soc
                    data = i.recv(self.bufsize)
                    if data:
                        if local: local_data += data
                        else: out.send(data)
                        count = 0
            if count == max_idling: break
        if local: return local_data
        return None

    def do_HEAD(self):
        return self.do_GET(True)

    do_POST = do_GET
    do_PUT  = do_GET
    do_DELETE=do_GET

    def log_message (self, format, *args):
        self.server.logger.log (logging.INFO, "%s", format % args)
        
    def log_error (self, format, *args):
        self.server.logger.log (logging.ERROR, "%s", format % args)

class ThreadingHTTPServer (SocketServer.ThreadingMixIn,
                           BaseHTTPServer.HTTPServer):
    def __init__ (self, server_address, RequestHandlerClass, logger=None, cache_list = "cache.txt"):
        BaseHTTPServer.HTTPServer.__init__ (self, server_address,
                                            RequestHandlerClass)
        self.logger = logger
        self.replace_list = []
        self.load_cache_list(cache_list)

    def load_cache_list(self, fn):
        try:
            with open(fn, "r") as f:
                for l in f:
                    l = l.decode(errors='ignore').strip()

                    if not l or l[0] == '#':
                        continue

                    if '->' in l:
                        url, fn = l.split('->')
                    else:
                        url = l

                        if url.rfind('?') > url.rfind('/'):
                            fn = url[url.rfind('/')+1:url.rfind('?')]
                        else:
                            fn = url[url.rfind('/')+1:]

                    if not fn:
                        continue

                    try:
                        open(fn).close()
                        self.replace_list.append([url, fn])
                        # self.logger.log(logging.DEBUG, "loaded cache: url %s -> file %s " % (url, fn))
                    except IOError as e:
                        pass

            self.logger.log(logging.INFO, "%d local caches loaded" % (len(self.replace_list)))
        except IOError as e:
            pass

def logSetup (filename, log_size, daemon):
    logger = logging.getLogger ("VitaProxy")
    logger.setLevel (logging.DEBUG)
    if not filename:
        if not daemon:
            # display to the screen
            handler = logging.StreamHandler ()
        else:
            handler = logging.handlers.RotatingFileHandler (DEFAULT_LOG_FILENAME,
                                                            maxBytes=(log_size*(1<<20)),
                                                            backupCount=5)
    else:
        handler = logging.handlers.RotatingFileHandler (filename,
                                                        maxBytes=(log_size*(1<<20)),
                                                        backupCount=5)
    """
    fmt = logging.Formatter ("[%(asctime)-12s.%(msecs)03d] "
                             "%(levelname)-8s {%(name)s %(threadName)s}"
                             " %(message)s",
                             "%Y-%m-%d %H:%M:%S")
    """
    fmt = logging.Formatter ("%(message)s")
    handler.setFormatter (fmt)
        
    logger.addHandler (handler)
    return logger

def usage (msg=None):
    if msg: print msg
    print sys.argv[0], "[-p port] [-l logfile] [-c cachelist] [-dh] [allowed_client_name ...]]"
    print
    print "   -p       - Port to bind to"
    print "   -l       - Path to logfile. If not specified, STDOUT is used"
    print "   -c       - Load cache list"
    print "   -d       - Run in the background"
    print

def handler (signo, frame):
    while frame and isinstance (frame, FrameType):
        if frame.f_code and isinstance (frame.f_code, CodeType):
            if "run_event" in frame.f_code.co_varnames:
                frame.f_locals["run_event"].set ()
                return
        frame = frame.f_back
    
def daemonize (logger):
    class DevNull (object):
        def __init__ (self): self.fd = os.open ("/dev/null", os.O_WRONLY)
        def write (self, *args, **kwargs): return 0
        def read (self, *args, **kwargs): return 0
        def fileno (self): return self.fd
        def close (self): os.close (self.fd)
    class ErrorLog:
        def __init__ (self, obj): self.obj = obj
        def write (self, string): self.obj.log (logging.ERROR, string)
        def read (self, *args, **kwargs): return 0
        def close (self): pass
        
    if os.fork () != 0:
        ## allow the child pid to instanciate the server
        ## class
        sleep (1)
        sys.exit (0)
    os.setsid ()
    fd = os.open ('/dev/null', os.O_RDONLY)
    if fd != 0:
        os.dup2 (fd, 0)
        os.close (fd)
    null = DevNull ()
    log = ErrorLog (logger)
    sys.stdout = null
    sys.stderr = log
    sys.stdin = null
    fd = os.open ('/dev/null', os.O_WRONLY)
    #if fd != 1: os.dup2 (fd, 1)
    os.dup2 (sys.stdout.fileno (), 1)
    if fd != 2: os.dup2 (fd, 2)
    if fd not in (1, 2): os.close (fd)

def main ():
    logfile = None
    daemon  = False
    max_log_size = 20
    port = 12345
    allowed = []
    run_event = threading.Event ()
    cache_list = "cache.txt"
    
    try: opts, args = getopt.getopt (sys.argv[1:], "l:c:dhp:", [])
    except getopt.GetoptError, e:
        usage (str (e))
        return 1

    for opt, value in opts:
        if opt == "-p": port = int (value)
        if opt == "-l": logfile = value
        if opt == "-d": daemon = not daemon
        if opt == "-c": cache_list = value
        if opt == "-h":
            usage ()
            return 0
        
    # setup the log file
    logger = logSetup (logfile, max_log_size, daemon)
    
    if daemon:
        daemonize (logger)
    signal.signal (signal.SIGINT, handler)
        
    if args:
        allowed = []
        for name in args:
            client = socket.gethostbyname(name)
            allowed.append(client)
            logger.log (logging.INFO, "Accept: %s (%s)" % (client, name))
        ProxyHandler.allowed_clients = allowed
    else:
        logger.log (logging.INFO, "Any clients will be served...")

    server_address = ("0.0.0.0", port)
    ProxyHandler.protocol_version = "HTTP/1.1"

    try:
        os.chdir(os.path.dirname(cache_list))
    except OSError as e:
        pass

    httpd = ThreadingHTTPServer (server_address, ProxyHandler, logger, cache_list)
    sa = httpd.socket.getsockname ()
    print "Servering HTTP on", sa[0], "port", sa[1]
    req_count = 0
    while not run_event.isSet ():
        try:
            httpd.handle_request ()
            req_count += 1
            if req_count == 1000:
                logger.log (logging.INFO, "Number of active threads: %s",
                            threading.activeCount ())
                req_count = 0
        except select.error, e:
            if e[0] == 4 and run_event.isSet (): pass
            else:
                logger.log (logging.CRITICAL, "Errno: %d - %s", e[0], e[1])
        except socket.error as e:
            if e.errno == 10054:
                print ("Connection reset by peer")
            else:
                print (str(e))
                
    logger.log (logging.INFO, "Server shutdown")
    return 0

if __name__ == '__main__':
    sys.exit (main ())
