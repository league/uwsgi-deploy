
def hello_world(environ, start_response):
    a = environ['SOME_BIT_OF_CONFIG']
    b = environ['MY_SUPER_BIG_SECRET']
    start_response('200 OK', [('Content-Type', 'text/plain')])
    return ["Hello, world.\n", a, "\n", b, "\n"]

application = hello_world
