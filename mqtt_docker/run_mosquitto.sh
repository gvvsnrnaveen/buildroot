#!/bin/sh
mosquitto -c /etc/mosquitto/mosquitto.conf &

sleep 3

# create a client
mosquitto_ctrl -h localhost -p 1883 -u naveen -P mypassword dynsec createclient user1 -p user1password

# create a role
mosquitto_ctrl -h localhost -p 1883 -u naveen -P mypassword dynsec createrole role1

# create a group
mosquitto_ctrl -h localhost -p 1883 -u naveen -P mypassword dynsec creategroup group1

# add ACL roles
mosquitto_ctrl -h localhost -p 1883 -u naveen -P mypassword dynsec addroleacl role1 subscribepattern topic1 allow
mosquitto_ctrl -h localhost -p 1883 -u naveen -P mypassword dynsec addroleacl role1 unsubscribepattern topic1 allow
mosquitto_ctrl -h localhost -p 1883 -u naveen -P mypassword dynsec addroleacl role1 publishclientsend topic1 allow
mosquitto_ctrl -h localhost -p 1883 -u naveen -P mypassword dynsec addroleacl role1 publishclientreceive topic1 allow

# add client to group
mosquitto_ctrl -h localhost -p 1883 -u naveen -P mypassword dynsec addgroupclient group1 user1

# add ACL role to group
mosquitto_ctrl -h localhost -p 1883 -u naveen -P mypassword dynsec addgrouprole group1 role1

# This is for sending periodical updates to Monitor client, as we will see in the example usecase below
# Create a Monitor client
mosquitto_ctrl -p 1883 -u naveen -P mypassword dynsec createclient monitor -p monitorpassword

# create monitor role
mosquitto_ctrl -p 1883 -u naveen -P mypassword dynsec createrole monitorrole

# create monitor group
mosquitto_ctrl -p 1883 -u naveen -P mypassword dynsec creategroup monitorgroup

# create monitor dynsec role ACL
mosquitto_ctrl -p 1883 -u naveen -P mypassword dynsec addroleacl monitorrole subscribepattern monitor allow
mosquitto_ctrl -p 1883 -u naveen -P mypassword dynsec addroleacl monitorrole unsubscribepattern monitor allow
mosquitto_ctrl -p 1883 -u naveen -P mypassword dynsec addroleacl monitorrole publishclientsend monitor allow
mosquitto_ctrl -p 1883 -u naveen -P mypassword dynsec addroleacl monitorrole publishclientreceive monitor allow

# add the monitor and user1 to monitor group along with ACL roles
mosquitto_ctrl -p 1883 -u naveen -P mypassword dynsec addgrouprole monitorgroup monitorrole
mosquitto_ctrl -p 1883 -u naveen -P mypassword dynsec addgroupclient monitorgroup user1
mosquitto_ctrl -p 1883 -u naveen -P mypassword dynsec addgroupclient monitorgroup monitor

# keep the script in loop for mosquitto to run
while [ true ];
do
	sleep 1
done
