
FROM gcc:14

WORKDIR /app

COPY backend/ ./backend/
COPY frontend/ ./frontend/

RUN gcc -D_DEFAULT_SOURCE -O2 backend/main.c -o contacts-server

ENV PORT=10000

EXPOSE 10000

CMD ["./contacts-server"]
