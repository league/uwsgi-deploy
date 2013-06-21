# uwsgi-deploy

This is a facility that will automatically deploy a web application to
the [uWSGI](http://projects.unbit.it/uwsgi/) server (in
"[Emperor](http://uwsgi-docs.readthedocs.org/en/latest/Emperor.html)"
mode) when you do `git push`.

The basic workflow is:

  1. Develop and test your application locally.

  2. Use `git push` to an account on your server (probably `www-data`).

  3. The `pre-receive` hook of the server repository checks out a
  fresh copy of the latest code into some `$WORKDIR`.

  4. We load environment variables from `$WORKDIR/environment.sh` and
  then `~/environment.sh` and then run the `$WORKDIR/setup` script.
  Any error code returned by that script aborts the deployment and
  refuse the push.

  5. Next, we copy the `$WORKDIR/uwsgi.ini` to the "vassals"
  configuration, adding the configuration of an HTTP socket on a
  random port. If your server does not respond successfully (200 OK)
  to a request for the URL `/ping`, then we abort the deployment. This
  gives your application the opportunity to ensure the database is up,
  and essentials are in place before we replace the running version.

  6. Assuming everything looks okay so far, the vassal is reconfigured
  to connect to a UNIX socket. Once that socket becomes available, we
  replace the symbolic link to which your web server (probably
  [nginx](http://nginx.org/)) is forwarding traffic.

  7. The vassal for the previous version is removed, gracefully
  freeing up its resources.

