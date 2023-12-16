all:
	$(CC) prog1.c -o prog1
	$(CC) prog2_json_read.c -o prog2_json_read -ljson-c
	$(CC) prog3_json_create.c -o prog3_json_create -ljson-c
	$(CC) prog4_yaml_read.c -o prog4_yaml_read -lyaml
	$(CC) prog5_yaml_write.c -o prog5_yaml_write -lyaml

clean:
	rm -rf prog1 prog2_json_read
