FROM ubuntu:16.04
COPY assets /usr/bin/assets
COPY reader /usr/bin/
COPY hb /usr/bin/
COPY start.sh /usr/bin/
COPY config.json /usr/bin/
RUN chmod +x /usr/bin/start.sh
ENTRYPOINT ["start.sh"]

