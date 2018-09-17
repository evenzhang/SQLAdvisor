FROM centos:7

MAINTAINER yulo <uyy2244@gmail.com>

RUN yum install gcc gcc-c++ cmake make git libaio-devel libffi-devel glib2 glib2-devel wget -y 
RUN yum install http://www.percona.com/downloads/percona-release/redhat/0.1-4/percona-release-0.1-4.noarch.rpm  -y  
RUN yum update -y 
RUN yum install Percona-Server-shared-56 -y 
RUN git clone https://github.com/evenzhang/SQLAdvisor-HTTP.git /SQLAdvisor



RUN wget http://www.monkey.org/~provos/libevent-2.0.10-stable.tar.gz
RUN tar zxvf libevent-2.0.10-stable.tar.gz

WORKDIR /libevent-2.0.10-stable
RUN ./configure
RUN make
RUN make install
RUN ln -s /usr/local/lib/libevent-2.0.so.5 /usr/lib64/libevent-2.0.so.5

WORKDIR /SQLAdvisor

RUN  cmake -DBUILD_CONFIG=mysql_release -DCMAKE_BUILD_TYPE=debug -DCMAKE_INSTALL_PREFIX=/usr/local/sqlparser ./ 
RUN make 
RUN make install


WORKDIR /SQLAdvisor/sqladvisor-http

RUN  cmake -DCMAKE_BUILD_TYPE=debug ./ 
RUN make 
RUN ln -s /SQLAdvisor/sqladvisor-http/sqladvisor /bin/sqladvisor

ENTRYPOINT  ["sqladvisor"]

