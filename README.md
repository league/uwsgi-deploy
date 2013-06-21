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
  then `~/environment.sh`, assuming they exist. The reason we use both
  is that you may want some environment variables to contain passwords
  or secret keys that should not be stored in the repository.

  5. Next, we run the `$WORKDIR/setup` script. Any error code returned
  by that script aborts the deployment and refuse the push. This
  script can do things like set up your Python `virtualenv` and
  migrate your database schema.

  6. At this point, we expect to find your uWSGI "vassal"
  configuration in `$WORKDIR/uwsgi.ini` (it can be created by the
  `setup` script, if you wish). We expand on the configuration by
  adding the variables defined in the `environment.sh` files.

  7. Next, we copy the `$WORKDIR/uwsgi.ini` to the vassals
  configuration directory, adding the parameters for an HTTP socket on
  a random port. If your server does not respond successfully (200 OK)
  to a request for the URL `/ping`, then we abort the deployment. This
  gives your application the opportunity to ensure the database is up,
  and essentials are in place before we replace the running version.

  8. Assuming everything looks okay so far, the vassal is reconfigured
  to connect to a UNIX socket. Once that socket becomes available, we
  replace the symbolic link to which your web server (probably
  [nginx](http://nginx.org/)) is forwarding traffic.

  9. The vassal for the previous version is removed, gracefully
  freeing up its resources.

  10. A symlink is updated within `~`, pointing to the `$WORKDIR`.

**Note:** If you would rather keep the required files
(`environment.sh`, `setup`, `uwsgi.ini`) in some sub-directory of your
repository, you may create a symbolic link `.deploy` pointing to
that sub-directory.

The file `~/deploy.log` will be kept up to date with the list of
successful deployments, with time stamps.

The script `deploy-version` can be used to manually deploy any
previous version on the file system. Use it to revert if a problem
develops with the newest pushed version.

The script `deploy-cleanup` will remove all but the latest four
deployed versions from the file system. Specify a parameter to keep a
different number of versions.

