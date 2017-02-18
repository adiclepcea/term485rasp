FROM alpine:3.5
COPY assets /tmp/assets
COPY reader /tmp
COPY hb /tmp
ENTRYPOINT ["/tmp/start.sh"]

