import os
from django import get_version

def show_var(buf, v):
    buf.append(v)
    buf.append(" is ")
    buf.append(os.environ[v])
    buf.append("\n")

def hello_world(environ, start_response):
    try:
        buf = ["Hello, world...\n"]
        buf.append('Django %s is available.\n' % get_version())
        show_var(buf, 'SOME_BIT_OF_CONFIG')
        show_var(buf, 'MY_SUPER_BIG_SECRET')
        start_response('200 OK', [('Content-Type', 'text/plain')])
        return buf
    except KeyError as exn:
        start_response('500 Internal Server Error',
                       [('Content-Type', 'text/plain')])
        return ["Missing variable: ", str(exn)]

application = hello_world
