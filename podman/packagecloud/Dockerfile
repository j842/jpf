FROM docker.io/library/ruby:2.4

RUN set -ex && \
    gem install package_cloud

COPY entrypoint.sh /usr/local/bin
RUN chmod +x /usr/local/bin/entrypoint.sh

ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]

