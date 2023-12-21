FROM alpine:latest
RUN apk update
RUN apk add mosquitto
RUN apk add mosquitto-clients
COPY mosquitto.conf /etc/mosquitto/mosquitto.conf
RUN mosquitto_ctrl dynsec init /etc/mosquitto/dynamic-security.json naveen mypassword
RUN chown mosquitto:mosquitto /etc/mosquitto/dynamic-security.json
RUN chmod 777 /etc/mosquitto/dynamic-security.json
COPY run_mosquitto.sh /sbin/run_mosquitto.sh
CMD /sbin/run_mosquitto.sh
