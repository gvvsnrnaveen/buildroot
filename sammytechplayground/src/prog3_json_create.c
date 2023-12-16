#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <json-c/json.h>

// structure to host sample data
struct sample_data {
	int id; //identifier of the data
	char name[128]; // name of the person
	char designation[128]; // designation of the person
	char address[256]; // address of the person
};

int main(int argc, char **argv){
	int i = 0;
	struct sample_data data_obj[3] = {0};
	struct json_object *main_obj = NULL;

	for (i=0; i < 3; i++){
		memset(&data_obj[i], 0, sizeof(struct sample_data));
		data_obj[i].id = i + 1;
		snprintf(data_obj[i].name, sizeof(data_obj[i].name), "DemoUser%d", i + 1);
		snprintf(data_obj[i].designation, sizeof(data_obj[i].designation), "DemoDesignation%d", i + 1);
		snprintf(data_obj[i].address, sizeof(data_obj[i].address), "Address%d", i + 1);
	}
	
	// create the json object from scratch
	main_obj = json_object_new_object();
	if(main_obj){
		// create a json array object
		struct json_object *arr_obj = json_object_new_array();
		if(arr_obj){
			// create the individual json objects for struct sample_data array and add to json array
			for(i=0; i < 3; i++){
				struct json_object *user_obj = json_object_new_object();
				json_object_object_add(user_obj, "id", json_object_new_int(data_obj[i].id));
				json_object_object_add(user_obj, "name", json_object_new_string(data_obj[i].name));
				json_object_object_add(user_obj, "designation", json_object_new_string(data_obj[i].designation));
				json_object_object_add(user_obj, "address", json_object_new_string(data_obj[i].address));
				json_object_array_add(arr_obj, user_obj);
			}
			json_object_object_add(main_obj, "data", arr_obj);
		}

		// print the generated JSON string
		printf("Result: %s\n", json_object_to_json_string(main_obj));

		// Write the data to output file 
		FILE *fp = NULL;
		fp = fopen("sample_data.json", "w");
		if(fp){
			fprintf(fp, "%s", json_object_to_json_string(main_obj));
			fclose(fp);
		}
	
		json_object_put(main_obj);
	}
	return 0;
}
