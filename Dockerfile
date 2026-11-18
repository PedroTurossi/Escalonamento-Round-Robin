FROM gcc:13
WORKDIR /app
COPY RoundRobinV1.c .
RUN gcc RoundRobinV1.c -o escalonador
CMD ["./escalonador"]
