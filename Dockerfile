FROM   ubuntu:12.04
MAINTAINER Christopher League <league@contrapunctus.net>

RUN    apt-get update
RUN    apt-get install -y build-essential
RUN    apt-get install -y nginx upstart
RUN    apt-get install -y uwsgi-plugin-python python-virtualenv
RUN    apt-get install -y mysql-server libmysqlclient-dev python-dev
RUN    apt-get install -y openssh-server
RUN    apt-get install -y git

ADD    authorized_keys /var/www/.ssh/authorized_keys
ADD    . /var/www/uwsgi-deploy
RUN    make -C /var/www/uwsgi-deploy
RUN    mkdir /var/run/sshd 
RUN    mkdir /var/www/webapp.git
RUN    git --git-dir=/var/www/webapp.git init --bare
RUN    ln -s ../../uwsgi-deploy/pre-receive /var/www/webapp.git/hooks/

EXPOSE 22
CMD    /usr/sbin/sshd -D

