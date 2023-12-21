#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <mosquitto.h>
#include <json-c/json.h>
#include <pthread.h>
#include <time.h>

//#define MQTT_BROKER_URL "192.168.1.196"
#define MQTT_BROKER_URL "15.206.203.14"
#define MQTT_PORT 1883
#define MQTT_USERNAME "user1"
#define MQTT_PASSWORD "user1password"

struct mosquitto *mqtt;
bool connection = false;

// thread function to send health status to monitor topic every 5 seconds
void* send_health_status(void *ptr){
	printf("Health status thread launched\n");
	char buffer[128] = {0};
	int result = 0;
	while(true){
		sleep(5);
		struct json_object *main_obj;
		main_obj = json_object_new_object();
		if(main_obj){
			FILE *fp = NULL;
			fp = popen("free -h | grep Mem | awk '{print $3}'", "r");
			if(fp){
				memset(buffer, 0, sizeof(buffer));
				fscanf(fp, "%s", buffer);
				pclose(fp);
				json_object_object_add(main_obj, "ram_free", json_object_new_int(atoi(buffer)));
				json_object_object_add(main_obj, "timestamp", json_object_new_int64(time(NULL)));
			}
			const char *payload_str = json_object_to_json_string(main_obj);
			result = mosquitto_publish(mqtt, NULL, "monitor", strlen(payload_str), payload_str, 0, 0);
			if(result != MOSQ_ERR_SUCCESS){
				printf("Publish failed\n");
			} else {
				printf("Publish success\n");
			}
			json_object_put(main_obj);
		}
	}
}

// connection callback function
void connect_callback(struct mosquitto *mqtt, void *data, int reason_code){
	int result = 0;
	result = mosquitto_subscribe(mqtt, NULL, "topic1", 0);
	if(result != MOSQ_ERR_SUCCESS){
		printf("Failed to subscribe to the topic\n");
	}

	pthread_t tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&tid, &attr, send_health_status, NULL);
}

// log callback function
void log_callback(struct mosquitto *mqtt, void *userData, int level, const char *str){
	printf("Log: %s\n", str);
}

//subscription callback function
void subscribe_callback(struct mosquitto *mqtt, void *data, int qos, int mid, const int *granted_qos){
	printf("Subscription callback\n");
}

// message callback function
void message_callback(struct mosquitto *mqtt, void *obj, const struct mosquitto_message *msg){
	struct json_object *main_obj = NULL;
	struct json_object *data_obj = NULL;
	char msg_id[128] = {0};
	char msg_data[1024] = {0};
	printf("Message received: %s\n", msg->payload);
	main_obj = json_tokener_parse(msg->payload);
	if(main_obj){
		if(json_object_object_get_ex(main_obj, "msg_id", &data_obj)){
			strcpy(msg_id, json_object_get_string(data_obj));
			if(json_object_object_get_ex(main_obj, "payload", &data_obj)){
				strcpy(msg_data, json_object_get_string(data_obj));
			}

			printf("Received msg_id: %s\n", msg_id);
			printf("Received data: %s\n", msg_data);
		} else {
			printf("Incorrect json object, ignoring\n");
		}
		json_object_put(main_obj);
	} else {
		printf("payload is not json object, ignoring\n");
	}
	
}


// publish callback function
void publish_callback(struct mosquitto *mqtt, void *data, int qos){
	printf("Publish callback\n");
}


// signal handlers for graceful exit
void signal_handler(int signum){
	printf("Caught signal, cleaning up\n");
	switch(signum){
		case SIGINT:
		case SIGKILL:
			if(connection){
				mosquitto_disconnect(mqtt);
			}
			mosquitto_lib_cleanup();
			break;
		default:
			break;
	}
}

int main(int argc, char **argv){
	int result = 0;


	// setup signal handlers
	signal(SIGINT, signal_handler);
	signal(SIGKILL, signal_handler);

	// initialize the mosquitto library
	mosquitto_lib_init();
	
	// create mosquitto object
	mqtt = mosquitto_new(NULL, true, NULL);
	if(!mqtt){
		printf("Failed to create mqtt object\n");
		return -1;
	}

	// set the username and password 
	result = mosquitto_username_pw_set(mqtt, MQTT_USERNAME, MQTT_PASSWORD);
	if(result != MOSQ_ERR_SUCCESS){
		printf("Failed to set the username and password\n");
		goto cleanup;
	}

	// callbacks for mosquitto 
	mosquitto_connect_callback_set(mqtt, connect_callback);
	mosquitto_subscribe_callback_set(mqtt, subscribe_callback);
	mosquitto_log_callback_set(mqtt, log_callback);
	mosquitto_message_callback_set(mqtt, message_callback);
	mosquitto_publish_callback_set(mqtt, publish_callback);

	// connect to MQTT broker
	result = mosquitto_connect(mqtt, MQTT_BROKER_URL, MQTT_PORT, 60);
	if(result != MOSQ_ERR_SUCCESS){
		printf("Failed to connect to mqtt broker, exiting\n");
		goto cleanup;
	}
	connection = true;

	printf("connect success, starting service operations\n");
	// start the mqtt operational service
	// this is a blocking call
	mosquitto_loop_forever(mqtt, -1, 1);

	// disconnect from mqtt
	mosquitto_disconnect(mqtt);

cleanup:
	mosquitto_lib_cleanup();
	return 0;
}
