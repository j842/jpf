FROM ubuntu:22.04

RUN echo 'APT::Install-Suggests "0";' >> /etc/apt/apt.conf.d/00-docker
RUN echo 'APT::Install-Recommends "0";' >> /etc/apt/apt.conf.d/00-docker

RUN apt-get -y update && \
    apt-get install -y python3 python3-pip nodejs nginx fcgiwrap && \
    rm -rf /var/lib/apt/lists/*
RUN pip install google-sheets-to-csv


COPY copy/nginx-jpf.conf /etc/nginx/sites-available/nginx-jpf.conf
RUN ln -s /etc/nginx/sites-available/nginx-jpf.conf /etc/nginx/sites-enabled/

RUN mkdir /var/www/scripts
COPY copy/update.sh /var/www/scripts/update.sh
RUN chmod -R 755 /var/www/scripts


COPY copy/jpf /usr/bin/jpf
RUN chmod 755 /usr/bin/jpf

