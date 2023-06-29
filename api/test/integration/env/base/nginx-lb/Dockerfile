FROM nginx

COPY nginx.conf /etc/nginx/nginx.conf

ADD entrypoint.sh /scripts/entrypoint.sh

<<<<<<< HEAD
HEALTHCHECK --interval=1s --timeout=3s --retries=10 CMD curl --fail http://localhost/ || exit 1
=======
HEALTHCHECK --interval=30s --timeout=3s CMD curl --fail http://localhost/ || exit 1
>>>>>>> cb9703fd01 (Save nginx in save_logs)
