FROM   ubuntu:12.04
MAINTAINER Christopher League <league@contrapunctus.net>

RUN    apt-get update
RUN    apt-get install -y build-essential nginx supervisor openssh-server git curl
RUN    apt-get install -y uwsgi-plugin-python python-virtualenv
RUN    apt-get install -y mysql-server libmysqlclient-dev python-dev

ADD    supervisord.conf /etc/supervisor/conf.d/supervisord.conf
ADD    nginx-site.conf /etc/nginx/sites-available/default
ADD    authorized_keys /var/www/.ssh/authorized_keys
ADD    environment.sh /var/www/environment.sh
ADD    . /var/www/uwsgi-deploy

RUN    echo "daemon off;" >>/etc/nginx/nginx.conf
RUN    mkdir /var/run/sshd 
RUN    mkdir /var/www/webapp.git
RUN    git --git-dir=/var/www/webapp.git init --bare
RUN    ln -s ../../uwsgi-deploy/pre-receive /var/www/webapp.git/hooks/
RUN    mkdir -p /etc/uwsgi/vassals /var/log/uwsgi /etc/uwsgi/run
RUN    chown -R www-data:www-data /var/www /etc/uwsgi/vassals /etc/uwsgi/run

# Build AFTER chown, because it creates setuid executable
RUN    make -C /var/www/uwsgi-deploy

EXPOSE 22 80
CMD    /usr/bin/supervisord

