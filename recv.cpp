#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "msg.h"    /* For the message struct */


/* The size of the shared memory chunk */
#define SHARED_MEMORY_CHUNK_SIZE 1000

/* The ids for the shared memory segment and the message queue */
int shmid, msqid;

/* The pointer to the shared memory */
void *sharedMemPtr;

/* The name of the received file */
const char recvFileName[] = "recvfile";

message recMsge;
message sendMsge;

/**
 * Sets up the shared memory segment and message queue
 * @param shmid - the id of the allocated shared memory
 * @param msqid - the id of the shared memory
 * @param sharedMemPtr - the pointer to the shared memory
 */

void init(int& shmid, int& msqid, void*& sharedMemPtr)
{

	/* TODO: 1. Create a file called keyfile.txt containing string "Hello world" (you may do
 		    so manually or from the code).
	         2. Use ftok("keyfile.txt", 'a') in order to generate the key.
		 3. Use the key in the TODO's below. Use the same key for the queue
		    and the shared memory segment. This also serves to illustrate the difference
		    between the key and the id used in message queues and shared memory. The id
		    for any System V object (i.e. message queues, shared memory, and sempahores)
		    is unique system-wide among all System V objects. Two objects, on the other hand,
		    may have the same key.
	 */



	printf("Generating key\n");
	key_t key = ftok("keyfile.txt", 'A'); //Generate the key
	if (key == -1) {
		perror("Could not generate key\n");
		exit(1);
	}
	else {
		printf("Key has been generated successfully\n");
	}


	/* TODO: Allocate a piece of shared memory. The size of the segment must be SHARED_MEMORY_CHUNK_SIZE. */
	printf("Memory is being allocated\n");
	//int shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE, 0666|IPC_CREAT);
	if (shmget(key, SHARED_MEMORY_CHUNK_SIZE, 0666|IPC_CREAT) == -1) {
		perror("Could not allocate memory\n");
		exit(1);
	}
	else {
		printf("Memory has been allocated successfully\n");
	}

	/* TODO: Attach to the shared memory */
	printf("In the process of attaching to shared memory\n");
	sharedMemPtr = shmat(shmid, (void*)0, 0);
	// if (sharedMemPtr == (void *)-1) {
	// 	perror("Could not attach to shared memory\n");
	// 	exit(1);
	// }
	//else {
		printf("Attached to shared memory successfully\n");
	//}

	/* TODO: Create a message queue */
	//int msgid = msgget(key, 0666|IPC_CREAT); //Creates a message queue
	printf("Message queue is being created\n");
	if (msgget(key, 0666|IPC_CREAT) == -1) {
		perror("Message queue could not be created\n");
		exit(1);
	}
	else {
		printf("Message queue was created successfully\n");
	}

	/* Store the IDs and the pointer to the shared memory region in the corresponding parameters */

}


/**
 * The main loop
 */
void mainLoop()
{
	/* The size of the mesage */
	int msgSize = 0;

	/* Open the file for writing */
	FILE* fp = fopen(recvFileName, "w");

	/* Error checks */
	if(!fp){
		perror("fopen");
		exit(-1);
	}
	else {
		printf("File has been opened successfully. Standing by for message.\n");
	}

    /* TODO: Receive the message and get the message size. The message will
     * contain regular information. The message will be of SENDER_DATA_TYPE
     * (the macro SENDER_DATA_TYPE is defined in msg.h).  If the size field
     * of the message is not 0, then we copy that many bytes from the shared
     * memory region to the file. Otherwise, if 0, then we close the file and
     * exit.
     *
     * NOTE: the received file will always be saved into the file called
     * "recvfile"
     */

	/* Keep receiving until the sender set the size to 0, indicating that
 	 * there is no more data to send
 	 */

	msgSize++;

	while(msgSize != 0)
	{
		printf("A new message is currently being read\n");
		//int recieveMsg = msgrcv(msqid, &recMsge, sizeof(recMsge), SENDER_DATA_TYPE, 0);
		if (msgrcv(msqid, &recMsge, sizeof(recMsge), SENDER_DATA_TYPE, 0) == -1) {
			perror("There was an error retrieving the message\n");
			exit(1);
		}
		else {
			printf("Successfully read in the message\n");
		}

		msgSize = recMsge.size;

		/* If the sender is not telling us that we are done, then get to work */
		if(msgSize != 0)
		{
			/* Save the shared memory to file */
			if(fwrite(sharedMemPtr, sizeof(char), msgSize, fp) < 0)
			{
				perror("fwrite\n");
			}

			/* TODO: Tell the sender that we are ready for the next file chunk.
 			 * I.e. send a message of type RECV_DONE_TYPE (the value of size field
 			 * does not matter in this case).
 			 */
			printf("Prepared to recieve the next chunk\n");

			sendMsge.mtype = RECV_DONE_TYPE;
			sendMsge.size = 0;

			printf("Going to send the empty message back\n");
			int messageSnd = msgsnd(msqid, &sendMsge, 0, 0);
			if (messageSnd == -1) {
				perror("There was an error in sending the empty message\n");
			}
			else {
				printf("Successfully sent the empty message\n");
			}
		}
		/* We are done */
		else
		{
			/* Close the file */
			fclose(fp);
			printf("The file is now closed\n");
		}
	}
}



/**
 * Perfoms the cleanup functions
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

	/* TODO: Deallocate the shared memory chunk */
	printf("Going to deallocate the shared memory chunk\n");
	//int sharedMemChunk = shmctl(shmid, IPC_RMID, NULL);

	if (shmctl(shmid, IPC_RMID, NULL) == -1) {
		perror("Could not deallocate shared memory chunk\n");
		exit(1);
	}
	else {
		printf("Deallocated from shared memory\n");
	}

	/* TODO: Deallocate the message queue */
	printf("Going to deallocate the message queue\n");
	//int msgQ = msgctl(msqid, IPC_RMID, NULL);

	if (msgctl(msqid, IPC_RMID, NULL) == -1) {
		perror("Could not deallocate the message queue\n");
		exit(1);
	}
	else {
		printf("The message queue was deallocated\n");
	}
}

/**
 * Handles the exit signal
 * @param signal - the signal type
 */

void ctrlCSignal(int signal)
{
	/* Free system V resources */
	cleanUp(shmid, msqid, sharedMemPtr);
	printf("User ended the program\n");
}

int main(int argc, char** argv)
{

	/* TODO: Install a singnal handler (see signaldemo.cpp sample file).
 	 * In a case user presses Ctrl-c your program should delete message
 	 * queues and shared memory before exiting. You may add the cleaning functionality
 	 * in ctrlCSignal().
 	 */
	signal(SIGINT, ctrlCSignal);

	/* Initialize */
	init(shmid, msqid, sharedMemPtr);

	/* Go to the main loop */
	mainLoop();

	/** TODO: Detach from shared memory segment, and deallocate shared memory and message queue (i.e. call cleanup) **/
	cleanUp(shmid, msqid, sharedMemPtr);

	printf("Finished\n");

	return 0;
}
