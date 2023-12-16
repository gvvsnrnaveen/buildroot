#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <json-c/json.h>

int main(int argc, char **argv){
	FILE *fp = NULL;
	char buffer[1024] = {0};
	struct json_object *main_obj = NULL;
	struct json_object *data_obj = NULL;

	printf("Hello Sammy'sTechPlayground\n");


	// read the entire file data
	fp = fopen("read_data.json", "r");
	if(fp){
		fread(buffer, sizeof(buffer), 1024, fp);
		fclose(fp);
	}
	printf("File data: %s\n", buffer);

	// parse the json object
	main_obj = json_tokener_parse(buffer);
	if(main_obj){
		printf("Parsing success\n");

		if(json_object_object_get_ex(main_obj, "name", &data_obj)){
			printf("Name from JSON: %s\n", json_object_get_string(data_obj));
		}

		if(json_object_object_get_ex(main_obj, "passion", &data_obj)){
			printf("Passion from JSON: %s\n", json_object_get_string(data_obj));
		}

		if(json_object_object_get_ex(main_obj, "linkedin-url", &data_obj)){
			printf("LinkedIn URL from JSON: %s\n", json_object_get_string(data_obj));
		}

		json_object_put(main_obj);
	}

	return 0;
}
