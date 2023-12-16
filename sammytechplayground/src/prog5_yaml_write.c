#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <yaml.h>

int main(int argc, char **argv){
	FILE *fp = NULL;
	fp = fopen("sample_data.yaml", "w");
	if(!fp){
		printf("Failed to open the target file\n");
		return -1;
	}

	yaml_emitter_t emitter;
	yaml_emitter_initialize(&emitter);
	yaml_emitter_set_output_file(&emitter, fp);

	yaml_event_t event;
	yaml_stream_start_event_initialize(&event, YAML_UTF8_ENCODING);
	yaml_emitter_emit(&emitter, &event);

	yaml_document_start_event_initialize(&event, NULL, NULL, NULL, 0);
	yaml_emitter_emit(&emitter, &event);

	// Start mapping node
	yaml_mapping_start_event_initialize(&event, NULL, (yaml_char_t*)YAML_MAP_TAG, 1, YAML_ANY_MAPPING_STYLE); 
	yaml_emitter_emit(&emitter, &event);

	// Emit key-value pairs
	yaml_scalar_event_initialize(&event, NULL, NULL, (yaml_char_t *)"name", -1, 1, 0, YAML_PLAIN_SCALAR_STYLE);
	yaml_emitter_emit(&emitter, &event);

	yaml_scalar_event_initialize(&event, NULL, NULL, (yaml_char_t *)"Naveen From C", -1, 1, 0, YAML_PLAIN_SCALAR_STYLE);
	yaml_emitter_emit(&emitter, &event);

	yaml_scalar_event_initialize(&event, NULL, NULL, (yaml_char_t *)"age", -1, 1, 0, YAML_PLAIN_SCALAR_STYLE);
	yaml_emitter_emit(&emitter, &event);

	yaml_scalar_event_initialize(&event, NULL, NULL, (yaml_char_t *)"38", -1, 1, 0, YAML_PLAIN_SCALAR_STYLE);
	yaml_emitter_emit(&emitter, &event);



	// End mapping node
	yaml_mapping_end_event_initialize(&event);
	yaml_emitter_emit(&emitter, &event);

	yaml_document_end_event_initialize(&event, 0);
	yaml_emitter_emit(&emitter, &event);

	// End YAML document
	yaml_stream_end_event_initialize(&event);
	yaml_emitter_emit(&emitter, &event);

	yaml_emitter_flush(&emitter);

	// delete the emitter object
	yaml_emitter_delete(&emitter);
	printf("written\n");

	if(fp)
		fclose(fp);
	return 0;
}
