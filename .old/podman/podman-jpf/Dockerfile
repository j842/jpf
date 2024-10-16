FROM docker.io/library/ruby

RUN set -ex && \
    gem install fpm package_cloud

COPY .packagecloud /root/.packagecloud

