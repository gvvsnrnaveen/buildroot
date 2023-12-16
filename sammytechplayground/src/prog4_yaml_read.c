#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <yaml.h>

struct address {
	char city[128];
	char state[128];
	char country[128];
};

struct person {
	char name[128];
	int age;
	struct address addr;
};

int main(int argc, char **argv){
	FILE *fp = NULL;
	struct person obj;

	memset(&obj, 0, sizeof(struct person));

	fp = fopen("read_data.yaml", "r");
	if(!fp){
		printf("Failed to open read_data.yaml file\n");
		return -1;
	}
	yaml_parser_t yaml_parser;
	yaml_token_t token;

	// Initialize the yaml parser object
	if(!yaml_parser_initialize(&yaml_parser)){
		printf("Failed to create parser object\n");
		return -1;
	}

	// set the file as parse source
	yaml_parser_set_input_file(&yaml_parser, fp);

	char data_type[128]={0};
	char placeholder[128] = {0};

	do {
		yaml_parser_scan(&yaml_parser, &token);
		
		switch(token.type)
		{
			case YAML_STREAM_START_TOKEN:
				break;
			case YAML_STREAM_END_TOKEN:
				break;
			case YAML_KEY_TOKEN:
				memset(data_type, 0, sizeof(data_type));
				strcpy(data_type, "key");
				break;
			case YAML_VALUE_TOKEN:
				memset(data_type, 0, sizeof(data_type));
				strcpy(data_type, "value");
			       break;
			case YAML_BLOCK_SEQUENCE_START_TOKEN:
			       break;
			case YAML_BLOCK_ENTRY_TOKEN:
			       break;
			case YAML_BLOCK_END_TOKEN:
			       break;
			case YAML_BLOCK_MAPPING_START_TOKEN:
			       break;
			case YAML_SCALAR_TOKEN:
				if(!strncmp(data_type, "key", 3)){
					// current data is key, so copy to placeholder
					memset(placeholder, 0, sizeof(placeholder));
					strcpy(placeholder, token.data.scalar.value);
				}
				if(!strncmp(data_type, "value", 5)){
					if(!strncmp(placeholder, "name", 4)){
						strcpy(obj.name, token.data.scalar.value);
					}
					if(!strncmp(placeholder, "age", 3)){
						obj.age = atoi(token.data.scalar.value);
					}
					if(!strncmp(placeholder, "city", 4)){
						strcpy(obj.addr.city, token.data.scalar.value);
					}
					if(!strncmp(placeholder, "state", 5)){
						strcpy(obj.addr.state, token.data.scalar.value);
					}
					if(!strncmp(placeholder, "country", 7)){
						strcpy(obj.addr.country, token.data.scalar.value);
					}
				}
				break;
			default:
				break;
		}
		if(token.type != YAML_STREAM_END_TOKEN)
			yaml_token_delete(&token);
	} while(token.type != YAML_STREAM_END_TOKEN);

	// delete the token object
	yaml_token_delete(&token);
	
	// print the result
	printf("yaml parsing completed, printing struct data\n");
	printf("Name: %s, age: %d, city: %s, state: %s, country: %s\n", 
			obj.name,
			obj.age,
			obj.addr.city,
			obj.addr.state,
			obj.addr.country);
cleanup:
	// Delete the yaml parser object
	yaml_parser_delete(&yaml_parser);

	return 0;
}
