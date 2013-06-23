import os

def hello_world(environ, start_response):
    try:
        buf = ["Hello, world.\n"]
        buf.append(os.environ['SOME_BIT_OF_CONFIG'])
        buf.append("\n")
        buf.append(os.environ['MY_SUPER_BIG_SECRET'])
        buf.append("\n")
        start_response('200 OK', [('Content-Type', 'text/plain')])
        return buf
    except KeyError as exn:
        start_response('500 Internal Server Error',
                       [('Content-Type', 'text/plain')])
        return ["Missing variable: ", str(exn)]

application = hello_world
