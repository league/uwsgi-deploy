
def hello_world(environ, start_response):
    start_response('200 OK', [('Content-Type', 'text/html')])
    return ["Hello, world: ", str(environ)]

application = hello_world
