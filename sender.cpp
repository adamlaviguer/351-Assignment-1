//Hammad Qureshi
//Adam Laviguer

#include <sys/shm.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "msg.h"    /* For the message struct */

/* The size of the shared memory chunk */
#define SHARED_MEMORY_CHUNK_SIZE 1000

/* The ids for the shared memory segment and the message queue */
int shmid, msqid;

/* The pointer to the shared memory */
void* sharedMemPtr;

//message recMsge;
//message sendMsge;


/**
 * Sets up the shared memory segment and message queue
 * @param shmid - the id of the allocated shared memory
 * @param msqid - the id of the shared memory
 */

void init(int& shmid, int& msqid, void*& sharedMemPtr)
{
	/* TODO:
        1. Create a file called keyfile.txt containing string "Hello world" (you may do
 		    so manually or from the code).
	    2. Use ftok("keyfile.txt", 'a') in order to generate the key.
		3. Use the key in the TODO's below. Use the same key for the queue
		    and the shared memory segment. This also serves to illustrate the difference
		    between the key and the id used in message queues and shared memory. The id
		    for any System V objest (i.e. message queues, shared memory, and sempahores)
		    is unique system-wide among all System V objects. Two objects, on the other hand,
		    may have the same key.
	 */
	printf("Generating the key\n");
	key_t key = ftok("keyfile.txt", 'a'); /*Generate the key*/
	if (key == -1) {
		perror("Could not generate key\n");
		exit(1);
	}
	else {
		printf("Key has been generated successfully\n");
	}


	/* TODO: Get the id of the shared memory segment. The size of the segment must be SHARED_MEMORY_CHUNK_SIZE */
	printf("Getting the ID of the shared memory segment\n");
	shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE, 0666|IPC_CREAT);
	if (shmid == -1) {
		perror("Could not get the ID\n");
		exit(1);
	}
	else {
		printf("Received the ID\n");
	}

	/* TODO: Attach to the shared memory */
	printf("In the process of attaching to shared memory\n");
	sharedMemPtr = shmat(shmid, (void *)0, 0);
	if (sharedMemPtr == (void *)-1) {
		perror("Could not attach to shared memory\n");
		exit(1);
	}
	else {
		printf("Attached to shared memory successfully\n");
	}

	/* TODO: Attach to the message queue */
	printf("Message queue is being created\n");
	msqid = msgget(key, 0666 | IPC_CREAT); //Creates a message queue
	if (msqid == -1) {
		perror("Message queue could not be created\n");
		exit(1);
	}
	else {
		printf("Message queue was created successfully\n");
	}

	/* Store the IDs and the pointer to the shared memory region in the corresponding parameters */


}

/**
 * Performs the cleanup functions
 * @param sharedMemPtr - the pointer to the shared memory
 * @param shmid - the id of the shared memory segment
 * @param msqid - the id of the message queue
 */

void cleanUp(const int& shmid, const int& msqid, void* sharedMemPtr)
{
	/* TODO: Detach from shared memory */
	printf("Going to detach from shared memory\n");

	if (shmdt(sharedMemPtr) == -1) {
		perror("There was an error detaching from memory in cleanUp\n");
		exit(1);
	}
	else {
		printf("Detached from shared memory\n");
	}
}

/**
 * The main send function
 * @param fileName - the name of the file
 */
void send(const char* fileName)
{
	/* Open the file for reading */
	FILE* fp = fopen(fileName, "r");


	/* A buffer to store message we will send to the receiver. */
	message sndMsg;

	/* A buffer to store message received from the receiver. */
	message rcvMsg;

	/* Was the file open? */
	if(!fp)
	{
		perror("fopen\n");
		exit(-1);
	}

	/* Read the whole file */
	while(!feof(fp))
	{
		/* Read at most SHARED_MEMORY_CHUNK_SIZE from the file and store them in shared memory.
 		 * fread will return how many bytes it has actually read (since the last chunk may be less
 		 * than SHARED_MEMORY_CHUNK_SIZE).
 		 */
		if((sndMsg.size = fread(sharedMemPtr, sizeof(char), SHARED_MEMORY_CHUNK_SIZE, fp)) < 0)
		{
			perror("fread\n");
			exit(-1);
		}


		/* TODO: Send a message to the receiver telling him that the data is ready
 		 * (message of type SENDER_DATA_TYPE)
 		 */
		sndMsg.mtype = SENDER_DATA_TYPE;
		printf("Sending the message\n");

		if (msgsnd(msqid, &sndMsg, sizeof(sndMsg), 0) == -1) {
			perror("Error sending the message in void send\n");
			exit(1);
		}
		else{
			printf("The message was sent successfully\n");
		}

		/* TODO: Wait until the receiver sends us a message of type RECV_DONE_TYPE telling us
 		 * that he finished saving the memory chunk.
 		 */
		if (msgrcv(msqid, &rcvMsg, 0, RECV_DONE_TYPE, 0)) {
			perror("Could not retrieve message from receiver of type RECV_DONE_TYPE\n");
			exit(1);
		}
		else{
			printf("Message was retrieved from receiver successfully\n");
		}
	}


	/** TODO: once we are out of the above loop, we have finished sending the file.
 	  * Lets tell the receiver that we have nothing more to send. We will do this by
 	  * sending a message of type SENDER_DATA_TYPE with size field set to 0.
	  */
	sndMsg.size = 0;
	sndMsg.mtype = SENDER_DATA_TYPE;

	printf("Sending an empty message\n");
	//int sendMsge = msgsnd(msqid, &sndMsg, sizeof(sndMsg), 0);
	if (msgsnd(msqid, &sndMsg, sizeof(sndMsg), 0) == -1) {
		perror("There was an error sending the empty message\n");
	}
	else {
		printf("The empty message was sent successfully\n");
	}

	/* Close the file */
	fclose(fp);
	printf("The file has successfully been closed\n");

}


int main(int argc, char** argv)
{

	/* Check the command line arguments */
	if(argc < 2)
	{
		fprintf(stderr, "USAGE: %s <FILE NAME>\n", argv[0]);
		exit(-1);
	}

	/* Connect to shared memory and the message queue */
	init(shmid, msqid, sharedMemPtr);

	/* Send the file */
	send(argv[1]);

	/* Cleanup */
	cleanUp(shmid, msqid, sharedMemPtr);

	return 0;
}
